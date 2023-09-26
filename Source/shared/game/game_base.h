/*
  ==============================================================================

    game_base.h
    Created: 30 Jul 2023 3:45:04pm
    Author:  Vincenzo

  ==============================================================================
*/

#pragma once

// #include "../lumatone_editor_library/LumatoneController.h"
// #include "../lumatone_editor_library/actions/edit_actions.h"

#include "../lumatone_editor_library/data/lumatone_context.h"
#include "../lumatone_editor_library/listeners/midi_listener.h"

class LumatoneAction;
class LumatoneController;

class KeyColorConstrainer
{
public:

    KeyColorConstrainer() {}
    virtual ~KeyColorConstrainer() {}

    virtual juce::Colour validColour(juce::Colour targetColour, int boardIndex, int keyIndex) 
    { 
        return targetColour; 
    };


};

#define MAX_QUEUE_SIZE 280


class LumatoneSandboxGameBase : public LumatoneEditor::MidiListener
{
public:

    LumatoneSandboxGameBase(LumatoneController* controllerIn, juce::String gameName);
    virtual ~LumatoneSandboxGameBase()
    {
        reset(true);
    }

    void queueLayout(const LumatoneLayout& layout);
    LumatoneLayout getIdentityLayout(bool resetColors);
    LumatoneContext getIdentityWithLayoutContext(bool resetColors);

    virtual void reset(bool clearActionQueue);
    virtual void nextTick() = 0;
    virtual void pauseTick() { }

    virtual void clearQueue();
    void readQueue(LumatoneAction** buffer, int& numActions);

    virtual void end();

    virtual double getLockedFps() const { return 0; }

    juce::String getName() const { return name; }

    const LumatoneLayout& getLayoutBeforeStart() const { return layoutBeforeStart; }

protected:

    void noteOn(int midiChannel, int midiNote, juce::uint8 velocity);
    void noteOff(int midiChannel, int midiNote);

    void allNotesOff(int midiChannel);
    void allNotesOff();

private:

    int getQueuePtr() const { return (queuePtr + queueSize - 1) % MAX_QUEUE_SIZE; }

protected:

    LumatoneLayout layoutBeforeStart;

    virtual void addToQueue(LumatoneAction* action);
    virtual LumatoneAction* renderFrame() = 0;

    //juce::OwnedArray<juce::UndoableAction, juce::CriticalSection> queuedActions;

    LumatoneAction* queuedActions[MAX_QUEUE_SIZE];
    int queueSize = 0;
    int queuePtr = 0;

    LumatoneController* controller;

private:
    juce::String name;
};
