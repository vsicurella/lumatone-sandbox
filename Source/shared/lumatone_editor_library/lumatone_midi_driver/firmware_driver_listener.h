/*
  ==============================================================================

    firmware_driver_listener.h
    Created: 30 Sep 2023
    Author:  Vincenzo

  ==============================================================================
*/

#ifndef LUMATONE_FIRMWARE_DRIVER_LISTENER_H
#define LUMATONE_FIRMWARE_DRIVER_LISTENER_H

#include <JuceHeader.h>

// juce::MidiMessageCollector version of previous TerpstraMidiDriver::Listener, as to keep the Midi thread lightweight
class LumatoneFirmwareDriverListener : protected juce::MidiMessageCollector
{
    juce::MidiBuffer messagesSentQueue;

public:
    virtual ~LumatoneFirmwareDriverListener() {}
    
    virtual void midiMessageReceived(juce::MidiInput* source, const juce::MidiMessage& message) = 0;
    virtual void midiMessageSent(juce::MidiOutput* target, const juce::MidiMessage& message) = 0;
    virtual void midiSendQueueSize(int size) = 0;
    // virtual void generalLogMessage(juce::String textMessage, ErrorLevel errorLevel) {}

    // Realtime messages before a device is connected - not for heavy processing!
    virtual void noAnswerToMessage(juce::MidiDeviceInfo expectedDevice, const juce::MidiMessage& message) = 0;
//		virtual void testMessageReceived(int testInputIndex, const juce::MidiMessage& midiMessage) {};
};

#endif
