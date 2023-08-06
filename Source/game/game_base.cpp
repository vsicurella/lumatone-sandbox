/*
  ==============================================================================

    game_base.cpp
    Created: 30 Jul 2023 3:45:04pm
    Author:  Vincenzo

  ==============================================================================
*/

#include "game_base.h"

LumatoneSandboxGameBase::LumatoneSandboxGameBase(LumatoneController* controllerIn, juce::String actionName)
    : LumatoneMidiState(*controllerIn)
    , controller(controllerIn)
    , name(actionName)
    , layoutBeforeStart(controller->getNumBoards(), controller->getOctaveBoardSize())
{
    reset(true);
}

void LumatoneSandboxGameBase::reset(bool clearQueue)
{
    if (clearQueue)
    {
        queuedActions.clear();
    }
}

void LumatoneSandboxGameBase::readQueue(juce::OwnedArray<juce::UndoableAction>& buffer)
{
    if (queuedActions.size() == 0)
        return;

    queuedActions.swapWithArray(buffer);
}
