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
    , runGameFps(fps)
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

const LumatoneSandboxGameBase* LumatoneSandboxGameEngine::getGameRunning() const
{
    if (gameIsRunning || gameIsPaused)
        return game.get();
    return nullptr;
}

double LumatoneSandboxGameEngine::getTimeIntervalMs() const
{
    return 1000.0 / runGameFps;
}

double LumatoneSandboxGameEngine::getFps() const
{
    return runGameFps;
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
    if (gameIsRunning)
    {
        jassert(game != nullptr);
        if (gameIsPaused)
        {
            gameIsPaused = false;
        }
        else
        {
            return true;
        }
    }
    else if (game != nullptr)
    {
        addListener(game.get());
        game->reset(true);
        gameIsRunning = true;

        DBG("LumatoneSandboxGameEngine: Starting game " + game->getName());

        engineListeners.call(&LumatoneSandboxGameEngine::Listener::gameStarted);
    }
    else
    {
        return false;
    }

    int fps = game->getLockedFps();
    if (fps == 0)
        fps = defaultFps;

    if (runGameFps != fps)
    {
        runGameFps = fps;
    }

    DBG("Running at " + juce::String(fps) + " fps | " + juce::String(getTimeIntervalMs()) + "ms.");
    
    startTimer(getTimeIntervalMs());
    return true;
}

void LumatoneSandboxGameEngine::forceFps(double fps)
{
    runGameFps = fps;
    startTimer(getTimeIntervalMs());
}

void LumatoneSandboxGameEngine::pauseGame()
{
    if (gameIsRunning)
    {
        gameIsPaused = true;
        // stopTimer();
    }
}

bool LumatoneSandboxGameEngine::endGame()
{
    stopTimer();

    gameIsRunning = false;

    if (game != nullptr)
    {
        game->reset(true);
        removeListener(game.get());

        engineListeners.call(&LumatoneSandboxGameEngine::Listener::gameEnded);

        return true;
    }

    return false;
}

void LumatoneSandboxGameEngine::resetGame()
{
    if (gameIsRunning)
    {
        jassert(game != nullptr);
        if (gameIsPaused)
        {
            gameIsPaused = false;
        }

        game->reset(true);
    }
    else if (game != nullptr)
    {
        addListener(game.get());
        game->reset(true);

        DBG("LumatoneSandboxGameEngine: restarted " + game->getName());

        engineListeners.call(&LumatoneSandboxGameEngine::Listener::gameStarted);
    }
}

void LumatoneSandboxGameEngine::timerCallback()
{
    if (!gameIsRunning)
    {
        stopTimer();
        return;
    }

    if (gameIsPaused)
    {
        game->pauseTick();
    }
    else
    {
        game->nextTick();
    }

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
