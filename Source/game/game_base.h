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


class LumatoneSandboxGameBase
{
public:

    LumatoneSandboxGameBase(LumatoneController* controllerIn, juce::String gameName);
    virtual ~LumatoneSandboxGameBase()
    {
        queuedActions.clear();
    }

    virtual void reset(bool clearQueue);
    virtual void nextTick() = 0;

    void readQueue(juce::OwnedArray<juce::UndoableAction>& buffer);

    juce::String getName() const { return name; }

protected:

    virtual juce::UndoableAction* renderFrame() = 0;

    juce::OwnedArray<juce::UndoableAction> queuedActions;

    LumatoneController* controller;

private:

    juce::String name;
};
