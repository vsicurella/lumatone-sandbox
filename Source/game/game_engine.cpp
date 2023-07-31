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
    game = nullptr;
    controller = nullptr;
}

int LumatoneSandboxGameEngine::getTimeInterval() const
{
    return juce::roundToInt((float)1000 / (float)desiredFps);
}

void LumatoneSandboxGameEngine::setGame(LumatoneSandboxGameBase* newGameIn)
{
    endGame();
    game.reset(newGameIn);
}


bool LumatoneSandboxGameEngine::startGame()
{
    if (!gameIsRunning && game != nullptr)
    {
        gameIsRunning = true;
        startTimer(getTimeInterval());
        engineListeners.call(&LumatoneSandboxGameEngine::Listener::gameStarted);
        return true;
    }

    return false;
}


bool LumatoneSandboxGameEngine::endGame()
{
    stopTimer();

    gameIsRunning = false;
    sentFirstGameMessage = false;

    if (game != nullptr)
    {
        game->reset(true);

        engineListeners.call(&LumatoneSandboxGameEngine::Listener::gameEnded);

        return true;
    }

    return false;
}

void LumatoneSandboxGameEngine::resetGame()
{
    endGame();
}

void LumatoneSandboxGameEngine::timerCallback()
{
    if (!gameIsRunning)
    {
        stopTimer();
        return;
    }

    game->nextTick();

    juce::OwnedArray<juce::UndoableAction> actions;
    game->readQueue(actions);

    if (actions.size() == 0)
        return;

    auto title = "Game Action: " + game->getName();
    auto newTransaction = sentFirstGameMessage == false;
    if (newTransaction)
        sentFirstGameMessage = true;

    for (int i = 0; i < actions.size(); i++)
        controller->performUndoableAction(actions.removeAndReturn(0), newTransaction, title);
}
