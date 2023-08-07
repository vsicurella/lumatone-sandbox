/*
  ==============================================================================

    game_base.h
    Created: 30 Jul 2023 3:45:04pm
    Author:  Vincenzo

  ==============================================================================
*/

#pragma once
#include "../lumatone_editor_library/actions/edit_actions.h"

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


class LumatoneSandboxGameBase : public LumatoneMidiState::Listener
{
public:

    LumatoneSandboxGameBase(LumatoneController* controllerIn, juce::String gameName);
    virtual ~LumatoneSandboxGameBase()
    {
        reset(true);
    }

    LumatoneLayout queueIdentityLayout(bool resetColors);

    virtual void reset(bool clearActionQueue);
    virtual void nextTick() = 0;

    virtual void clearQueue();
    void readQueue(juce::UndoableAction** buffer, int& numActions);

    juce::String getName() const { return name; }

    const LumatoneLayout& getLayoutBeforeStart() const { return layoutBeforeStart; }

private:

    int getQueuePtr() const { return (queuePtr + queueSize - 1) % MAX_QUEUE_SIZE; }

protected:

    LumatoneLayout layoutBeforeStart;

    virtual void addToQueue(juce::UndoableAction* action);
    virtual juce::UndoableAction* renderFrame() = 0;

    //juce::OwnedArray<juce::UndoableAction, juce::CriticalSection> queuedActions;

    juce::UndoableAction* queuedActions[MAX_QUEUE_SIZE];
    int queueSize = 0;
    int queuePtr = 0;

    LumatoneController* controller;

private:

    juce::String name;
};
