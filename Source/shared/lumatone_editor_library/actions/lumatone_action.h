/*
  ==============================================================================

    lumatone_action.h
    Created: 30 Jul 2023 4:13:04pm
    Author:  Vincenzo

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class LumatoneApplicationState;

class LumatoneAction : public juce::UndoableAction
{
public:

    LumatoneAction(LumatoneApplicationState* controllerIn, juce::String nameIn);

    virtual bool perform() = 0;
    virtual bool undo() = 0;

    int getSizeInUnits() override = 0;

    juce::String getName() const { return name; }

protected:

    juce::String name;

    LumatoneApplicationState* state = nullptr;

    int octaveBoardSize = 56;
    int numOctaveBoards = 5;
};
