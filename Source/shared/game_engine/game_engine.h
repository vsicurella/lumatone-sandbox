/*
  ==============================================================================

    game_engine.h
    Created: 30 Jul 2023 3:44:42pm
    Author:  Vincenzo

  ==============================================================================
*/

#pragma once

#include "../lumatone_editor_library/listeners/midi_listener.h"

#include "../lumatone_editor_library/actions/edit_actions.h"

#include "./game_engine_state.h"
#include "../debug/LumatoneSandboxLogger.h"

#define MAX_QUEUE_SIZE 280

class LumatoneSandboxGameBase;
class LumatoneController;
class LumatoneKeyboardComponent;

class LumatoneSandboxState;

class LumatoneSandboxGameEngine : public LumatoneGameEngineState
                                , private LumatoneGameEngineState::Controller
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

    LumatoneSandboxGameEngine(LumatoneSandboxState& stateIn);
    ~LumatoneSandboxGameEngine() override;
    
    void loadGame(juce::String gameId);

    bool startGame();
    void pauseGame();
    bool endGame();
    void resetGame();

    void forceFps(double fps) override;

    const LumatoneSandboxGameBase* getGameLoaded() const;

    void setVirtualKeyboard(LumatoneKeyboardComponent* keyboardIn);

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
    LumatoneKeyboardComponent* keyboard;

    std::unique_ptr<LumatoneSandboxGameBase> game;

    LumatoneEditor::LayoutAction actionQueue[MAX_QUEUE_SIZE];
    int numActions = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LumatoneSandboxGameEngine)
};
