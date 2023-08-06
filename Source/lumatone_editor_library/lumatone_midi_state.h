/*
  ==============================================================================

    lumatone_midi_state.h
    Created: 6 Aug 2023 8:58:06am
    Author:  Vincenzo

  ==============================================================================
*/

#pragma once
#include "./ApplicationListeners.h"
#include "./lumatone_state.h"


class LumatoneMidiState : private juce::MidiKeyboardState
{
public:

    LumatoneMidiState(LumatoneState state);
    virtual ~LumatoneMidiState() {}

    void reset();

    bool isNoteOn(int midiChannel, int midiNoteNumber) const noexcept { return juce::MidiKeyboardState::isNoteOn(midiChannel, midiNoteNumber); }
    bool isNoteOnForChannels(int midiChannelMask, int midiNoteNumber) const noexcept { return juce::MidiKeyboardState::isNoteOnForChannels(midiChannelMask, midiNoteNumber); }
    void allNotesOff(int midiChannel) { return juce::MidiKeyboardState::allNotesOff(midiChannel); }

    void processNextMidiEvent(const juce::MidiMessage& message);
    void processNextMidiBuffer(juce::MidiBuffer& buffer, int startSample, int numSamples, bool injectIndirectEvents);

public:

    void noteOn(const int midiChannel, const int midiNote, const juce::uint8 velocity);
    void noteOff(const int midiChannel, const int midiNote, const juce::uint8 velocity);
    void aftertouch(const int midiChannel, const int midiNote, const juce::uint8 aftertouch);
    void controller(const int midiChannel, const int number, const juce::uint8 value);

public:

    class Listener
    {
    public:
        virtual ~Listener() {}

        virtual void handleNoteOn(LumatoneMidiState* midiState, int midiChannel, int midiNote, juce::uint8 velocity) {}
        virtual void handleNoteOff(LumatoneMidiState* midiState, int midiChannel, int midiNote) {}
        virtual void handleAftertouch(LumatoneMidiState* midiState, int midiChannel, int midiNote, juce::uint8 aftertouch) {}
        virtual void handleController(LumatoneMidiState* midiState, int midiChannel, int midiNote, juce::uint8 controller) {}
    };

public:
    void addListener(LumatoneMidiState::Listener* listener);
    void removeListener(LumatoneMidiState::Listener* listener);
private:
    juce::ListenerList<Listener> midiListeners;

protected:

    virtual void noteOnInternal(const juce::MidiMessage& msg, int midiChannel, int midiNote, juce::uint8 velocity);
    virtual void noteOffInternal(const juce::MidiMessage& msg, int midiChannel, int midiNote, juce::uint8 velocity);
    virtual void aftertouchInternal(int midiChannel, int midiNote, juce::uint8 aftertouch);
    // virtual void lumatouchInternal(int midiChannel, int midiNote, juce::uint8 lumatouch);
    virtual void controllerInternal(int midiChannel, int midiNote, juce::uint8 value);

protected:
    LumatoneState state;

private:

    juce::CriticalSection lock;
    juce::MidiBuffer eventsToAdd;

    juce::uint8 velocityStates[16][128];
    juce::uint8 aftertouchStates[16][128];
    juce::uint8 controllerStates[16][128];
};