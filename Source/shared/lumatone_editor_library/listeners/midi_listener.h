#ifndef LUMATONE_EDITOR_MIDI_LISTENER_H
#define LUMATONE_EDITOR_MIDI_LISTENER_H

#include <JuceHeader.h>
 
namespace LumatoneEditor
{

class MidiListener
{
public:
    /*
        Callback for device or app MIDI note ons.
        If this needs to invoke any intensive code, be sure to do it on a message thread.
    */
    virtual void handleAnyNoteOn(int midiChannel, int midiNote, juce::uint8 velocity) {}
    
    /*
        Callback for device or app MIDI note offs.
        If this needs to invoke any intensive code, be sure to do it on a message thread.
    */
    virtual void handleAnyNoteOff(int midiChannel, int midiNote) {}
    
    /*
        Callback for device or app MIDI aftertouch updates.
        If this needs to invoke any intensive code, be sure to do it on a message thread.
    */
    virtual void handleAnyAftertouch(int midiChannel, int midiNote, juce::uint8 value) {}
    
    /*
        Callback for device or app MIDI controller updates.
        If this needs to invoke any intensive code, be sure to do it on a message thread.
    */
    virtual void handleAnyController(int midiChannel, int ccNum, juce::uint8 volocity) {}

    /*
        Callback for device MIDI note ons.
        If this needs to invoke any intensive code, be sure to do it on a message thread.
    */
    virtual void handleDeviceNoteOn(int midiChannel, int midiNote, juce::uint8 velocity) {}

    /*
        Callback for device MIDI note offs.
        If this needs to invoke any intensive code, be sure to do it on a message thread.
    */
    virtual void handleDeviceNoteOff(int midiChannel, int midiNote) {}

    /*
        Callback for device MIDI aftertouch updates.
        If this needs to invoke any intensive code, be sure to do it on a message thread.
    */
    virtual void handleDeviceAftertouch(int midiChannel, int midiNote, juce::uint8 value) {}

    /*
        Callback for device MIDI controller updates.
        If this needs to invoke any intensive code, be sure to do it on a message thread.
    */
    virtual void handleDeviceController(int midiChannel, int ccNum, juce::uint8 value) {}

    /*
        Callback for app MIDI note ons.
    */
    virtual void handleAppNoteOn(int midiChannel, int midiNote, juce::uint8 velocity) {}

    /*
        Callback for app MIDI note offs.
    */
    virtual void handleAppNoteOff(int midiChannel, int midiNote) {}

    /*
        Callback for app MIDI aftertouch updates.
    */
    virtual void handleAppAftertouch(int midiChannel, int midiNote, juce::uint8 value) {}

    /*
        Callback for app MIDI controller updates.
    */
    virtual void handleAppController(int midiChannel, int ccNum, juce::uint8 value) {}

    /*
        Callback for components that use a MIDI Clock
    */
    virtual void handleMidiClock() {}

    /*
        Callback for sustain pedal messages
    */
    virtual void handleSustain(bool toggled) {}
};

}

#endif
