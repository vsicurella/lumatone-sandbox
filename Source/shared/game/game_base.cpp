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

void LumatoneSandboxGameBase::noteOff(int midiChannel, int midiNote)
{
    auto msg = juce::MidiMessage::noteOff(midiChannel, midiNote);
    controller->sendMidiMessage(msg);
}

void LumatoneSandboxGameBase::allNotesOff(int midiChannel)
{
    auto msg = juce::MidiMessage::allNotesOff(midiChannel);
    controller->sendMidiMessage(msg);
}

void LumatoneSandboxGameBase::allNotesOff()
{
    for (int ch = 1; ch <=16; ch++)
        allNotesOff(ch);
}

void LumatoneSandboxGameBase::noteOn(int midiChannel, int midiNote, juce::uint8 velocity)
{
    auto msg = juce::MidiMessage::noteOn(midiChannel, midiNote, velocity);
    controller->sendMidiMessage(msg);
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

LumatoneLayout LumatoneSandboxGameBase::queueIdentityLayout(bool resetColors)
{
    LumatoneLayout layout = LumatoneLayout::IdentityMapping(controller->getNumBoards(), controller->getOctaveBoardSize());
    queueLayout(layout);
    return layout;
}
