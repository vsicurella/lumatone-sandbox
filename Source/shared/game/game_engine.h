/*
  ==============================================================================

    game_engine.h
    Created: 30 Jul 2023 3:44:42pm
    Author:  Vincenzo

  ==============================================================================
*/

#pragma once
#include "game_base.h"

class LumatoneSandboxGameEngine : public LumatoneMidiState
                                , public LumatoneMidiState::Listener
                                , private juce::Timer
{
public:

    struct Listener
    {
        Listener() {}
        virtual ~Listener() {}

        virtual void gameStarted() {};
        virtual void gameEnded() {};
    };


public:

    struct Options
    {

    };

public:

    LumatoneSandboxGameEngine(LumatoneController* controllerIn, int fps);
    ~LumatoneSandboxGameEngine();

    LumatoneController* getController()  { return controller; }
   
    void setGame(LumatoneSandboxGameBase* newGameIn);

    bool startGame();
    void pauseGame();
    bool endGame();
    void resetGame();

    const LumatoneSandboxGameBase* getGameRunning() const;

    double getTimeIntervalMs() const;
    double getFps() const;
    void forceFps(double fps);

    bool isGameRunning() const { return gameIsRunning; }

public:

    void handleLumatoneMidi(LumatoneMidiState* midiState, const juce::MidiMessage& msg) override;

private:

    juce::ListenerList<LumatoneSandboxGameEngine::Listener> engineListeners;

public:

    void addEngineListener(LumatoneSandboxGameEngine::Listener* listenerIn) { engineListeners.add(listenerIn); }
    void removeEngineListener(LumatoneSandboxGameEngine::Listener* listenerIn) { engineListeners.add(listenerIn); }

private:

    void advanceFrame();

    void processGameActionQueue();

    void timerCallback() override;

private:

    juce::ApplicationCommandManager* commandManager;
    LumatoneController* controller;

    std::unique_ptr<LumatoneSandboxGameBase> game;

    LumatoneAction* actionQueue[MAX_QUEUE_SIZE];
    int numActions = 0;

    double defaultFps = 30;
    double runGameFps = 30;

    bool gameIsRunning = false;
    bool gameIsPaused = false;
    bool sentFirstGameMessage = false;
};