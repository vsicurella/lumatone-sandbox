#include "lumatone_midi_manager.h"
#include "./application_state.h"
#include "../listeners/midi_listener.h"

#include "../lumatone_midi_driver/lumatone_midi_driver.h"

LumatoneApplicationMidiController::LumatoneApplicationMidiController(LumatoneApplicationState stateIn, LumatoneFirmwareDriver& firmwareDriverIn)
    : appState("LumatoneApplicationMidiController", stateIn)
    , firmwareDriver(firmwareDriverIn)
    // , LumatoneSandboxLogger("LumatoneApplicationMidiController")
{
    firmwareDriver.addDriverListener(this);
}

LumatoneApplicationMidiController::~LumatoneApplicationMidiController()
{
    listeners.clear();
    firmwareDriver.removeDriverListener(this);
}

void LumatoneApplicationMidiController::sendMidiMessage(const juce::MidiMessage msg)
{
    // logInfo("sendMidiMessage", msg.getDescription());
    appMidiState.processNextMidiEvent(msg);
    firmwareDriver.sendMessageNow(msg);
}

void LumatoneApplicationMidiController::sendMidiMessageInContext(const juce::MidiMessage msg, int boardIndex, int keyIndex)
{
    if (! appState.isContextSet())
    {
        sendMidiMessage(msg);
        return;
    }

    bool setNote = msg.isNoteOnOrOff() || msg.isAftertouch();
    bool setController = msg.isController();

    // auto key = appState.getKeyContext(msg.getChannel(), setNote ? msg.getNoteNumber() : msg.getControllerNumber());
    auto key = appState.getKeyContext(boardIndex, keyIndex);
        
    juce::MidiMessage newMsg = msg;
    newMsg.setChannel(key.channelNumber);

    if (setNote)
        newMsg.setNoteNumber(key.noteNumber);

    if (setController)
        newMsg = juce::MidiMessage::controllerEvent(key.channelNumber, key.noteNumber, msg.getControllerValue());
        
    sendMidiMessage(newMsg);
}

void LumatoneApplicationMidiController::sendKeyNoteOn(int boardIndex, int keyIndex, juce::uint8 velocity, bool ignoreContext)
{
    bool useContext = !ignoreContext && appState.isContextSet();

    // const LumatoneKey* keyData = appState.getKey(boardIndex, keyIndex);
    // LumatoneKeyContext context = appState.getKeyContext(boardIndex, keyIndex);
    // LumatoneKey key = *keyData;
    // LumatoneKey ctx = appState.getKeyContext(boardIndex, keyIndex);

// #if JUCE_DEBUG
//     LumatoneKey key;
//     if (useContext)
//         key = (LumatoneKey)appState.getKeyContext(boardIndex, keyIndex);
//     else 
//         key = appState.getKeyContext(boardIndex, keyIndex);
// #else
//     LumatoneKey key = (useContext)
//         ? (LumatoneKey)appState.getKeyContext(boardIndex, keyIndex);
//         : appState.getKeyContext(boardIndex, keyIndex); 
// #endif

    LumatoneKey key
    #if JUCE_DEBUG
    ; if (useContext)
        key = 
    #else 
       = (useContext) ? 
    #endif
        (LumatoneKey)appState.getKeyContext(boardIndex, keyIndex)
    #if JUCE_DEBUG
    ; else
        key = 
    #else
        :
    #endif
        *appState.getKey(boardIndex, keyIndex);
    

    jassert(key.channelNumber > 0 && key.channelNumber <= 16 && key.noteNumber >= 0 && key.noteNumber < 128);
        
    juce::MidiMessage msg = juce::MidiMessage::noteOn(key.channelNumber, key.noteNumber, velocity);
    sendMidiMessage(msg);
}

void LumatoneApplicationMidiController::sendKeyNoteOff(int boardIndex, int keyIndex, bool ignoreContext)
{
    bool useContext = !ignoreContext && appState.isContextSet();

    LumatoneKey key = (useContext)
        ? (LumatoneKey)appState.getKeyContext(boardIndex, keyIndex)
        : *appState.getKey(boardIndex, keyIndex);

    jassert(key.channelNumber > 0 && key.channelNumber <= 16 && key.noteNumber >= 0 && key.noteNumber < 128);
    
    juce::MidiMessage msg = juce::MidiMessage::noteOff(key.channelNumber, key.noteNumber);
    sendMidiMessage(msg);
}

