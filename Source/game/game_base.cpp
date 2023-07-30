/*
  ==============================================================================

    game_base.cpp
    Created: 30 Jul 2023 3:45:04pm
    Author:  Vincenzo

  ==============================================================================
*/

#include "game_base.h"

LumatoneSandboxGameBase::LumatoneSandboxGameBase(LumatoneController* controllerIn)
    : controller(controllerIn)
{
    reset(true);
}

void LumatoneSandboxGameBase::reset(bool clearQueue)
{
    if (clearQueue)
    {
        queuedActions.clearQuick();
    }
}

void LumatoneSandboxGameBase::readQueue(juce::Array<LumatoneSandboxGameBase::OwnedActionPtr>& buffer)
{
    buffer.clearQuick();

    if (queuedActions.size() == 0)
        return;

    buffer.addArray(queuedActions);
    
    queuedActions.clear();
}
