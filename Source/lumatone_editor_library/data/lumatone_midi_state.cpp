/*
  ==============================================================================

    lumatone_midi_state.cpp
    Created: 6 Aug 2023 8:58:06am
    Author:  Vincenzo

  ==============================================================================
*/

#include "lumatone_midi_state.h"

LumatoneMidiState::LumatoneMidiState(LumatoneState stateIn) 
    : state(stateIn)
{
    reset();
}

void LumatoneMidiState::reset()
{
    juce::MidiKeyboardState::reset();

    for (int ch = 0; ch < 16; ch++)
    {
        for (int n = 0; n < 128; n++)
        {
            velocityStates[ch][n] = 0;
        }
    }

    eventsToAdd.clear();
}

void LumatoneMidiState::processNextMidiEvent(const juce::MidiMessage& message)
{
    if (message.isNoteOn())
    {
        noteOnInternal(message, message.getChannel(), message.getNoteNumber(), message.getVelocity());
    }
    else if (message.isNoteOn())
    {
        noteOffInternal(message, message.getChannel(), message.getNoteNumber(), message.getVelocity());
    }
    else if (message.isAftertouch())
    {
        aftertouchInternal(message.getChannel(), message.getNoteNumber(), message.getAfterTouchValue());
    }
    else if (message.isController())
    {
        controllerInternal(message.getChannel(), message.getControllerNumber(), message.getControllerValue());
    }
    else if (message.isAllNotesOff())
    {
        for (int i = 0; i < 128; ++i)
            noteOff(message.getChannel(), i, (juce::uint8)0);
    }
}

void LumatoneMidiState::processNextMidiBuffer(juce::MidiBuffer& buffer, int startSample, int numSamples, bool injectIndirectEvents)
{
    const juce::ScopedLock sl(lock);

    for (const auto metadata : buffer)
        processNextMidiEvent(metadata.getMessage());

    if (injectIndirectEvents)
    {
        const int firstEventToAdd = eventsToAdd.getFirstEventTime();
        const double scaleFactor = numSamples / (double)(eventsToAdd.getLastEventTime() + 1 - firstEventToAdd);

        for (const auto metadata : eventsToAdd)
        {
            const auto pos = juce::jlimit(0, numSamples - 1, juce::roundToInt((metadata.samplePosition - firstEventToAdd) * scaleFactor));
            buffer.addEvent(metadata.getMessage(), startSample + pos);
        }
    }

    eventsToAdd.clear();
}

void LumatoneMidiState::noteOn(const int midiChannel, const int midiNote, juce::uint8 velocity)
{
    jassert(midiChannel > 0 && midiChannel <= 16);
    jassert(juce::isPositiveAndBelow(midiNote, 128));

    if (juce::isPositiveAndBelow(midiNote, 128))
    {
        const int timeNow = (int)juce::Time::getMillisecondCounter();

        auto msg = juce::MidiMessage::noteOn(midiChannel, midiNote, velocity);
        eventsToAdd.addEvent(msg, timeNow);
        eventsToAdd.clear(0, timeNow - 500);

        noteOnInternal(msg, midiChannel, midiNote, velocity);
    }
}

void LumatoneMidiState::noteOff(const int midiChannel, const int midiNote, const juce::uint8 velocity)
{
    jassert(midiChannel > 0 && midiChannel <= 16);
    jassert(juce::isPositiveAndBelow(midiNote, 128));

    if (juce::isPositiveAndBelow(midiNote, 128))
    {
        const int timeNow = (int)juce::Time::getMillisecondCounter();

        auto msg = juce::MidiMessage::noteOn(midiChannel, midiNote, velocity);
        eventsToAdd.addEvent(msg, timeNow);
        eventsToAdd.clear(0, timeNow - 500);

        noteOffInternal(msg, midiChannel, midiNote, velocity);
    }
}

void LumatoneMidiState::aftertouch(const int midiChannel, const int midiNote, const juce::uint8 aftertouch)
{
    jassert(midiChannel > 0 && midiChannel <= 16);
    jassert(juce::isPositiveAndBelow(midiNote, 128));

    if (juce::isPositiveAndBelow(midiNote, 128))
    {
        const int timeNow = (int)juce::Time::getMillisecondCounter();

        auto msg = juce::MidiMessage::aftertouchChange(midiChannel, midiNote, aftertouch);
        eventsToAdd.addEvent(msg, timeNow);
        eventsToAdd.clear(0, timeNow - 500);

        aftertouchInternal(midiChannel, midiNote, aftertouch);
    }

}

void LumatoneMidiState::controller(const int midiChannel, const int number, const juce::uint8 value)
{
    jassert(midiChannel > 0 && midiChannel <= 16);
    jassert(juce::isPositiveAndBelow(number, 128));

    if (juce::isPositiveAndBelow(number, 128))
    {
        const int timeNow = (int)juce::Time::getMillisecondCounter();

        auto msg = juce::MidiMessage::controllerEvent(midiChannel, number, value);
        eventsToAdd.addEvent(msg, timeNow);
        eventsToAdd.clear(0, timeNow - 500);

        controllerInternal(midiChannel, number, value);
    }
}


void LumatoneMidiState::addListener(LumatoneMidiState::Listener* listener)
{
    midiListeners.add(listener);
}

void LumatoneMidiState::removeListener(LumatoneMidiState::Listener* listener)
{
    midiListeners.remove(listener);
}

void LumatoneMidiState::noteOnInternal(const juce::MidiMessage& msg, int midiChannel, int midiNote, juce::uint8 velocity)
{
    juce::MidiKeyboardState::processNextMidiEvent(msg);

    velocityStates[midiChannel - 1][midiNote] = velocity;
    midiListeners.call(&LumatoneMidiState::Listener::handleNoteOn, this, midiChannel, midiNote, velocity);
}

void LumatoneMidiState::noteOffInternal(const juce::MidiMessage& msg, int midiChannel, int midiNote, juce::uint8 velocity)
{
    juce::MidiKeyboardState::processNextMidiEvent(msg);

    velocityStates[midiChannel - 1][midiNote] = velocity;
    midiListeners.call(&LumatoneMidiState::Listener::handleNoteOff, this,  midiChannel, midiNote);
}

void LumatoneMidiState::aftertouchInternal(int midiChannel, int midiNote, juce::uint8 aftertouch)
{
    aftertouchStates[midiChannel - 1][midiNote] = aftertouch;
    midiListeners.call(&LumatoneMidiState::Listener::handleAftertouch, this, midiChannel, midiNote, aftertouch);
}

void LumatoneMidiState::controllerInternal(int midiChannel, int midiNote, juce::uint8 value)
{
    controllerStates[midiChannel - 1][midiNote] = value;
    midiListeners.call(&LumatoneMidiState::Listener::handleController, this, midiChannel, midiNote, value);
}

