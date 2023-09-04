/*
  ==============================================================================

    LumatoneCommandManager.h
    Created: 17 Apr 2021 10:17:31pm
    Author:  Vincenzo

  ==============================================================================
*/

#pragma once

#include "./ApplicationListeners.h"

#include "LumatoneEventManager.h"
#include "key_update_buffer.h"

#include "./data/application_state.h"
#include "./actions/lumatone_action.h"

//==============================================================================
// Helper class for parsing and comparing (todo) firmware versions


class LumatoneController :  public LumatoneApplicationState,
                            public LumatoneMidiState::Listener,
                            private LumatoneFirmwareDriver::Collector,
                            public LumatoneEditor::StatusListener,
                            public LumatoneEditor::StatusEmitter,
                            public LumatoneEditor::EditorEmitter,
                            protected LumatoneEditor::FirmwareListener
{
public:

    LumatoneController(juce::ValueTree state, LumatoneFirmwareDriver& firmwareDriverIn, juce::UndoManager* undoManager);
    ~LumatoneController();

    juce::ValueTree loadStateProperties(juce::ValueTree stateIn) override;

    void setContext(std::shared_ptr<LumatoneContext> contextIn) override;
    void clearContext() override;
    
    //============================================================================
    // Methods to configure firmware communication parameters

    const FirmwareSupport& getFirmwareSupport() const { return firmwareSupport; }

    juce::Array<juce::MidiDeviceInfo> getMidiInputList() { return firmwareDriver.getMidiInputList(); }
    juce::Array<juce::MidiDeviceInfo> getMidiOutputList() { return firmwareDriver.getMidiOutputList(); }

    int getMidiInputIndex() const { return firmwareDriver.getMidiInputIndex(); }
    int getMidiOutputIndex() const { return firmwareDriver.getMidiOutputIndex(); }

    void setMidiInput(int deviceIndex, bool test = true);
    void setMidiOutput(int deviceIndex, bool test = true);

public:

    bool performAction(LumatoneAction* action, bool undoable = true, bool newTransaction = true);

private:
    bool connectionConfirmed() const { return firmwareDriver.hasDevicesDefined() && currentDevicePairConfirmed; }
    void onConnectionConfirmed();

public:
    //============================================================================
    // Status Listener implementation

    void connectionStateChanged(ConnectionState newState) override;

public:
    //============================================================================
    void sendMidiMessage(const juce::MidiMessage& msg);

private:
    void handleNoteOn(LumatoneMidiState* midiState, int midiChannel, int midiNote, juce::uint8 velocity) override;
    void handleNoteOff(LumatoneMidiState* midiState, int midiChannel, int midiNote) override;

public:
    //============================================================================
    // Combined (hi-level) commands

    // Send all parametrizations of one sub board
    void sendAllParamsOfBoard(int boardId, const LumatoneBoard* boardData, bool signalEditorListeners=true, bool bufferKeyUpdates=false);

    // Send and save a complete key mapping
    void sendCompleteMapping(LumatoneLayout mappingData, bool signalEditorListeners=true, bool bufferKeyUpdates=true);

    // Send request to receive the current mapping of one sub board on the controller
    void sendGetMappingOfBoardRequest(int boardId);

    // Send request to receive the complete current mapping on the controller
    void sendGetCompleteMappingRequest();

    // Send parametrization of one key to the device
    void sendKeyParam(int boardId, int keyIndex, LumatoneKey keyData, bool signalEditorListeners=true, bool bufferKeyUpdates=false);

    void sendSelectionParam(const juce::Array<MappedLumatoneKey>& selection, bool signalEditorListeners=true, bool bufferKeyUpdates=false);

    void sendSelectionColours(const juce::Array<MappedLumatoneKey>& selection, bool signalEditorListeners=true, bool bufferKeyUpdates=false);

    // Send configuration of a certain look up table
    void sendTableConfig(LumatoneConfigTable::TableType velocityCurveType, const juce::uint8* table);

    // Reset configuration of a certain look up table to factory settings
    void resetVelocityConfig(LumatoneConfigTable::TableType velocityCurveType);

    // Ping a device cached in the device list, will primarily use GetSerialIdentity, or Ping if determined version happens to be >= 1.0.9
    unsigned int sendTestMessageToDevice(int deviceIndex, unsigned int pingId);

    // Sends a generic test message to the current device pair to see if we get an expected response
    void testCurrentDeviceConnection();

    //============================================================================
    // Single (mid-level) commands

    // Send note, channel, cc, and fader polarity data
    void sendKeyConfig(int boardId, int keyIndex, const LumatoneKey& noteDataConfig, bool signalEditorListeners=true, bool bufferKeyUpdates=false);

    // Send RGB colour data
    void sendKeyColourConfig(int boardId, int keyIndex, juce::Colour colour, bool signalEditorListeners = true, bool bufferKeyUpdates=false);
    void sendKeyColourConfig(int boardId, int keyIndex, const LumatoneKey& keyColourConfig, bool signalEditorListeners = true, bool bufferKeyUpdates=false);

    // Send expression pedal sensivity
    void sendExpressionPedalSensivity(unsigned char value);

    // Send parametrization of foot controller
    void sendInvertFootController(bool value);

    // Colour for macro button in active state
    void sendMacroButtonActiveColour(juce::String colourAsString);

    // Colour for macro button in inactive state
    void sendMacroButtonInactiveColour(juce::String colourAsString);

    // Send parametrization of light on keystrokes
    void sendLightOnKeyStrokes(bool value);

    // Send a value for a velocity lookup table
    void sendVelocityConfig(const juce::uint8 velocityTable[]);

    // Save velocity config to EEPROM
    void saveVelocityConfig();

    void resetVelocityConfig();

    void setFaderConfig(const juce::uint8 faderTable[]);

    void resetFaderConfig();

    void setAftertouchEnabled(bool isEnabled);

    void startCalibrateAftertouch();

    void setAftertouchConfig(const juce::uint8 aftertouchTable[]);

    void resetAftertouchConfig();

    void setCalibratePitchModWheel(bool startCalibration);

    void setVelocityIntervalConfig(const int velocityIntervalTable[]);

    void getRedLEDConfig(int boardIndex);

    void getGreenLEDConfig(int boardIndex);

    void getBlueLEDConfig(int boardIndex);

    void getChannelConfig(int boardIndex);

    void getNoteConfig(int boardIndex);

    void getKeyTypeConfig(int boardIndex);

    void sendVelocityConfigRequest();

    // CMD 1Eh: Read back the current fader look up table of the keyboard.
    void sendFaderConfigRequest();

    // CMD 1Fh: Read back the current aftertouch look up table of the keyboard.
    void sendAftertouchConfigRequest();

    void sendVelocityIntervalConfigRequest();

    // CMD 22h: Read back the fader type of all keys on the targeted board.
	void getFaderTypeConfig(int boardIndex);

    // This command is used to read back the serial identification number of the keyboard.
    void sendGetSerialIdentityRequest();

    void startCalibrateKeys();

    void setLumatouchConfig(const juce::uint8 lumatouchTable[]);

    void resetLumatouchConfig();

    void getLumatouchConfig();

    // This command is used to read back the current Lumatone firmware revision.
    void sendGetFirmwareRevisionRequest();

    // Send a value from 0-127 for the Lumatone to echo back, returns actual value sent (in case of 7-bit masking); used for auto device connection and monitoring
    int pingLumatone(juce::uint8 pingId);

    // Set MIDI Channels of peripheral controllers, pitch & mod wheels, expression & sustain pedals
    void setPeripheralChannels(int pitchWheelChannel, int modWheelChannel, int expressionChannel, int sustainChannel);
    void setPeripheralChannels(PeripheralChannelSettings channelSettings);

    // Get MIDI Channels of peripheral controllers, pitch & mod wheels, expression & sustain pedals
    void getPeripheralChannels();

    // Invert the polarity of the sustain pedal
    void invertSustainPedal(bool setInverted);

    // Reset preset mappings to factory mappings
    void resetPresetsToFactoryDefault();
    void resetPresetToFactoryDefault(int presetIndex);

    // Get interaction flags of current preset
    void requestPresetFlags();

    // Get sensitivity setting of expression pedal
    void requestExpressionPedalSensitivity();

public:

    bool loadLayoutFromFile(const juce::File& file) override;

public:

    void addMidiListener(LumatoneMidiState::Listener* listener);
    void removeMidiListener(LumatoneMidiState::Listener* listener);

protected:
    //============================================================================
    // LumatoneEditor::FirmwareListener implementation

    void serialIdentityReceived(const int* serialBytes) override;

    void firmwareRevisionReceived(FirmwareVersion version) override;

    void pingResponseReceived(unsigned int pingValue) override;

    void octaveColourConfigReceived(int boardId, juce::uint8 rgbFlag, const int* colourData) override;
    void octaveChannelConfigReceived(int octaveIndex, const int* channelData) override;
    void octaveNoteConfigReceived(int octaveIndex, const int* noteData) override;
    void keyTypeConfigReceived(int boardId, const int* keyTypeData) override;

    //============================================================================
    // Test functions

    //void loadRandomMapping(int testTimeoutMs, int maxIterations, int i = 0);

protected:
    //============================================================================
    // Implementation of LumatoneFirmwareDriver::Listener

    virtual void midiMessageReceived(juce::MidiInput* source, const juce::MidiMessage& midiMessage) override;

    virtual void midiMessageSent(juce::MidiOutput* target, const juce::MidiMessage& midiMessage) override {};
    virtual void midiSendQueueSize(int queueSize) override {};
    //virtual void generalLogMessage(juce::String textMessage, HajuErrorVisualizer::ErrorLevel errorLevel) override;
    virtual void noAnswerToMessage(juce::MidiDeviceInfo expectedDevice, const juce::MidiMessage& midiMessage) override;
    
private:

    LumatoneFirmwareDriver& firmwareDriver;
    LumatoneKeyUpdateBuffer updateBuffer;

    std::unique_ptr<LumatoneEventManager>   eventManager;

    const int bufferReadTimeoutMs   = 30;
    const int bufferReadSize        = 16;
    bool      bufferReadRequested   = false;

    std::atomic<int>    readQueueSize;
    int                 sendQueueSize = 0;

    int     lastTestDeviceSent          = -1;
    int     lastTestDeviceResponded     = -1;
    bool    waitingForTestResponse      = false;
    bool    currentDevicePairConfirmed  = false;
};
