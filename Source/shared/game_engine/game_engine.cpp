/*
  ==============================================================================

    game_engine.cpp
    Created: 30 Jul 2023 3:44:42pm
    Author:  Vincenzo

  ==============================================================================
*/

#include "./game_engine.h"
#include "./game_loader.h"
#include "../games/game_base.h"

#include "../lumatone_editor_library/LumatoneController.h"
#include "../SandboxMenu.h"

LumatoneSandboxGameEngine::LumatoneSandboxGameEngine(LumatoneController* controllerIn, juce::ValueTree parentTreeIn)
    : LumatoneGameEngineState("GameEngine", parentTreeIn)
    , controller(controllerIn)
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

const LumatoneSandboxGameBase* LumatoneSandboxGameEngine::getGameLoaded() const
{
    if (isGameLoaded())
        return game.get();

    logWarning("getGameLoaded", "No game is running.");
    return nullptr;
}

void LumatoneSandboxGameEngine::loadGame(juce::String gameId)
{
    endGame();

    auto newGame = LumatoneSandboxGameLoader::CreateGameInstance(gameId, state, controller);
    if (newGame == nullptr)
    {
        logError("setGame", "Error loading game: " + gameId);
        return;
    }

    game.reset(newGame);
    logInfo("setGame", "New game loaded: " + game->getName());

    setGameStatus(LumatoneGameEngineState::GameStatus::Loaded);
}

void LumatoneSandboxGameEngine::setGameStatus(LumatoneGameEngineState::GameStatus newStatus)
{
    LumatoneGameEngineState::setGameStatus(newStatus);
    
    LumatoneSandboxGameBase* gamePtr = isGameLoaded() ? game.get() : nullptr;
    engineListeners.call(&LumatoneSandboxGameEngine::Listener::gameStatusChanged, gamePtr, newStatus);
}

// void LumatoneSandboxGameEngine::setGame(LumatoneSandboxGameBase *newGameIn)
// {
//     endGame();
//     game.reset(newGameIn);

//     logInfo("setGame", "New game loaded: " + game->getName());
// }

bool LumatoneSandboxGameEngine::startGame()
{
    if (isGameLoaded())
    {
        jassert(game != nullptr);
        if (isGameRunning())
        {
            return true;
        }
        else if (game != nullptr)
        {
            controller->addMidiListener(game.get());
            controller->addEditorListener(game.get());

            game->reset(true);
            logInfo("startGame", "Starting game " + game->getName());
        }
        else
        {
            setGameStatus(LumatoneGameEngineState::GameStatus::NoGame);
            logError("startGame", "Cannot start: game status is 'loaded' but game pointer is empty.");
            return false;
        }
    }
    else
    {
        logWarning("startGame", "Cannot start, no game is loaded.");
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
    
    startTimer(juce::roundToInt(getTimeIntervalMs()));
    setGameStatus(LumatoneGameEngineState::GameStatus::Running);
    return true;
}

void LumatoneSandboxGameEngine::forceFps(double fps)
{
    LumatoneGameEngineState::forceFps(fps);
    startTimer(getTimeIntervalMs());
}

void LumatoneSandboxGameEngine::pauseGame()
{
    if (isGameLoaded() && isGameRunning())
    {
        setGameStatus(LumatoneGameEngineState::GameStatus::Paused);
    }
}

bool LumatoneSandboxGameEngine::endGame()
{
    if (isGameLoaded())
    {
        logInfo("endGame", "Stopping game.");

        stopTimer();

        setGameStatus(LumatoneGameEngineState::GameStatus::Stopped);

        if (game != nullptr)
        {
            controller->removeMidiListener(game.get());
            controller->removeEditorListener(game.get());
            
            game->end();
            processGameActionQueue();

            engineListeners.call(&LumatoneSandboxGameEngine::Listener::gameEnded);
            return true;
        }

        logError("endGame", "Stopped timer, but game pointer is null.");
        return false;
    }

    logWarning("endGame", "No game loaded.");
    return false;
}

void LumatoneSandboxGameEngine::resetGame()
{
    if (isGameLoaded())
    {
        jassert(game != nullptr);
        game->reset(true);

        setGameStatus(LumatoneGameEngineState::GameStatus::Stopped);
    }
    else if (game != nullptr)
    {
        controller->addMidiListener(game.get());
        game->reset(true);

        setGameStatus(LumatoneGameEngineState::GameStatus::Loaded);
        // DBG("LumatoneSandboxGameEngine: restarted " + game->getName());
        logInfo("resetGame", "Restarted game.");

        // engineListeners.call(&LumatoneSandboxGameEngine::Listener::gameStarted, game);
    }
    else
    {
        logWarning("resetGame", "Trying to reset with no game loaded.");
    }
}

void LumatoneSandboxGameEngine::advanceFrame()
{
    if (isGamePaused())
    {
        game->pauseTick();
    }
    else if (isGameRunning())
    {
        game->nextTick();
    }
    else
    {
        logError("advanceFrame", "Trying to advance frame when game is not paused or running!");
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

void LumatoneSandboxGameEngine::handleStatePropertyChange(juce::ValueTree stateIn, const juce::Identifier &property)
{
    LumatoneSandboxGameEngine::handleStatePropertyChange(stateIn, property);

    // Should probably incorporate this with Logger
    if (property == LumatoneGameEngineState::ID::LogLevel)
    {
        int defaultLevel = 
        #if JUCE_DEBUG
            static_cast<int>(LumatoneSandboxLogger::LogLevel::INFO);
        #else
            static_cast<int>(LumatoneSandboxLogger::LogLevel::NONE);
        #endif

        int readLevel = static_cast<int>(stateIn.getProperty(property, juce::var(defaultLevel)));
        logLevel = static_cast<LumatoneSandboxLogger::LogLevel>(readLevel);
    }
}

void LumatoneSandboxGameEngine::timerCallback()
{
    stopTimer();

    if (isGameLoaded())
    {
        if (isGameRunning() || isGamePaused())
            advanceFrame();
            
        processGameActionQueue();

        startTimer(getTimeIntervalMs());
    }
}
