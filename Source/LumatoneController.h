/*
  ==============================================================================

    LumatoneCommandManager.h
    Created: 17 Apr 2021 10:17:31pm
    Author:  Vincenzo

  ==============================================================================
*/

#pragma once

#include "SandboxState.h"

#include "./EditorEmitter.h"
#include "./lumatone/KeyboardDataStructure.h"
//#include "DeviceActivityMonitor.h"
//#include "FirmwareTransfer.h"

#include "./lumatone_midi_driver/lumatone_midi_driver.h"

//==============================================================================
// Helper class for parsing and comparing (todo) firmware versions


class LumatoneController :  public LumatoneSandboxState,
                            private TerpstraMidiDriver::Collector,
                            public LumatoneEditor::StatusListener,
                            public LumatoneEditor::StatusEmitter,
                            protected LumatoneEditor::EditorEmitter,
                            private juce::Timer
                            //private juce::ChangeListener
{

public:

    LumatoneController(juce::ValueTree state, TerpstraMidiDriver& midiDriver, juce::UndoManager* undoManager);
    ~LumatoneController();
    
    //============================================================================
    // Methods to configure firmware communication parameters

    sysExSendingMode getEditorMode() const { return editingMode; }
    void setSysExSendingMode(sysExSendingMode newMode);

    FirmwareVersion getFirmwareVersion() const { return firmwareVersion; }

    LumatoneFirmwareVersion getConfirmedVersion() const { return determinedVersion; }

    const FirmwareSupport& getFirmwareSupport() const { return firmwareSupport; }

    int getOctaveSize() const { return octaveSize; }
    
    int getNumBoards() const { return BOARD_OCT_5; } // TODO: Set variable based on serial/version

    void refreshAvailableMidiDevices();

    juce::Array<juce::MidiDeviceInfo> getMidiInputList() { return midiDriver.getMidiInputList(); }
    juce::Array<juce::MidiDeviceInfo> getMidiOutputList() { return midiDriver.getMidiOutputList(); }

    int getMidiInputIndex() const { return midiDriver.getMidiInputIndex(); }
    int getMidiOutputIndex() const { return midiDriver.getMidiOutputIndex(); }

    void setMidiInput(int deviceIndex, bool test = true);
    void setMidiOutput(int deviceIndex, bool test = true);

    bool isConnected() const { return midiDriver.hasDevicesDefined() && currentDevicePairConfirmed; }

    //============================================================================
    // Status Listener implementation

    void connectionFailed() override;

    void connectionEstablished(juce::String inputDeviceId, juce::String outputDeviceId) override;

    void connectionLost() override;

    //============================================================================
    // Combined (hi-level) commands

    // Send all parametrizations of one sub board
    void sendAllParamsOfBoard(int boardIndex, const LumatoneBoard* boardData, bool signalEditorListeners=false);

    // Send and save a complete key mapping
    void sendCompleteMapping(LumatoneLayout mappingData);

    // Send request to receive the current mapping of one sub board on the controller
    void sendGetMappingOfBoardRequest(int boardIndex);

    // Send request to receive the complete current mapping on the controller
    void sendGetCompleteMappingRequest();

    // Send parametrization of one key to the device
    void sendKeyParam(int boardIndex, int keyIndex, LumatoneKey keyData);

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
    void sendKeyConfig(int boardIndex, int keyIndex, const LumatoneKey& noteDataConfig, bool signalEditorListeners=true);

    // Send RGB colour data
    void sendKeyColourConfig(int boardIndex, int keyIndex, juce::Colour colour, bool signalEditorListeners = true);
    void sendKeyColourConfig(int boardIndex, int keyIndex, const LumatoneKey& keyColourConfig, bool signalEditorListeners = true);

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
    void getPresetFlags();

    // Get sensitivity setting of expression pedal
    void getExpressionPedalSensitivity();
    
    //============================================================================
    // juce::Timer implementation

    void timerCallback() override {};
    
    //============================================================================
    // LumatoneEditor::Status Listener implementation

    //void changeListenerCallback(juce::ChangeBroadcaster* source) override;


    //============================================================================
    // Test functions

    //void loadRandomMapping(int testTimeoutMs, int maxIterations, int i = 0);
    
private:
    
    // Takes a generic firmware version and parses it into a recognized firmware version
    void setFirmwareVersion(FirmwareVersion firmwareVersionIn);

    // Takes a recognized firmware version
    void setFirmwareVersion(LumatoneFirmwareVersion lumatoneVersion, bool parseVersion = true);

protected:
    //============================================================================
    // Implementation of TerpstraMidiDriver::Listener

    virtual void midiMessageReceived(juce::MidiInput* source, const juce::MidiMessage& midiMessage) override {};
    virtual void midiMessageSent(juce::MidiOutput* target, const juce::MidiMessage& midiMessage) override {};
    virtual void midiSendQueueSize(int queueSize) override {};
    //virtual void generalLogMessage(juce::String textMessage, HajuErrorVisualizer::ErrorLevel errorLevel) override;
    virtual void noAnswerToMessage(juce::MidiInput* expectedDevice, const juce::MidiMessage& midiMessage) override;
    
private:

    TerpstraMidiDriver&         midiDriver;


    juce::CriticalSection       criticalSection;

    FirmwareSupport             firmwareSupport;

    juce::String                connectedSerialNumber;
    LumatoneFirmwareVersion     determinedVersion = LumatoneFirmwareVersion::NO_VERSION;
    FirmwareVersion             firmwareVersion = { 0, 0, 0 };
    FirmwareVersion             incomingVersion = { 0, 0, 0 };
    int                         octaveSize = 56;

    HajuErrorVisualizerPlaceholder      errorVisualizer;

    const int                   bufferReadTimeoutMs = 30;
    const int                   bufferReadSize = 16;
    bool                        bufferReadRequested = false;

    std::atomic<int>            readQueueSize;
    int                         sendQueueSize = 0;

    int                         lastTestDeviceSent = -1;
    int                         lastTestDeviceResponded = -1;
    bool                        waitingForTestResponse = false;
    bool                        currentDevicePairConfirmed = false;
    
    sysExSendingMode editingMode = sysExSendingMode::offlineEditor;
};
