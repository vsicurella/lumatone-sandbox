/*
  ==============================================================================

    lumatone_action.h
    Created: 30 Jul 2023 4:13:04pm
    Author:  Vincenzo

  ==============================================================================
*/

#pragma once
#include "../LumatoneController.h"

class LumatoneAction : public juce::UndoableAction
{
public:

    LumatoneAction(LumatoneController* controllerIn)
    {
        if (controllerIn)
        {
            controller = controllerIn;
            octaveBoardSize = controller->getOctaveBoardSize();
            numOctaveBoards = controller->getNumBoards();
        }
    }

    virtual bool perform() = 0;
    virtual bool undo() = 0;

    int getSizeInUnits() override = 0;

protected:

    LumatoneController* controller = nullptr;

    int octaveBoardSize = 56;
    int numOctaveBoards = 5;
};
