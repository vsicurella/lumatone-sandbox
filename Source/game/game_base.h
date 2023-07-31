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

    LumatoneSandboxGameBase(LumatoneController* controllerIn);
    virtual ~LumatoneSandboxGameBase() {}

    virtual void reset(bool clearQueue);
    virtual void nextTick() = 0;

    void readQueue(juce::Array<juce::UndoableAction*>& buffer);

protected:

    virtual juce::UndoableAction* renderFrame() = 0;

    juce::OwnedArray<juce::UndoableAction> queuedActions;

    LumatoneController* controller;
};
