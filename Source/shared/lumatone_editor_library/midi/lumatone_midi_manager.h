/*
  ==============================================================================

    lumatone_midi_manager.h
    Created: 24 Sep 2023
    Author:  vsicurella

    Combine Device MIDI with app generated MIDI states and provide 
    a listener interface to react to them inclusively or exclusively.

  ==============================================================================
*/


#ifndef LUMATONE_MIDI_MANAGER_H
#define LUMATONE_MIDI_MANAGER_H

#include "./lumatone_midi_state.h"

#include "../data/application_state.h"
#include "../data/lumatone_context.h"

#include "../lumatone_midi_driver/firmware_driver_listener.h"

// #include "../../debug/LumatoneSandboxLogger.h"

class LumatoneFirmwareDriver;

namespace LumatoneEditor
{
    class MidiListener;
}

class LumatoneApplicationMidiController : public LumatoneMidiState::Listener
                                        , protected LumatoneFirmwareDriverListener
                                        // , private LumatoneSandboxLogger
{
public:

    LumatoneApplicationMidiController(LumatoneApplicationState state, LumatoneFirmwareDriver& firmwareDriver);
    virtual ~LumatoneApplicationMidiController() override;

    LumatoneMidiState* getDeviceMidiState() { return &deviceMidiState; }
    LumatoneMidiState* getAppMidiState() { return &appMidiState; }

    void sendMidiMessage(const juce::MidiMessage msg);
    void sendMidiMessageInContext(const juce::MidiMessage msg, int boardIndex, int keyIndex);

    void sendKeyNoteOn(int boardIndex, int keyIndex, juce::uint8 velocity, bool ignoreContext=false);
    void sendKeyNoteOff(int boardIndex, int keyIndex, bool ignoreContext=false);

    void allNotesOff(int midiChannel);
    void allNotesOff();

private:
    juce::ListenerList<LumatoneEditor::MidiListener> listeners;

public:
    void addMidiListener(LumatoneEditor::MidiListener* listenerIn) { listeners.add(listenerIn); }
    void removeMidiListener(LumatoneEditor::MidiListener* listenerIn) { listeners.remove(listenerIn); }

protected:

    // LumatoneMidiState::Listener implementation
    void handleLumatoneMidi(LumatoneMidiState* midiState, const juce::MidiMessage& msg) override;

    void handleNoteOn(LumatoneMidiState* midiState, int midiChannel, int midiNote, juce::uint8 velocity) override;
    void handleNoteOff(LumatoneMidiState* midiState, int midiChannel, int midiNote) override;
    void handleAftertouch(LumatoneMidiState* midiState, int midiChannel, int midiNote, juce::uint8 aftertouch) override;
    void handleController(LumatoneMidiState* midiState, int midiChannel, int midiNote, juce::uint8 controller) override;


    // LumatoneFirmwareDriver::Collector implementation
	void midiMessageReceived(juce::MidiInput* source, const juce::MidiMessage& message) override;
    void midiMessageSent(juce::MidiOutput* target, const juce::MidiMessage& message) override {}
    void midiSendQueueSize(int size) override {}
    void noAnswerToMessage(juce::MidiDeviceInfo expectedDevice, const juce::MidiMessage& message) override {}

private:

    LumatoneApplicationState appState;
    LumatoneFirmwareDriver& firmwareDriver;

    LumatoneMidiState deviceMidiState;
    LumatoneMidiState appMidiState;

    int quarterNoteInterval = 24;
};


#endif
