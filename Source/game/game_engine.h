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
    bool endGame();
    void resetGame();

    int getTimeInterval() const;

    bool isGameRunning() const { return gameIsRunning; }

public:

    void handleLumatoneMidi(LumatoneMidiState* midiState, const juce::MidiMessage& msg) override;

private:

    juce::ListenerList<LumatoneSandboxGameEngine::Listener> engineListeners;

public:

    void addEngineListener(LumatoneSandboxGameEngine::Listener* listenerIn) { engineListeners.add(listenerIn); }
    void removeEngineListener(LumatoneSandboxGameEngine::Listener* listenerIn) { engineListeners.add(listenerIn); }

private:

    void timerCallback() override;

private:

    juce::ApplicationCommandManager* commandManager;
    LumatoneController* controller;

    std::unique_ptr<LumatoneSandboxGameBase> game;

    juce::UndoableAction* actionQueue[MAX_QUEUE_SIZE];
    int numActions = 0;

    int desiredFps = 30;

    bool gameIsRunning = false;
    bool sentFirstGameMessage = false;
};