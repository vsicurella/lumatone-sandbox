/*
  ==============================================================================

    LumatoneCommandManager.cpp
    Created: 17 Apr 2021 10:17:31pm
    Author:  Vincenzo

  ==============================================================================
*/

#include "lumatone_controller.h"

#include "lumatone_event_manager.h"
#include "../lumatone_midi_driver/lumatone_midi_driver.h"
#include "../listeners/editor_listener.h"

LumatoneController::LumatoneController(const LumatoneApplicationState& stateIn, LumatoneFirmwareDriver& driverIn)
    : LumatoneApplicationState("LumatoneController", stateIn)
    , LumatoneApplicationState::DeviceController(static_cast<LumatoneApplicationState&>(*this))
    , LumatoneApplicationMidiController(stateIn, driverIn)
    , firmwareDriver(driverIn)
    , updateBuffer(driverIn, stateIn)
    // , LumatoneSandboxLogger("LumatoneController")
{
    firmwareDriver.addDriverListener(this);

    eventManager = std::make_unique<LumatoneEventManager>(firmwareDriver, stateIn);
    eventManager->addFirmwareListener(this);

    addStatusListener(this);
}

LumatoneController::~LumatoneController()
{
    firmwareDriver.removeDriverListener(this);
}

juce::ValueTree LumatoneController::loadStateProperties(juce::ValueTree stateIn)
{
    LumatoneApplicationState::loadStateProperties(stateIn);
    return state;
}

void LumatoneController::connectionStateChanged(ConnectionState newState)
{
    switch (newState)
    {
    case ConnectionState::DISCONNECTED:
        currentDevicePairConfirmed = false;
        break;

    case ConnectionState::ONLINE:
        onConnectionConfirmed();
        return;

    default:
        break;
    }

    //statusListeners.call(&LumatoneEditor::StatusListener::connectionStateChanged, newState);
}


// int LumatoneController::getMidiInputIndex() const
// {
//     return firmwareDriver.getMidiInputIndex();
// }

// int LumatoneController::getMidiOutputIndex() const
// {
//     return firmwareDriver.getMidiOutputIndex();
// }

void LumatoneController::setDriverMidiInput(int deviceIndex, bool test)
{
    const bool changed = firmwareDriver.getMidiInputIndex() != deviceIndex;
    firmwareDriver.setMidiInput(deviceIndex);

    if (changed)
        currentDevicePairConfirmed = false;

    if (test && deviceIndex >= 0)
        testCurrentDeviceConnection();
}

void LumatoneController::setDriverMidiOutput(int deviceIndex, bool test)
{
    const bool changed = firmwareDriver.getMidiOutputIndex() != deviceIndex;

    firmwareDriver.setMidiOutput(deviceIndex);

    if (changed)
        currentDevicePairConfirmed = false;

    if (test && deviceIndex >= 0)
        testCurrentDeviceConnection();
}

/*
==============================================================================
Combined (hi-level) commands
*/


void LumatoneController::sendAllParamsOfBoard(int boardId, const LumatoneBoard* boardData, bool signalEditorListeners, bool bufferKeyUpdates)
{
    for (int keyIndex = 0; keyIndex < getOctaveBoardSize(); keyIndex++)
    {
        LumatoneKey key = boardData->getKey(keyIndex);
        sendKeyParam(boardId, keyIndex, key, false, bufferKeyUpdates);
    }
}

void LumatoneController::sendCompleteMapping(const LumatoneLayout& mappingData, bool signalEditorListeners, bool bufferKeyUpdates)
{
    for (int boardId = 1; boardId <= getNumBoards(); boardId++)
        sendAllParamsOfBoard(boardId, &mappingData.getBoard(boardId - 1), false, bufferKeyUpdates);

    clearContext();
}

void LumatoneController::sendCurrentCompleteConfig(bool signalEditorListeners)
{
	// MIDI channel, MIDI note, colour and key type config for all keys
	sendCompleteMapping(*getMappingData(), true, false);

	// General options
	setAftertouchEnabled(getMappingData()->getAftertouchOn());
	sendLightOnKeyStrokes(getMappingData()->getLightOnKeyStrokes());
	sendInvertFootController(getMappingData()->getInvertExpression());
	sendExpressionPedalSensivity(getMappingData()->getExpressionSensitivity());
    invertSustainPedal(getMappingData()->getInvertSustain());

	// Velocity curve config
	setVelocityIntervalConfig(getMappingData()->getConfigTable(LumatoneConfigTable::velocityInterval)->velocityValues);
}

