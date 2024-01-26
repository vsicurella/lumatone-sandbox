/*
  ==============================================================================

    lumatone_event_manager.h
    Created: 5 Jun 2023 8:19:04pm
    Author:  Vincenzo

  ==============================================================================
*/

#pragma once

#include "../data/application_state.h"
#include "../midi/lumatone_midi_state.h"
#include "../lumatone_midi_driver/firmware_driver_listener.h"
#include "../listeners/firmware_listener.h"

class LumatoneEventManager : public LumatoneApplicationState
                           , public LumatoneMidiState
                           , private LumatoneApplicationState::Controller
                           , private LumatoneFirmwareDriverListener
                           , private LumatoneEditor::FirmwareListener
                           , private juce::Timer
{

public:
    LumatoneEventManager(LumatoneFirmwareDriver& midiDriver, const LumatoneApplicationState& stateIn);
    ~LumatoneEventManager() override;

private:

    void timerCallback() override;

protected:
    //============================================================================
    // Implementation of LumatoneFirmwareDriver::Listener

    virtual void midiMessageReceived(juce::MidiInput* source, const juce::MidiMessage& midiMessage) override;
    virtual void midiMessageSent(juce::MidiOutput* target, const juce::MidiMessage& midiMessage) override;
    virtual void midiSendQueueSize(int queueSize) override;
    //virtual void generalLogMessage(juce::String textMessage, HajuErrorVisualizer::ErrorLevel errorLevel) override;
    virtual void noAnswerToMessage(juce::MidiDeviceInfo expectedDevice, const juce::MidiMessage& midiMessage) override;

    //============================================================================
    // Implementation of LumatoneEditor::FirmwareListener
    void octaveColourConfigReceived(int boardId, juce::uint8 rgbFlag, const int* colourData) override;
    void octaveChannelConfigReceived(int octaveIndex, const int* channelData) override;
    void octaveNoteConfigReceived(int octaveIndex, const int* noteData) override;
    void keyTypeConfigReceived(int boardId, const int* keyTypeData) override;

    void macroButtonColoursReceived(juce::Colour inactiveColour, juce::Colour activeColour) override;

    void presetFlagsReceived(LumatoneFirmware::PresetFlags presetFlags) override;

private:
    juce::ListenerList<LumatoneEditor::FirmwareListener> firmwareListeners;
public:
    void    addFirmwareListener(LumatoneEditor::FirmwareListener* listenerIn) { firmwareListeners.add(listenerIn); }
    void removeFirmwareListener(LumatoneEditor::FirmwareListener* listenerIn) { firmwareListeners.remove(listenerIn); }

//private:
//    ListenerList<MidiListener> midiListeners;
//public:
//    void    addMidiListener(MidiListener* listenerIn) { midiListeners.add(listenerIn); }
//    void removeMidiListener(MidiListener* listenerIn) { midiListeners.remove(listenerIn); }

private:
    //============================================================================
    // Lower-level message unpacking and handling

    FirmwareSupport::Error handleOctaveConfigResponse(
        const juce::MidiMessage& midiMessage,
        std::function <FirmwareSupport::Error(const juce::MidiMessage&, int&, juce::uint8, int*)> unpackFunction,
        std::function <void(int, void*)> callbackFunctionIfNoError = {}
    );

    FirmwareSupport::Error handleTableConfigResponse(
        const juce::MidiMessage& midiMessage,
        std::function<FirmwareSupport::Error(const juce::MidiMessage&, int*)> unpackFunction,
        std::function <void(void*)> callbackFunctionIfNoError = {}
    );

    FirmwareSupport::Error handleLEDConfigResponse(const juce::MidiMessage& midiMessage);

    FirmwareSupport::Error handleChannelConfigResponse(const juce::MidiMessage& midiMessage);

    FirmwareSupport::Error handleNoteConfigResponse(const juce::MidiMessage& midiMessage);

    FirmwareSupport::Error handleKeyTypeConfigResponse(const juce::MidiMessage& midiMessage);

    FirmwareSupport::Error handleVelocityConfigResponse(const juce::MidiMessage& midiMessage);

    FirmwareSupport::Error handleAftertouchConfigResponse(const juce::MidiMessage& midiMessage);

    FirmwareSupport::Error handleVelocityIntervalConfigResponse(const juce::MidiMessage& midiMessage);

    FirmwareSupport::Error handleFaderConfigResponse(const juce::MidiMessage& midiMessage);

    FirmwareSupport::Error handleFaderTypeConfigResponse(const juce::MidiMessage& midiMessage);

    FirmwareSupport::Error handleSerialIdentityResponse(const juce::MidiMessage& midiMessage);

    FirmwareSupport::Error handleFirmwareRevisionResponse(const juce::MidiMessage& midiMessage);

    FirmwareSupport::Error handleLumatouchConfigResponse(const juce::MidiMessage& midiMessage);

    FirmwareSupport::Error handlePingResponse(const juce::MidiMessage& midiMessage);

    FirmwareSupport::Error handlePeripheralCalibrationData(const juce::MidiMessage& midiMessage);
    FirmwareSupport::Error handleExpressionPedalCalibrationData(const juce::MidiMessage& midiMessage);
    FirmwareSupport::Error handleWheelsCalibrationData(const juce::MidiMessage& midiMessage);

    FirmwareSupport::Error handleGetPeripheralChannelResponse(const juce::MidiMessage& midiMessage);

    FirmwareSupport::Error handleGetPresetFlagsResponse(const juce::MidiMessage& midiMessage);

    FirmwareSupport::Error handleGetExpressionPedalSensitivityResponse(const juce::MidiMessage& midiMessage);

    FirmwareSupport::Error handleGetMacroLightIntensityResponse(const juce::MidiMessage& midiMessage);

    void handleMidiDriverError(FirmwareSupport::Error errorToHandle, int commandReceived = -1);

    // Buffer read helpers
    FirmwareSupport::Error getBufferErrorCode(const juce::uint8* sysExData);
    FirmwareSupport::Error handleBufferCommand(const juce::MidiMessage& midiMessage);


private:
    LumatoneFirmwareDriver&     midiDriver;

    const int                   bufferReadTimeoutMs = 30;
    const int                   bufferReadSize = 16;
    bool                        bufferReadRequested = false;

    std::atomic<int>            readQueueSize;
    int                         sendQueueSize = 0;

    int                         verbose = 0;
};