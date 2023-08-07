/*
  ==============================================================================

    game_engine.cpp
    Created: 30 Jul 2023 3:44:42pm
    Author:  Vincenzo

  ==============================================================================
*/

#include "game_engine.h"

LumatoneSandboxGameEngine::LumatoneSandboxGameEngine(LumatoneController* controllerIn, int fps)
    : LumatoneMidiState(*controllerIn)
    , controller(controllerIn)
    , desiredFps(fps)
{
    controller->addMidiListener(this);
}

LumatoneSandboxGameEngine::~LumatoneSandboxGameEngine()
{
    for (int i = 0; i < numActions; i++)
        delete actionQueue[i];

    engineListeners.clear();
    game = nullptr;
    controller = nullptr;
}

int LumatoneSandboxGameEngine::getTimeInterval() const
{
    return juce::roundToInt((float)1000 / (float)desiredFps);
}

void LumatoneSandboxGameEngine::handleLumatoneMidi(LumatoneMidiState* midiState, const juce::MidiMessage& msg)
{
    processNextMidiEvent(msg);
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
        addListener(game.get());
        game->reset(true);
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
        removeListener(game.get());
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
    game->readQueue(actionQueue, numActions);

    if (numActions == 0)
        return;

    auto title = "Game Action: " + game->getName();

    int queueSize = numActions;
    for (int i = 0; i < queueSize; i++)
    {
        controller->performUndoableAction(actionQueue[i], true, title);
        actionQueue[i] = nullptr;
        numActions--;
    }
}