void LumatoneController::sendGetMappingOfBoardRequest(int boardId)
{
    getRedLEDConfig(boardId);
    getGreenLEDConfig(boardId);
    getBlueLEDConfig(boardId);
    getChannelConfig(boardId);
    getNoteConfig(boardId);
    getKeyTypeConfig(boardId);
    getFaderTypeConfig(boardId);
}

void LumatoneController::sendGetCompleteMappingRequest()
{
    for (int boardId = 1; boardId <= getNumBoards(); boardId++)
        sendGetMappingOfBoardRequest(boardId);
}

void LumatoneController::resetVelocityConfig(LumatoneConfigTable::TableType velocityCurveType)
{
    switch (velocityCurveType)
    {
    case LumatoneConfigTable::TableType::velocityInterval:
        resetVelocityConfig();
        break;
    case LumatoneConfigTable::TableType::fader:
        resetFaderConfig();
        break;
    case LumatoneConfigTable::TableType::afterTouch:
        resetAftertouchConfig();
        break;
    case LumatoneConfigTable::TableType::lumaTouch:
        resetLumatouchConfig();
        break;
    default:
        jassert(false);
        break;
    }
}

unsigned int LumatoneController::sendTestMessageToDevice(int deviceIndex, unsigned int pingId)
{
    if (!firmwareDriver.testIsIncomplete())
    {
        firmwareDriver.openAvailableDevicesForTesting();
    }

    int value = (pingId < 0)
        ? deviceIndex
        : pingId &= 0xFFFFFFF;

    if (getLumatoneVersion() >= LumatoneFirmware::ReleaseVersion::VERSION_1_0_9)
    {
        firmwareDriver.ping(value, deviceIndex);
    }
    else
    {
        firmwareDriver.sendGetSerialIdentityRequest(deviceIndex);
    }

    // lastTestDeviceSent = deviceIndex;
    checkingDeviceIsLumatone = true;

    return value;
}

void LumatoneController::testCurrentDeviceConnection()
{
    // On confirmed connection send connection listener message
    if (firmwareDriver.hasDevicesDefined())
    {
        checkingDeviceIsLumatone = true;

        if (getSerialNumber().isNotEmpty() && getLumatoneVersion() >= LumatoneFirmware::ReleaseVersion::VERSION_1_0_9)
        {
            pingLumatone(0xf);
        }

        else
        {
            sendGetSerialIdentityRequest();
        }
    }
    else
    {
        currentDevicePairConfirmed = false;
    }
}

// Send parametrization of one key to the device
void LumatoneController::sendKeyParam(int boardId, int keyIndex, LumatoneKey keyData, bool signalEditorListeners, bool bufferKeyUpdates)
{
    // Default CC polarity = 1, Inverted CC polarity = 0
    sendKeyConfig(boardId, keyIndex, keyData, false, bufferKeyUpdates);
    sendKeyColourConfig(boardId, keyIndex, keyData, false, bufferKeyUpdates);
}

// Send configuration of a certain look up table
void LumatoneController::sendTableConfig(LumatoneConfigTable::TableType velocityCurveType, const juce::uint8* table)
{
    switch (velocityCurveType)
    {
    case LumatoneConfigTable::TableType::fader:
        setFaderConfig(table);
        break;

    case LumatoneConfigTable::TableType::afterTouch:
        setAftertouchConfig(table);
        break;

    case LumatoneConfigTable::TableType::lumaTouch:
        setLumatouchConfig(table);
        break;

    default:
        sendVelocityConfig(table);
    }
}

void LumatoneController::sendTableConfig(LumatoneConfigTable::TableType velocityCurveType, const int* table)
{
    juce::uint8 formatted[128];
    for (int i = 0; i < 128; i++)
    {
        formatted[i] = (juce::uint8)table[i];
    }

    switch (velocityCurveType)
    {
    case LumatoneConfigTable::TableType::fader:
        setFaderConfig(formatted);
        break;

    case LumatoneConfigTable::TableType::afterTouch:
        setAftertouchConfig(formatted);
        break;

    case LumatoneConfigTable::TableType::lumaTouch:
        setLumatouchConfig(formatted);
        break;

    default:
        sendVelocityConfig(formatted);
    }
}

