#include "lumatone_midi_manager.h"
#include "../listeners/midi_listener.h"

#include "../lumatone_midi_driver/lumatone_midi_driver.h"

LumatoneApplicationMidi::LumatoneApplicationMidi(const LumatoneApplicationState& stateIn, LumatoneFirmwareDriver& firmwareDriverIn)
    : appState("LumatoneApplicationMidi", stateIn)
    , firmwareDriver(firmwareDriverIn)
    // , LumatoneSandboxLogger("LumatoneApplicationMidi")
{
    firmwareDriver.addDriverListener(this);
}

LumatoneApplicationMidi::~LumatoneApplicationMidi()
{
    listeners.clear();
    firmwareDriver.removeDriverListener(this);
}

void LumatoneApplicationMidi::Controller::sendMidiMessage(const juce::MidiMessage msg)
{
    // logInfo("sendMidiMessage", msg.getDescription());
    appMidi.appMidiState.processNextMidiEvent(msg);
    appMidi.firmwareDriver.sendMessageNow(msg);
}

void LumatoneApplicationMidi::Controller::sendMidiMessageInContext(const juce::MidiMessage msg, int boardIndex, int keyIndex)
{
    if (! appMidi.appState.isContextSet())
    {
        sendMidiMessage(msg);
        return;
    }

    bool setNote = msg.isNoteOnOrOff() || msg.isAftertouch();
    bool setController = msg.isController();

    // auto key = appMidi.appState.getKeyContext(msg.getChannel(), setNote ? msg.getNoteNumber() : msg.getControllerNumber());
    auto key = appMidi.appState.getKeyContext(boardIndex, keyIndex);

    juce::MidiMessage newMsg = msg;
    newMsg.setChannel(key.getMidiChannel());

    if (setNote)
        newMsg.setNoteNumber(key.getMidiNumber());

    if (setController)
        newMsg = juce::MidiMessage::controllerEvent(key.getMidiChannel(), key.getMidiNumber(), msg.getControllerValue());

    sendMidiMessage(newMsg);
}

void LumatoneApplicationMidi::Controller::sendKeyNoteOn(int boardIndex, int keyIndex, juce::uint8 velocity, bool ignoreContext)
{
    bool useContext = !ignoreContext && appMidi.appState.isContextSet();

    // const LumatoneKey* keyData = appMidi.appState.getKey(boardIndex, keyIndex);
    // LumatoneKeyContext context = appMidi.appState.getKeyContext(boardIndex, keyIndex);
    // LumatoneKey key = *keyData;
    // LumatoneKey ctx = appMidi.appState.getKeyContext(boardIndex, keyIndex);

// #if JUCE_DEBUG
//     LumatoneKey key;
//     if (useContext)
//         key = (LumatoneKey)appMidi.appState.getKeyContext(boardIndex, keyIndex);
//     else
//         key = appMidi.appState.getKeyContext(boardIndex, keyIndex);
// #else
//     LumatoneKey key = (useContext)
//         ? (LumatoneKey)appMidi.appState.getKeyContext(boardIndex, keyIndex);
//         : appMidi.appState.getKeyContext(boardIndex, keyIndex);
// #endif

    LumatoneKey key
    #if JUCE_DEBUG
    ; if (useContext)
        key =
    #else
       = (useContext) ?
    #endif
        (LumatoneKey)appMidi.appState.getKeyContext(boardIndex, keyIndex)
    #if JUCE_DEBUG
    ; else
        key =
    #else
        :
    #endif
        appMidi.appState.getKey(boardIndex, keyIndex);


    jassert(key.getMidiChannel() > 0 && key.getMidiChannel() <= 16 && key.getMidiNumber() >= 0 && key.getMidiNumber() < 128);

    juce::MidiMessage msg = juce::MidiMessage::noteOn(key.getMidiChannel(), key.getMidiNumber(), velocity);
    sendMidiMessage(msg);
}

void LumatoneApplicationMidi::Controller::sendKeyNoteOff(int boardIndex, int keyIndex, bool ignoreContext)
{
    bool useContext = !ignoreContext && appMidi.appState.isContextSet();

    LumatoneKey key = (useContext)
        ? (LumatoneKey)appMidi.appState.getKeyContext(boardIndex, keyIndex)
        : appMidi.appState.getKey(boardIndex, keyIndex);

    jassert(key.getMidiChannel() > 0 && key.getMidiChannel() <= 16 && key.getMidiNumber() >= 0 && key.getMidiNumber() < 128);

    juce::MidiMessage msg = juce::MidiMessage::noteOff(key.getMidiChannel(), key.getMidiNumber());
    sendMidiMessage(msg);
}

