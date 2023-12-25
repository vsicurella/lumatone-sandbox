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
    // virtual void handleAnyNoteOn(int midiChannel, int midiNote, juce::uint8 velocity) {}
    
    /*
        Callback for device or app MIDI note offs.
        If this needs to invoke any intensive code, be sure to do it on a message thread.
    */
    // virtual void handleAnyNoteOff(int midiChannel, int midiNote) {}
    
    /*
        Callback for device or app MIDI aftertouch updates.
        If this needs to invoke any intensive code, be sure to do it on a message thread.
    */
    // virtual void handleAnyAftertouch(int midiChannel, int midiNote, juce::uint8 value) {}
    
    /*
        Callback for device or app MIDI controller updates.
        If this needs to invoke any intensive code, be sure to do it on a message thread.
    */
    // virtual void handleAnyController(int midiChannel, int ccNum, juce::uint8 volocity) {}

    /*
        Callback for device MIDI note ons.
        If this needs to invoke any intensive code, be sure to do it on a message thread.
    */
    virtual void handleNoteOn(int midiChannel, int midiNote, juce::uint8 velocity) {}

    /*
        Callback for device MIDI note offs.
        If this needs to invoke any intensive code, be sure to do it on a message thread.
    */
    virtual void handleNoteOff(int midiChannel, int midiNote) {}

    /*
        Callback for device MIDI aftertouch updates.
        If this needs to invoke any intensive code, be sure to do it on a message thread.
    */
    virtual void handleAftertouch(int midiChannel, int midiNote, juce::uint8 value) {}

    /*
        Callback for device MIDI controller updates.
        If this needs to invoke any intensive code, be sure to do it on a message thread.
    */
    virtual void handleController(int midiChannel, int ccNum, juce::uint8 value) {}

    /*
        Callback for components that use a MIDI Clock
    */
    virtual void handleMidiClock(int quarterNoteDuration) {}

    /*
        Callback for sustain pedal messages
    */
    virtual void handleSustain(bool toggled) {}



    // /*
    //     Callback for virtual key pressed.
    // */
    // virtual void handleKeyDown(int keyNum) {}

    // /*
    //     Callback for virtual key released.
    // */
    // virtual void handleKeyUp(int keyNum) {}

    // /*
    //     Callback for virtual key dragged.
    // */
    // virtual void handleKeyHold(int key, float xDistance, float yDistance) {}

};

}

#endif