void LumatoneApplicationMidiController::allNotesOff(int midiChannel)
{
    // auto msg = juce::MidiMessage::allNotesOff(midiChannel);
    // sendMidiMessage(msg);

    for (int i = 0; i < 128; i++)
    {
        if (appMidiState.isNoteOn(midiChannel, i))
        {
            auto msg = juce::MidiMessage::noteOff(midiChannel, i);
            sendMidiMessage(msg);
        }
        
    }

}

void LumatoneApplicationMidiController::allNotesOff()
{
    for (int ch = 1; ch <=16; ch++)
        allNotesOff(ch);
}

void LumatoneApplicationMidiController::handleLumatoneMidi(LumatoneMidiState *midiState, const juce::MidiMessage &msg)
{
    appMidiState.processNextMidiEvent(msg);
}

void LumatoneApplicationMidiController::handleNoteOn(LumatoneMidiState *midiState, int midiChannel, int midiNote, juce::uint8 velocity)
{
    listeners.call(&LumatoneEditor::MidiListener::handleAnyNoteOn, midiChannel, midiNote, velocity);
    listeners.call(&LumatoneEditor::MidiListener::handleAppNoteOn, midiChannel, midiNote, velocity);
}

void LumatoneApplicationMidiController::handleNoteOff(LumatoneMidiState* midiState, int midiChannel, int midiNote)
{
    listeners.call(&LumatoneEditor::MidiListener::handleAnyNoteOff, midiChannel, midiNote);
    listeners.call(&LumatoneEditor::MidiListener::handleAppNoteOff, midiChannel, midiNote);
}

void LumatoneApplicationMidiController::handleAftertouch(LumatoneMidiState* midiState, int midiChannel, int midiNote, juce::uint8 aftertouch)
{
    listeners.call(&LumatoneEditor::MidiListener::handleAnyAftertouch, midiChannel, midiNote, aftertouch);
    listeners.call(&LumatoneEditor::MidiListener::handleAppAftertouch, midiChannel, midiNote, aftertouch);
}

void LumatoneApplicationMidiController::handleController(LumatoneMidiState* midiState, int midiChannel, int midiNote, juce::uint8 controller)
{
    listeners.call(&LumatoneEditor::MidiListener::handleAnyController, midiChannel, midiNote, controller);
    listeners.call(&LumatoneEditor::MidiListener::handleAppController, midiChannel, midiNote, controller);
}

// Always will be coming from the device
void LumatoneApplicationMidiController::midiMessageReceived(juce::MidiInput *source, const juce::MidiMessage &message)
{
    if (message.isSysEx())
        return;

    deviceMidiState.processNextMidiEvent(message);
    appMidiState.processNextMidiEvent(message);

    if (message.isMidiClock())
    {
        listeners.call(&LumatoneEditor::MidiListener::handleMidiClock, quarterNoteInterval);
    }
    else if (message.isNoteOn())
    {
        listeners.call(&LumatoneEditor::MidiListener::handleAnyNoteOn, message.getChannel(), message.getNoteNumber(), message.getVelocity());
        listeners.call(&LumatoneEditor::MidiListener::handleDeviceNoteOn, message.getChannel(), message.getNoteNumber(), message.getVelocity());
    }
    else if (message.isNoteOff())
    {
        listeners.call(&LumatoneEditor::MidiListener::handleAnyNoteOff, message.getChannel(), message.getNoteNumber());
        listeners.call(&LumatoneEditor::MidiListener::handleDeviceNoteOff, message.getChannel(), message.getNoteNumber());
    }
    else if (message.isAftertouch())
    {
        listeners.call(&LumatoneEditor::MidiListener::handleAnyAftertouch, message.getChannel(), message.getNoteNumber(), (juce::uint8) message.getAfterTouchValue());
        listeners.call(&LumatoneEditor::MidiListener::handleDeviceAftertouch, message.getChannel(), message.getNoteNumber(), (juce::uint8) message.getAfterTouchValue());
    }
    else if (message.isController())
    {
        listeners.call(&LumatoneEditor::MidiListener::handleAnyController, message.getChannel(), message.getControllerNumber(), (juce::uint8) message.getControllerValue());
        listeners.call(&LumatoneEditor::MidiListener::handleDeviceController, message.getChannel(), message.getControllerNumber(), (juce::uint8) message.getControllerValue());
    }
    else if (message.isSustainPedalOn() || message.isSoftPedalOff())
    {
        bool isToggled = message.isSustainPedalOn();
        listeners.call(&LumatoneEditor::MidiListener::handleSustain, isToggled);
    }
}
