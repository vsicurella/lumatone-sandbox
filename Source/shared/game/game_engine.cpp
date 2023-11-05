/*
  ==============================================================================

    game_engine.cpp
    Created: 30 Jul 2023 3:44:42pm
    Author:  Vincenzo

  ==============================================================================
*/

#include "game_engine.h"
#include "../lumatone_editor_library/LumatoneController.h"

LumatoneSandboxGameEngine::LumatoneSandboxGameEngine(LumatoneController* controllerIn, int fps)
    : controller(controllerIn)
    , runGameFps(fps)
    , LumatoneSandboxLogger("GameEngine")
{
    controller->addMidiListener(this);
    logInfo("LumatoneSandboxGameEngine", "Game Engine initialized.");
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

    logWarning("getGameRunning", "No game is running.");
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

void LumatoneSandboxGameEngine::setGame(LumatoneSandboxGameBase* newGameIn)
{
    endGame();
    game.reset(newGameIn);

    logInfo("setGame", "New game loaded: " + game->getName());
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
        controller->addMidiListener(game.get());
        controller->addEditorListener(game.get());

        game->reset(true);
        gameIsRunning = true;

        logInfo("startGame", "Starting game " + game->getName());

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

    logInfo("startGame", "Running at " + juce::String(fps) + " fps / " + juce::String(getTimeIntervalMs()) + "ms.");
    
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
    logInfo("endGame", "Stopping game.");

    stopTimer();

    gameIsRunning = false;

    if (game != nullptr)
    {
        controller->removeMidiListener(game.get());
        controller->removeEditorListener(game.get());
        
        game->end();
        processGameActionQueue();

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
        controller->addMidiListener(game.get());
        game->reset(true);

        // DBG("LumatoneSandboxGameEngine: restarted " + game->getName());
        logInfo("resetGame", "Restarted game.");

        engineListeners.call(&LumatoneSandboxGameEngine::Listener::gameStarted);
    }
    else
    {
        logWarning("resetGame", "Trying to reset with no game loaded.");
    }
}

void LumatoneSandboxGameEngine::advanceFrame()
{
    if (gameIsPaused)
    {
        game->pauseTick();
    }
    else
    {
        game->nextTick();
    }
}

void LumatoneSandboxGameEngine::processGameActionQueue()
{
    game->readQueue(actionQueue, numActions);

    if (numActions == 0)
        return;

    auto title = "Game Action: " + game->getName();

    int queueSize = numActions;
    for (int i = 0; i < queueSize; i++)
    {
        controller->performAction(actionQueue[i], false);
        actionQueue[i] = nullptr;
        numActions--;
    }
}

void LumatoneSandboxGameEngine::timerCallback()
{
    if (!gameIsRunning)
    {
        stopTimer();
        return;
    }

    advanceFrame();
    processGameActionQueue();
}
