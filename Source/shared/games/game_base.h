/*
  ==============================================================================

    game_base.h
    Created: 30 Jul 2023 3:45:04pm
    Author:  Vincenzo

  ==============================================================================
*/

#ifndef LUMATONE_GAME_BASE_H
#define LUMATONE_GAME_BASE_H

#include "../game_engine/game_engine_state.h"

#include "../lumatone_editor_library/data/lumatone_context.h"

#include "../lumatone_editor_library/actions/edit_actions.h"

#include "../lumatone_editor_library/listeners/midi_listener.h"
#include "../lumatone_editor_library/listeners/editor_listener.h"

#include "../lumatone_editor_library/ui/keyboard_component.h"

#include "../lumatone_editor_library/midi/lumatone_midi_manager.h"

#include "../debug/LumatoneSandboxLogger.h"


// class LumatoneAction;
class LumatoneSandboxState;

class LumatoneSandboxGameComponent;

class KeyColorConstrainer
{
public:
    KeyColorConstrainer() {}
    virtual ~KeyColorConstrainer() {}

    virtual juce::Colour validColour(juce::Colour targetColour, int boardIndex, int keyIndex) 
    { 
        return targetColour; 
    }
};

#define MAX_QUEUE_SIZE 280


class LumatoneSandboxGameBase : public LumatoneEditor::MidiListener
                              , public LumatoneEditor::EditorListener
                              , public LumatoneKeyboardComponent::Listener
                              , protected LumatoneApplicationMidi::Controller
                              , protected LumatoneSandboxLogger
{
public:

    LumatoneSandboxGameBase(juce::String gameName, const LumatoneApplicationState& stateIn);
    virtual ~LumatoneSandboxGameBase() override
    {
        reset(true);
    }

    void queueLayout(const LumatoneLayout& layout);

    // currently unused return statement
    virtual bool reset(bool clearActionQueue);

    // Return whether game should continue running or not
    virtual bool nextTick() = 0;
    virtual bool pauseTick() { return true; }

    virtual void clearQueue();
    void readQueue(LumatoneEditor::LayoutAction* buffer, int& numActions);

    virtual void requestQuit() { quitGame = true; }
    virtual void end();

    virtual double getLockedFps() const { return 0; }

    juce::String getName() const { return name; }

public:
    virtual LumatoneSandboxGameComponent* createController() { return nullptr; }

protected:
    virtual void quit() { quitGame = false; }

private:
    int getQueuePtr() const { return (queuePtr + queueSize - 1) % MAX_QUEUE_SIZE; }

protected:
    void completeMappingLoaded(const LumatoneLayout& layout) override;

protected:
    virtual void addToQueue(const LumatoneEditor::LayoutAction& action);
    virtual LumatoneEditor::LayoutAction renderFrame() const = 0;

    LumatoneEditor::LayoutAction queuedActions[MAX_QUEUE_SIZE];
    int queueSize = 0;
    int queuePtr = 0;

    bool quitGame;

private:
    juce::String name;
    const LumatoneApplicationState& appState;
};

#endif // LUMATONE_GAME_BASE_H
