#include "lumatone_midi_manager.h"
#include "./application_state.h"
#include "../listeners/midi_listener.h"

LumatoneApplicationMidiController::LumatoneApplicationMidiController(LumatoneApplicationState stateIn, LumatoneFirmwareDriver& firmwareDriverIn)
    : appState(stateIn)
    , firmwareDriver(firmwareDriverIn)
{
    firmwareDriver.addMessageCollector(this);
}

LumatoneApplicationMidiController::~LumatoneApplicationMidiController()
{
    listeners.clear();
    firmwareDriver.removeMessageCollector(this);
}

void LumatoneApplicationMidiController::sendMidiMessage(const juce::MidiMessage msg)
{
    firmwareDriver.sendMessageNow(msg);
}

void LumatoneApplicationMidiController::sendMidiMessageInContext(const juce::MidiMessage msg)
{
    if (! appState.isContextSet())
    {
        sendMidiMessage(msg);
        return;
    }

    bool setNote = msg.isNoteOnOrOff() || msg.isAftertouch();
    bool setController = msg.isController();

    auto key = appState.getKeyContext(msg.getChannel(), setNote ? msg.getNoteNumber() : msg.getControllerNumber());
        
    juce::MidiMessage newMsg = msg;
    newMsg.setChannel(key.channelNumber);

    if (setNote)
        newMsg.setNoteNumber(key.noteNumber);

    if (setController)
        newMsg = juce::MidiMessage::controllerEvent(key.channelNumber, key.noteNumber, msg.getControllerValue());
        
    sendMidiMessage(newMsg);
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

    if (message.isNoteOn())
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
}