//=============================================================================
// Mid-level firmware functions

// Send note, channel, cc, and fader polarity data
void LumatoneController::sendKeyConfig(int boardId, int keyIndex, const LumatoneKey& keyData, bool signalEditorListeners, bool bufferKeyUpdates)
{
    if (bufferKeyUpdates)
        updateBuffer.sendKeyConfig(boardId, keyIndex, keyData);
    else
        firmwareDriver.sendKeyFunctionParameters(boardId, keyIndex, keyData.getMidiNumber(), keyData.getMidiChannel(), keyData.getType(), keyData.isCCFaderDefault());
}

void LumatoneController::sendKeyColourConfig(int boardId, int keyIndex, juce::Colour colour, bool signalEditorListeners, bool bufferKeyUpdates)
{
    if (bufferKeyUpdates)
        updateBuffer.sendKeyColourConfig(boardId, keyIndex, colour);
    else
    {
        if (getLumatoneVersion() >= LumatoneFirmware::ReleaseVersion::VERSION_1_0_11)
            firmwareDriver.sendKeyLightParameters(boardId, keyIndex, colour.getRed(), colour.getGreen(), colour.getBlue());
        else
            firmwareDriver.sendKeyLightParameters_Version_1_0_0(boardId, keyIndex, colour.getRed() / 2, colour.getGreen() / 2, colour.getBlue() / 2);
    }
}

void LumatoneController::sendKeyColourConfig(int boardId, int keyIndex, const LumatoneKey& keyColourConfig, bool signalEditorListeners, bool bufferKeyUpdates)
{
    sendKeyColourConfig(boardId, keyIndex, keyColourConfig.getColour(), signalEditorListeners, bufferKeyUpdates);
}


// Send expression pedal sensivity
void LumatoneController::sendExpressionPedalSensivity(unsigned char value)
{
    LumatoneState::setExpressionSensitivity(value);
    firmwareDriver.sendExpressionPedalSensivity(value);
}

// Send parametrization of foot controller
void LumatoneController::sendInvertFootController(bool value)
{
    LumatoneState::setInvertExpression(value);
    firmwareDriver.sendInvertFootController(value);
}

// Colour for macro button in active state
void LumatoneController::sendMacroButtonActiveColour(juce::String colourAsString)
{
    auto c = juce::Colour::fromString(colourAsString);
    if (getLumatoneVersion() >= LumatoneFirmware::ReleaseVersion::VERSION_1_0_11)
        firmwareDriver.sendMacroButtonActiveColour(c.getRed(), c.getGreen(), c.getBlue());
    else
        firmwareDriver.sendMacroButtonActiveColour_Version_1_0_0(c.getRed(), c.getGreen(), c.getBlue());
}

// Colour for macro button in inactive state
void LumatoneController::sendMacroButtonInactiveColour(juce::String colourAsString)
{
    auto c = juce::Colour::fromString(colourAsString);
    if (getLumatoneVersion() >= LumatoneFirmware::ReleaseVersion::VERSION_1_0_11)
        firmwareDriver.sendMacroButtonInactiveColour(c.getRed(), c.getGreen(), c.getBlue());
    else
        firmwareDriver.sendMacroButtonInactiveColour_Version_1_0_0(c.getRed(), c.getGreen(), c.getBlue());
}

// Send parametrization of light on keystrokes
void LumatoneController::sendLightOnKeyStrokes(bool value)
{
    firmwareDriver.sendLightOnKeyStrokes(value);
}

// Send a value for a velocity lookup table
void LumatoneController::sendVelocityConfig(const juce::uint8 velocityTable[])
{
    firmwareDriver.sendVelocityConfig(velocityTable);
}

// Save velocity config to EEPROM
void LumatoneController::saveVelocityConfig()
{
    firmwareDriver.saveVelocityConfig();
}

void LumatoneController::resetVelocityConfig()
{
    firmwareDriver.resetVelocityConfig();
}

void LumatoneController::setFaderConfig(const juce::uint8 faderTable[])
{
    firmwareDriver.sendFaderConfig(faderTable);
}

void LumatoneController::resetFaderConfig()
{
    firmwareDriver.resetFaderConfig();
}

void LumatoneController::setAftertouchEnabled(bool value)
{
    firmwareDriver.sendAfterTouchActivation(value);
}

