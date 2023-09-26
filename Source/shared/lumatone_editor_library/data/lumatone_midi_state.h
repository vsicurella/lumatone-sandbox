/*
  ==============================================================================

    lumatone_midi_state.h
    Created: 6 Aug 2023 8:58:06am
    Author:  Vincenzo

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class LumatoneMidiState : private juce::MidiKeyboardState
{
public:

    LumatoneMidiState();
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

        virtual void handleLumatoneMidi(LumatoneMidiState* midiState, const juce::MidiMessage& msg) {}

        virtual void handleNoteOn(LumatoneMidiState* midiState, int midiChannel, int midiNote, juce::uint8 velocity) {}
        virtual void handleNoteOff(LumatoneMidiState* midiState, int midiChannel, int midiNote) {}
        virtual void handleAftertouch(LumatoneMidiState* midiState, int midiChannel, int midiNote, juce::uint8 aftertouch) {}
        virtual void handleController(LumatoneMidiState* midiState, int midiChannel, int midiNote, juce::uint8 controller) {}
    };

public:
    void addMidiStateListener(LumatoneMidiState::Listener* listener);
    void removeMidiStateListener(LumatoneMidiState::Listener* listener);
private:
    juce::ListenerList<Listener> midiListeners;

protected:

    virtual void noteOnInternal(const juce::MidiMessage& msg, int midiChannel, int midiNote, juce::uint8 velocity);
    virtual void noteOffInternal(const juce::MidiMessage& msg, int midiChannel, int midiNote, juce::uint8 velocity);
    virtual void aftertouchInternal(int midiChannel, int midiNote, juce::uint8 aftertouch);
    // virtual void lumatouchInternal(int midiChannel, int midiNote, juce::uint8 lumatouch);
    virtual void controllerInternal(int midiChannel, int midiNote, juce::uint8 value);


private:

    juce::CriticalSection lock;
    juce::MidiBuffer eventsToAdd;

    juce::uint8 velocityStates[16][128];
    juce::uint8 aftertouchStates[16][128];
    juce::uint8 controllerStates[16][128];
};