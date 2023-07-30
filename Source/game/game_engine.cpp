/*
  ==============================================================================

    game_engine.cpp
    Created: 30 Jul 2023 3:44:42pm
    Author:  Vincenzo

  ==============================================================================
*/

#include "game_engine.h"

LumatoneSandboxGameEngine::LumatoneSandboxGameEngine(LumatoneController* controllerIn, int fps)
    : controller(controllerIn)
    , desiredFps(fps)
{

}

LumatoneSandboxGameEngine::~LumatoneSandboxGameEngine()
{
    engineListeners.clear();

    controller = nullptr;
}

int LumatoneSandboxGameEngine::getTimeInterval() const
{
    return juce::roundToInt((float)1000 / (float)desiredFps);
}

void LumatoneSandboxGameEngine::setGame(std::unique_ptr<LumatoneSandboxGameBase> newGameIn)
{
    endGame();

    game.swap(newGameIn);
}


bool LumatoneSandboxGameEngine::startGame()
{
    if (!gameIsRunning && game != nullptr)
    {
        gameIsRunning = true;
        startTimer(getTimeInterval());
        engineListeners.call(&LumatoneSandboxGameEngine::Listener::gameStarted);
    }
}


bool LumatoneSandboxGameEngine::endGame()
{
    stopTimer();

    gameIsRunning = false;

    if (game != nullptr)
    {
        game->reset(true);

        engineListeners.call(&LumatoneSandboxGameEngine::Listener::gameEnded);
    }
}

void LumatoneSandboxGameEngine::timerCallback()
{
    if (!gameIsRunning)
    {
        stopTimer();
        return;
    }

    game->nextTick();

    juce::Array<LumatoneSandboxGameBase::OwnedActionPtr> actions;
    game->readQueue(actions);

    if (actions.size() == 0)
        return;

    juce::UndoableAction* combinedAction = actions[0].get();
    for (int i = 1; i < actions.size(); i++)
        combinedAction->createCoalescedAction(actions[i].get());

    bool performed = controller->performUndoableAction(combinedAction);
}