void LumatoneController::startCalibrateAftertouch()
{
    firmwareDriver.sendCalibrateAfterTouch();
}

void LumatoneController::setAftertouchConfig(const juce::uint8 aftertouchTable[])
{
    firmwareDriver.sendAftertouchConfig(aftertouchTable);
}

void LumatoneController::resetAftertouchConfig()
{
    firmwareDriver.resetAftertouchConfig();
}

void LumatoneController::setVelocityIntervalConfig(const int velocityIntervalTable[])
{
    firmwareDriver.sendVelocityIntervalConfig(velocityIntervalTable);
}

void LumatoneController::getRedLEDConfig(int boardIndex)
{
    firmwareDriver.sendRedLEDConfigRequest(boardIndex);
}

void LumatoneController::getGreenLEDConfig(int boardIndex)
{
    firmwareDriver.sendGreenLEDConfigRequest(boardIndex);
}

void LumatoneController::getBlueLEDConfig(int boardIndex)
{
    firmwareDriver.sendBlueLEDConfigRequest(boardIndex);
}

void LumatoneController::getChannelConfig(int boardIndex)
{
    firmwareDriver.sendChannelConfigRequest(boardIndex);
}

void LumatoneController::getNoteConfig(int boardIndex)
{
    firmwareDriver.sendNoteConfigRequest(boardIndex);
}

void LumatoneController::getKeyTypeConfig(int boardIndex)
{
    firmwareDriver.sendKeyTypeConfigRequest(boardIndex);
}

void LumatoneController::sendVelocityConfigRequest()
{
    firmwareDriver.sendVelocityConfigRequest();
}

void LumatoneController::sendFaderConfigRequest()
{
    firmwareDriver.sendFaderConfigRequest();
}

void LumatoneController::sendAftertouchConfigRequest()
{
    firmwareDriver.sendAftertouchConfigRequest();
}

void LumatoneController::sendVelocityIntervalConfigRequest()
{
    firmwareDriver.sendVelocityIntervalConfigRequest();
}

void LumatoneController::getFaderTypeConfig(int boardIndex)
{
    firmwareDriver.sendFaderTypeConfigRequest(boardIndex);
}

// This command is used to read back the serial identification number of the keyboard.
void LumatoneController::sendGetSerialIdentityRequest()
{
    firmwareDriver.sendGetSerialIdentityRequest();
}

void LumatoneController::startCalibrateKeys()
{
    firmwareDriver.sendCalibrateKeys();
}

void LumatoneController::setCalibratePitchModWheel(bool startCalibration)
{
    firmwareDriver.sendCalibratePitchModWheel(startCalibration);
}

void LumatoneController::setLumatouchConfig(const juce::uint8 lumatouchTable[])
{
    firmwareDriver.setLumatouchConfig(lumatouchTable);
}

void LumatoneController::resetLumatouchConfig()
{
    firmwareDriver.resetLumatouchConfig();
}

void LumatoneController::getLumatouchConfig()
{
    firmwareDriver.sendLumatouchConfigRequest();
}

// This command is used to read back the current Lumatone firmware revision.
void LumatoneController::sendGetFirmwareRevisionRequest()
{
    firmwareDriver.sendGetFirmwareRevisionRequest();
}

// Send a value for the Lumatone to echo back; used for auto device connection and monitoring
int LumatoneController::pingLumatone(juce::uint8 pingId)
{
    return firmwareDriver.ping(pingId);
}

// Set MIDI Channels of peripheral controllers, pitch & mod wheels, expression & sustain pedals
void LumatoneController::setPeripheralChannels(int pitchWheelChannel, int modWheelChannel, int expressionChannel, int sustainChannel)
{
    if (firmwareSupport.versionAcknowledgesCommand(getLumatoneVersion(), SET_PERIPHERAL_CHANNELS))
        firmwareDriver.setPeripheralChannels(pitchWheelChannel, modWheelChannel, expressionChannel, sustainChannel);
}

void LumatoneController::setPeripheralChannels(LumatoneFirmware::PeripheralChannelSettings channelSettings)
{
    setPeripheralChannels(channelSettings.pitchWheel, channelSettings.modWheel, channelSettings.expressionPedal, channelSettings.sustainPedal);
}

