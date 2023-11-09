/*
  ==============================================================================

    game_base.cpp
    Created: 30 Jul 2023 3:45:04pm
    Author:  Vincenzo

  ==============================================================================
*/

#include "./game_base.h"

#include "../lumatone_editor_library/LumatoneController.h"
#include "../lumatone_editor_library/actions/edit_actions.h"

LumatoneSandboxGameBase::LumatoneSandboxGameBase(juce::ValueTree engineStateIn, LumatoneController* controllerIn, juce::String nameIn)
    : engineStateTree(engineStateIn) 
    , engineState(nameIn, engineStateIn, nullptr)
    , controller(controllerIn)
    , name(nameIn)
    , layoutBeforeStart(*controllerIn->getMappingData())
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

void LumatoneSandboxGameBase::readQueue(LumatoneAction** buffer, int& numActions)
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

void LumatoneSandboxGameBase::end()
{
    reset(true);
    queueLayout(layoutBeforeStart);
}

LumatoneKeyContext LumatoneSandboxGameBase::getKeyAt(int boardIndex, int keyIndex) const
{
    return controller->getKeyContext(boardIndex, keyIndex);
}

void LumatoneSandboxGameBase::addToQueue(LumatoneAction* action)
{
    // jassert(action != nullptr);
    if (action == nullptr)
        return;

    if (queueSize < 0)
        queueSize = 1;
    else if (queueSize < MAX_QUEUE_SIZE)
        queueSize += 1;
    else
        delete queuedActions[getQueuePtr()];

    queuedActions[getQueuePtr()] = action;
}

void LumatoneSandboxGameBase::queueLayout(const LumatoneLayout& layout)
{
    for (int i = 0; i < controller->getNumBoards(); i++)
    {
        const LumatoneBoard* newBoard = layout.readBoard(i);
        addToQueue(new LumatoneEditAction::SectionEditAction(controller, i, *newBoard));
    }
}

LumatoneLayout LumatoneSandboxGameBase::getIdentityLayout(bool resetColors)
{
    LumatoneLayout identity = LumatoneLayout::IdentityMapping(controller->getNumBoards(), controller->getOctaveBoardSize());

    if (resetColors == false)
    {
        for (int b = 0; b < layoutBeforeStart.getNumBoards(); b++)
        {
            for (int k = 0; k < layoutBeforeStart.getOctaveBoardSize(); k++)
            {
                auto layoutKey = layoutBeforeStart.readKey(b, k);
                auto idKey = identity.getKey(b, k);
                idKey->colour = layoutKey->colour;
            }
        }
    }

    return identity;
}

LumatoneContext LumatoneSandboxGameBase::getIdentityWithLayoutContext(bool resetColors)
{
    LumatoneLayout identityLayout = getIdentityLayout(resetColors);
    LumatoneContext layoutContext = LumatoneContext(layoutBeforeStart);

    juce::Array<int> midiChannelContextMap;
    juce::Array<int> midiNoteContextMap;

    for (int b = 0; b < layoutBeforeStart.getNumBoards(); b++)
    {
        for (int k = 0; k < layoutBeforeStart.getOctaveBoardSize(); k++)
        {
            auto layoutKey = layoutBeforeStart.readKey(b, k);
            midiChannelContextMap.add(layoutKey->channelNumber);
            midiNoteContextMap.add(layoutKey->noteNumber);
        }
    }

    LumatoneContext context = LumatoneContext(identityLayout);
    context.setMappedMidiChannels(midiChannelContextMap);
    context.setMappedMidiNotes(midiNoteContextMap);

    return context;
}
