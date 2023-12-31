/*
  ==============================================================================

    LumatoneCommandManager.h
    Created: 17 Apr 2021 10:17:31pm
    Author:  Vincenzo

  ==============================================================================
*/

#pragma once

#include "./listeners/status_listener.h"
#include "./listeners/editor_listener.h"
#include "./listeners/firmware_listener.h"

#include "key_update_buffer.h"

#include "./data/application_state.h"
#include "./data/lumatone_midi_manager.h"

class LumatoneEventManager;
class LumatoneAction;

//==============================================================================
// Helper class for parsing and comparing (todo) firmware versions


class LumatoneController :  public LumatoneApplicationState
                         ,  public LumatoneApplicationMidiController
                         ,  public LumatoneEditor::StatusListener
                         ,  public LumatoneEditor::StatusEmitter
                         ,  public LumatoneEditor::EditorEmitter
                         ,  protected LumatoneEditor::FirmwareListener
{
public:

    LumatoneController(LumatoneApplicationState stateIn, LumatoneFirmwareDriver& firmwareDriverIn, juce::UndoManager* undoManager);
    ~LumatoneController() override;

    juce::ValueTree loadStateProperties(juce::ValueTree stateIn) override;

    void setContext(const LumatoneContext& contextIn) override;
    void clearContext() override;
    
    //============================================================================
    // Methods to configure firmware communication parameters

    const FirmwareSupport& getFirmwareSupport() const { return firmwareSupport; }

    juce::Array<juce::MidiDeviceInfo> getMidiInputList();
    juce::Array<juce::MidiDeviceInfo> getMidiOutputList();

    int getMidiInputIndex() const;
    int getMidiOutputIndex() const;

    void setMidiInput(int deviceIndex, bool test = true);
    void setMidiOutput(int deviceIndex, bool test = true);

public:
    bool performAction(LumatoneAction* action, bool undoable = true, bool newTransaction = true);

private:
    bool connectionConfirmed() const;
    void onConnectionConfirmed();

public:
    //============================================================================
    // Status Listener implementation

    void connectionStateChanged(ConnectionState newState) override;

public:
    //============================================================================
    // Combined (hi-level) commands

    // Send all parametrizations of one sub board
    void sendAllParamsOfBoard(int boardId, const LumatoneBoard* boardData, bool signalEditorListeners=true, bool bufferKeyUpdates=false);

    // Send and save a complete key mapping
    void sendCompleteMapping(const LumatoneLayout& mappingData, bool signalEditorListeners=true, bool bufferKeyUpdates=true);

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
    void sendGetSerialIdentityRequest(bool confirmConnectionAfterResponse);

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
    void setPeripheralChannels(LumatoneFirmware::PeripheralChannelSettings channelSettings);

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

private:
    // juce::ValueTree::Listener implementation

    void handleStatePropertyChange(juce::ValueTree stateIn, const juce::Identifier& property) override;

protected:
    //============================================================================
    // LumatoneEditor::FirmwareListener implementation

    void serialIdentityReceived(const int* serialBytes) override;

    void firmwareRevisionReceived(LumatoneFirmware::Version version) override;

    void pingResponseReceived(unsigned int pingValue) override;

    void octaveColourConfigReceived(int boardId, juce::uint8 rgbFlag, const int* colourData) override;
    void octaveChannelConfigReceived(int octaveIndex, const int* channelData) override;
    void octaveNoteConfigReceived(int octaveIndex, const int* noteData) override;
    void keyTypeConfigReceived(int boardId, const int* keyTypeData) override;

    //============================================================================
    // Test functions

    //void loadRandomMapping(int testTimeoutMs, int maxIterations, int i = 0);

private:

    LumatoneFirmwareDriver& firmwareDriver;
    LumatoneKeyUpdateBuffer updateBuffer;

    std::unique_ptr<LumatoneEventManager>   eventManager;

    bool    waitingForTestResponse      = false;
    bool    currentDevicePairConfirmed  = false;
    bool    waitingForFirmwareVersion   = false;
};
