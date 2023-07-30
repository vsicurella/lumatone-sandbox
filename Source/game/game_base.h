/*
  ==============================================================================

    game_base.h
    Created: 30 Jul 2023 3:45:04pm
    Author:  Vincenzo

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "../lumatone_editor_library/actions/edit_actions.h"

class KeyColorConstrainer
{
public:

    KeyColorConstrainer() {}
    ~KeyColorConstrainer() {}

    virtual juce::Colour validColour(juce::Colour targetColour, int boardIndex, int keyIndex) 
    { 
        return targetColour; 
    };


};


class LumatoneSandboxGameBase
{
public:
    using OwnedActionPtr = std::unique_ptr<juce::UndoableAction>;

public:

    LumatoneSandboxGameBase(LumatoneController* controllerIn);
    virtual ~LumatoneSandboxGameBase() {}

    virtual void reset(bool clearQueue);
    virtual void nextTick() = 0;

    void readQueue(juce::Array<OwnedActionPtr>& buffer);

protected:

    virtual OwnedActionPtr renderFrame() = 0;

    juce::Array<OwnedActionPtr> queuedActions;

    LumatoneController* controller;
};
