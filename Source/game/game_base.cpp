/*
  ==============================================================================

    game_base.cpp
    Created: 30 Jul 2023 3:45:04pm
    Author:  Vincenzo

  ==============================================================================
*/

#include "game_base.h"

LumatoneSandboxGameBase::LumatoneSandboxGameBase(LumatoneController* controllerIn, juce::String actionName)
    : controller(controllerIn)
    , name(actionName)
    , layoutBeforeStart(controllerIn->getNumBoards(), controllerIn->getOctaveBoardSize())
{
    reset(true);
}

void LumatoneSandboxGameBase::reset(bool clearActionQueue)
{
    if (clearActionQueue)
    {
        clearQueue();
    }
}

void LumatoneSandboxGameBase::clearQueue()
{
    for (int i = 0; i < queueSize; i++)
    {
        int ptr = (queuePtr + i) % MAX_QUEUE_SIZE;
        if (queuedActions[ptr] != nullptr)
            delete queuedActions[ptr];
    }

    queueSize = 0;
    queuePtr = 0;
}

void LumatoneSandboxGameBase::readQueue(juce::UndoableAction** buffer, int& numActions)
{
    if (queueSize < 0)
    {
        queueSize = 0;
    }

    numActions = queueSize;

    for (int i = 0; i < queueSize; i++)
    {
        int ptr = (queuePtr + i) % MAX_QUEUE_SIZE;
        buffer[i] = queuedActions[ptr];
        queuedActions[ptr] = nullptr;
    }

    queuePtr = 0;
    queueSize = 0;
}

void LumatoneSandboxGameBase::addToQueue(juce::UndoableAction* action)
{
    if (queueSize < 0)
        queueSize = 1;
    else if (queueSize < MAX_QUEUE_SIZE)
        queueSize += 1;
    else
        delete queuedActions[getQueuePtr()];

    queuedActions[getQueuePtr()] = action;
}

LumatoneLayout LumatoneSandboxGameBase::queueIdentityLayout(bool resetColors)
{
    LumatoneLayout layout = *controller->getMappingData();

    for (int i = 0; i < controller->getNumBoards(); i++)
    {
        LumatoneBoard& newBoard = *layout.getBoard(i);
        newBoard.board_idx = i;

        int midiChannel = i + 1;

        for (int k = 0; k < controller->getOctaveBoardSize(); k++)
        {
            newBoard.theKeys[k].channelNumber = midiChannel;
            newBoard.theKeys[k].noteNumber = k;
            newBoard.theKeys[k].keyType = LumatoneKeyType::noteOnNoteOff;

            if (resetColors)
            {
                newBoard.theKeys[k].colour = juce::Colour();
            }
        }

        addToQueue(new LumatoneEditAction::SectionEditAction(controller, i, newBoard));
    }

    return layout;
}