void LumatoneApplicationMidi::Controller::allNotesOff(int midiChannel)
{
    // auto msg = juce::MidiMessage::allNotesOff(midiChannel);
    // sendMidiMessage(msg);

    for (int i = 0; i < 128; i++)
    {
        if (appMidi.appMidiState.isNoteOn(midiChannel, i))
        {
            auto msg = juce::MidiMessage::noteOff(midiChannel, i);
            sendMidiMessage(msg);
        }

    }
}

void LumatoneApplicationMidi::Controller::allNotesOff()
{
    for (int ch = 1; ch <=16; ch++)
        allNotesOff(ch);
}

void LumatoneApplicationMidi::handleLumatoneMidi(LumatoneMidiState *midiState, const juce::MidiMessage &msg)
{
    appMidiState.processNextMidiEvent(msg);
}

void LumatoneApplicationMidi::handleNoteOn(LumatoneMidiState *midiState, int midiChannel, int midiNote, juce::uint8 velocity)
{
    listeners.call(&LumatoneEditor::MidiListener::handleNoteOn, midiChannel, midiNote, velocity);
    // listeners.call(&LumatoneEditor::MidiListener::handleKeyDown, midiChannel, midiNote, velocity);
}

void LumatoneApplicationMidi::handleNoteOff(LumatoneMidiState* midiState, int midiChannel, int midiNote)
{
    listeners.call(&LumatoneEditor::MidiListener::handleNoteOff, midiChannel, midiNote);
    // listeners.call(&LumatoneEditor::MidiListener::handleKeyUp, midiChannel, midiNote);
}

void LumatoneApplicationMidi::handleAftertouch(LumatoneMidiState* midiState, int midiChannel, int midiNote, juce::uint8 aftertouch)
{
    listeners.call(&LumatoneEditor::MidiListener::handleAftertouch, midiChannel, midiNote, aftertouch);
    // listeners.call(&LumatoneEditor::MidiListener::handleKeyHold, midiChannel, midiNote, aftertouch);
}

void LumatoneApplicationMidi::handleController(LumatoneMidiState* midiState, int midiChannel, int midiNote, juce::uint8 controller)
{
    listeners.call(&LumatoneEditor::MidiListener::handleController, midiChannel, midiNote, controller);
    // listeners.call(&LumatoneEditor::MidiListener::handleAppController, midiChannel, midiNote, controller);
}

// Always will be coming from the device
void LumatoneApplicationMidi::midiMessageReceived(juce::MidiInput *source, const juce::MidiMessage &message)
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
        // listeners.call(&LumatoneEditor::MidiListener::handleAnyNoteOn, message.getChannel(), message.getNoteNumber(), message.getVelocity());
        listeners.call(&LumatoneEditor::MidiListener::handleNoteOn, message.getChannel(), message.getNoteNumber(), message.getVelocity());
    }
    else if (message.isNoteOff())
    {
        // listeners.call(&LumatoneEditor::MidiListener::handleAnyNoteOff, message.getChannel(), message.getNoteNumber());
        listeners.call(&LumatoneEditor::MidiListener::handleNoteOff, message.getChannel(), message.getNoteNumber());
    }
    else if (message.isAftertouch())
    {
        // listeners.call(&LumatoneEditor::MidiListener::handleAnyAftertouch, message.getChannel(), message.getNoteNumber(), (juce::uint8) message.getAfterTouchValue());
        listeners.call(&LumatoneEditor::MidiListener::handleAftertouch, message.getChannel(), message.getNoteNumber(), (juce::uint8) message.getAfterTouchValue());
    }
    else if (message.isController())
    {
        // listeners.call(&LumatoneEditor::MidiListener::handleAnyController, message.getChannel(), message.getControllerNumber(), (juce::uint8) message.getControllerValue());
        listeners.call(&LumatoneEditor::MidiListener::handleController, message.getChannel(), message.getControllerNumber(), (juce::uint8) message.getControllerValue());
    }
    else if (message.isSustainPedalOn() || message.isSoftPedalOff())
    {
        bool isToggled = message.isSustainPedalOn();
        listeners.call(&LumatoneEditor::MidiListener::handleSustain, isToggled);
    }
}