// Get MIDI Channels of peripheral controllers, pitch & mod wheels, expression & sustain pedals
void LumatoneController::getPeripheralChannels()
{
    if (firmwareSupport.versionAcknowledgesCommand(getLumatoneVersion(), GET_PERIPHERAL_CHANNELS))
        firmwareDriver.getPeripheralChannels();
}

void LumatoneController::invertSustainPedal(bool setInverted)
{
    LumatoneState::setInvertSustain(setInverted);

    if (firmwareSupport.versionAcknowledgesCommand(getLumatoneVersion(), INVERT_SUSTAIN_PEDAL))
        firmwareDriver.sendInvertSustainPedal(setInverted);
}

void LumatoneController::resetPresetsToFactoryDefault()
{
    if (firmwareSupport.versionAcknowledgesCommand(getLumatoneVersion(), RESET_DEFAULT_PRESETS))
    {
        for (int i = 0; i < 10; i++)
            firmwareDriver.sendResetDefaultPresetsRequest(i);
    }
}

void LumatoneController::resetPresetToFactoryDefault(int presetIndex)
{
    firmwareDriver.sendResetDefaultPresetsRequest(presetIndex);
}

// Get interaction flags of current preset
void LumatoneController::requestPresetFlags()
{
    if (firmwareSupport.versionAcknowledgesCommand(getLumatoneVersion(), GET_PRESET_FLAGS))
        firmwareDriver.sendGetPresetFlagsReset();
}

// Get sensitivity setting of expression pedal
void LumatoneController::requestExpressionPedalSensitivity()
{
    if (firmwareSupport.versionAcknowledgesCommand(getLumatoneVersion(), GET_EXPRESSION_PEDAL_SENSITIVIY))
        firmwareDriver.sendGetExpressionPedalSensitivity();
}

void LumatoneController::requestMacroButtonColours()
{
    if (firmwareSupport.versionAcknowledgesCommand(getLumatoneVersion(), GET_MACRO_LIGHT_INTENSITY))
        firmwareDriver.sendGetMacroLightIntensity();
}

bool LumatoneController::connectionConfirmed() const
{
    return firmwareDriver.hasDevicesDefined() && currentDevicePairConfirmed;
}

void LumatoneController::onConnectionConfirmed()
{
    checkingDeviceIsLumatone = false;
    currentDevicePairConfirmed = true;
    
    if (getSerialNumber().isEmpty())
    {
        waitingForFirmwareVersion = true;
        sendGetSerialIdentityRequest();
    }
    else if (getSerialNumber() != SERIAL_55_KEYS)
    {
        waitingForFirmwareVersion = true;
        sendGetFirmwareRevisionRequest();
    }
    else
    {
        waitingForFirmwareVersion = false;
    }
}

void LumatoneController::handleStatePropertyChange(juce::ValueTree stateIn, const juce::Identifier &property)
{
    LumatoneApplicationState::handleStatePropertyChange(stateIn, property);

    if (waitingForFirmwareVersion && property == LumatoneStateProperty::LastConnectedFirmwareVersion)
    {
        waitingForFirmwareVersion = false;
    }
}

// LumatoneEditor::FirmwareListener Implementation

void LumatoneController::serialIdentityReceived(const int* serialBytes)
{
    juce::String serialNumber = firmwareSupport.serialIdentityToString(serialBytes);
    DBG("Device serial is: " + serialNumber);

    setConnectedSerialNumber(serialNumber);

    if (checkingDeviceIsLumatone)
    {
        if (serialNumber != SERIAL_55_KEYS)
            sendGetFirmwareRevisionRequest();
        else
            onConnectionConfirmed();
            
        setConnectionState(ConnectionState::ONLINE);
    }

    if (waitingForFirmwareVersion)
    {
        sendGetFirmwareRevisionRequest();
    }
}

void LumatoneController::firmwareRevisionReceived(LumatoneFirmware::Version version)
{
    // setFirmwareVersion(version, true);
    waitingForFirmwareVersion = false;

    if (checkingDeviceIsLumatone)
    {
        onConnectionConfirmed();
        setConnectionState(ConnectionState::ONLINE);
    }
}

void LumatoneController::pingResponseReceived(unsigned int pingValue)
{
    if (checkingDeviceIsLumatone)
    {
        onConnectionConfirmed();
        setConnectionState(ConnectionState::ONLINE);
    }
}
