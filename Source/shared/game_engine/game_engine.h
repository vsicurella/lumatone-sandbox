/*
  ==============================================================================

    game_engine.h
    Created: 30 Jul 2023 3:44:42pm
    Author:  Vincenzo

  ==============================================================================
*/

#pragma once

#include "../Lumatone_editor_library/listeners/midi_listener.h"

#include "./game_engine_state.h"
#include "../debug/LumatoneSandboxLogger.h"

#define MAX_QUEUE_SIZE 280

class LumatoneSandboxGameBase;
class LumatoneAction;
class LumatoneController;

class LumatoneSandboxGameEngine : public LumatoneGameEngineState
                                , private LumatoneEditor::MidiListener
                                , private juce::Timer
                                , private LumatoneSandboxLogger
{
public:
    struct Listener
    {
        Listener() {}
        virtual ~Listener() {}

        virtual void gameStatusChanged(LumatoneSandboxGameBase* game, LumatoneGameEngineState::GameStatus status) {}
        virtual void gameEnded() {}
    };

public:

    LumatoneSandboxGameEngine(LumatoneController* controllerIn, juce::ValueTree parentTreeIn);
    ~LumatoneSandboxGameEngine() override;

    LumatoneController* getController()  { return controller; }
   
    void loadGame(juce::String gameId);
    // void setGame(LumatoneSandboxGameBase* newGameIn);

    bool startGame();
    void pauseGame();
    bool endGame();
    void resetGame();

    void forceFps(double fps) override;

    const LumatoneSandboxGameBase* getGameLoaded() const;

    // DEBUG
    // bool checkNotesOn();

private:
    void setGameStatus(LumatoneGameEngineState::GameStatus newStatus, bool writeToState=true) override;

private:
    juce::ListenerList<LumatoneSandboxGameEngine::Listener> engineListeners;

public:
    void addEngineListener(LumatoneSandboxGameEngine::Listener* listenerIn) { engineListeners.add(listenerIn); }
    void removeEngineListener(LumatoneSandboxGameEngine::Listener* listenerIn) { engineListeners.add(listenerIn); }

private:
    void advanceFrame();
    void processGameActionQueue();

private:
    void handleStatePropertyChange(juce::ValueTree stateIn, const juce::Identifier& property) override;

private:
    void timerCallback() override;

private:
    juce::ApplicationCommandManager* commandManager;
    LumatoneController* controller;

    std::unique_ptr<LumatoneSandboxGameBase> game;

    LumatoneAction* actionQueue[MAX_QUEUE_SIZE];
    int numActions = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LumatoneSandboxGameEngine)
};
