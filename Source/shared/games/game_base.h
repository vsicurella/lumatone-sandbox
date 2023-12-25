/*
  ==============================================================================

    game_base.h
    Created: 30 Jul 2023 3:45:04pm
    Author:  Vincenzo

  ==============================================================================
*/

#pragma once

#include "../game_engine/game_engine_state.h"

#include "../lumatone_editor_library/data/lumatone_context.h"

#include "../lumatone_editor_library/listeners/midi_listener.h"
#include "../lumatone_editor_library/listeners/editor_listener.h"

#include "../lumatone_editor_library/LumatoneKeyboardComponent.h"

#include "../debug/LumatoneSandboxLogger.h"

class LumatoneAction;
class LumatoneController;

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
                              , protected LumatoneSandboxLogger
{
public:

    LumatoneSandboxGameBase(LumatoneController* controllerIn, juce::String gameName);
    virtual ~LumatoneSandboxGameBase()
    {
        reset(true);
    }

    void queueLayout(const LumatoneLayout& layout);
    LumatoneLayout getIdentityLayout(bool resetColors=true, juce::Colour boardColour={});
    LumatoneContext getIdentityWithLayoutContext(bool resetColors);

    // currently unused return statement
    virtual bool reset(bool clearActionQueue);

    // Return whether game should continue running or not
    virtual bool nextTick() = 0;
    virtual bool pauseTick() { return true; }

    virtual void clearQueue();
    void readQueue(LumatoneAction** buffer, int& numActions);

    virtual void requestQuit() { quitGame = true; }
    virtual void end();

    virtual double getLockedFps() const { return 0; }

    juce::String getName() const { return name; }

    const LumatoneLayout& getLayoutBeforeStart() const { return layoutBeforeStart; }
    void updateSavedLayout();

public:
    virtual LumatoneSandboxGameComponent* createController() { return nullptr; }

protected:
    LumatoneKeyContext getKeyAt(int boardIndex, int keyIndex) const;

    virtual void quit() { quitGame = false; }

private:
    int getQueuePtr() const { return (queuePtr + queueSize - 1) % MAX_QUEUE_SIZE; }

protected:
    void completeMappingLoaded(LumatoneLayout layout) override;

protected:
    LumatoneLayout layoutBeforeStart;

    virtual void addToQueue(LumatoneAction* action);
    virtual LumatoneAction* renderFrame() const { return nullptr; }

    LumatoneAction* queuedActions[MAX_QUEUE_SIZE];
    int queueSize = 0;
    int queuePtr = 0;

    LumatoneController* controller;

    bool quitGame;

private:
    juce::String name;
};
