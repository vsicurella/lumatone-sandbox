/*
  ==============================================================================

    LumatoneCommandManager.cpp
    Created: 17 Apr 2021 10:17:31pm
    Author:  Vincenzo

  ==============================================================================
*/

#include "LumatoneController.h"


LumatoneController::LumatoneController(juce::ValueTree state, LumatoneFirmwareDriver& firmwareDriverIn, juce::UndoManager* undoManager)
    : LumatoneState(state, undoManager)
    , firmwareDriver(firmwareDriverIn)
    , updateBuffer(firmwareDriverIn, state)
    , readQueueSize(0)
{
    reset(bufferReadSize);
    firmwareDriver.addMessageCollector(this);
    
    eventManager = std::make_unique<LumatoneEventManager>(firmwareDriver, *this);
    eventManager->addFirmwareListener(this);
}

LumatoneController::~LumatoneController()
{
    firmwareDriver.removeMessageCollector(this);
}

void LumatoneController::connectionStateChanged(ConnectionState newState)
{
    statusListeners.call(&LumatoneEditor::StatusListener::connectionStateChanged, newState);
}

// void LumatoneController::connectionFailed()
// {
//     statusListeners.call(&LumatoneEditor::StatusListener::connectionFailed);
// }

// void LumatoneController::connectionEstablished(juce::String inputDeviceId, juce::String outputDeviceId)
// {
//     statusListeners.call(&LumatoneEditor::StatusListener::connectionEstablished, inputDeviceId, outputDeviceId);
// }

// void LumatoneController::connectionLost()
// {
//     statusListeners.call(&LumatoneEditor::StatusListener::connectionLost);
// }

void LumatoneController::setMidiInput(int deviceIndex, bool test)
{
    const bool changed = firmwareDriver.getMidiInputIndex() != deviceIndex;

    firmwareDriver.setMidiInput(deviceIndex);

    if (changed)
        currentDevicePairConfirmed = false;

    if (test && deviceIndex >= 0)
        testCurrentDeviceConnection();
}

void LumatoneController::setMidiOutput(int deviceIndex, bool test)
{
    const bool changed = firmwareDriver.getMidiOutputIndex() != deviceIndex;

    firmwareDriver.setMidiOutput(deviceIndex);

    if (changed)
        currentDevicePairConfirmed = false;

    if (test && deviceIndex >= 0)
        testCurrentDeviceConnection();
}

bool LumatoneController::performAction(LumatoneAction* action, bool undoable, bool newTransaction)
{
    if (action == nullptr)
        return false;

    if (undoable)
    {
        if (undoManager == nullptr)
            return false;

        if (newTransaction)
            undoManager->beginNewTransaction();

        return undoManager->perform((juce::UndoableAction*)action, action->getName());
    }

    return action->perform();
}

void LumatoneController::midiMessageReceived(juce::MidiInput* source, const juce::MidiMessage& midiMessage)
{
    if (midiMessage.isSysEx())
    {

    }
}

void LumatoneController::noAnswerToMessage(juce::MidiDeviceInfo expectedDevice, const juce::MidiMessage& midiMessage)
{
    if (midiMessage.isSysEx())
    {
        // if (currentDevicePairConfirmed)
        //     statusListeners.call(&LumatoneEditor::StatusListener::connectionLost);
        // else
        //     statusListeners.call(&LumatoneEditor::StatusListener::connectionFailed);
    }
}


/*
==============================================================================
Combined (hi-level) commands
*/


void LumatoneController::sendAllParamsOfBoard(int boardId, const LumatoneBoard* boardData, bool signalEditorListeners, bool bufferKeyUpdates)
{
    for (int keyIndex = 0; keyIndex < getOctaveBoardSize(); keyIndex++)
    {
        auto key = &boardData->theKeys[keyIndex];
        sendKeyParam(boardId, keyIndex, *key, false, bufferKeyUpdates);
    }

    if (signalEditorListeners)
        editorListeners.call(&LumatoneEditor::EditorListener::boardChanged, *boardData);
}

void LumatoneController::sendCompleteMapping(LumatoneLayout mappingData, bool signalEditorListeners, bool bufferKeyUpdates)
{
    for (int boardId = 1; boardId <= getNumBoards(); boardId++)
        sendAllParamsOfBoard(boardId, mappingData.getBoard(boardId - 1), false, bufferKeyUpdates);

    if (signalEditorListeners)
        editorListeners.call(&LumatoneEditor::EditorListener::completeMappingLoaded, mappingData);
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

    if (getLumatoneVersion() >= LumatoneFirmwareVersion::VERSION_1_0_9)
    {
        firmwareDriver.ping(value, deviceIndex);
    }
    else
    {
        firmwareDriver.sendGetSerialIdentityRequest(deviceIndex);
    }

    lastTestDeviceSent = deviceIndex;
    waitingForTestResponse = true;

    return value;
}

void LumatoneController::testCurrentDeviceConnection()
{
    // On confirmed connection send connection listener message
    if (firmwareDriver.hasDevicesDefined())
    {
        waitingForTestResponse = true;

        if (getSerialNumber().isNotEmpty() && getLumatoneVersion() >= LumatoneFirmwareVersion::VERSION_1_0_9)
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

    if (signalEditorListeners)
        editorListeners.call(&LumatoneEditor::EditorListener::keyChanged, boardId - 1, keyIndex, keyData);
}

void LumatoneController::sendSelectionParam(const juce::Array<MappedLumatoneKey>& selection, bool signalEditorListeners, bool bufferKeyUpdates)
{
    for (auto mappedKey : selection)
    {
        sendKeyConfig(mappedKey.boardIndex + 1, mappedKey.keyIndex, (LumatoneKey)mappedKey, false, bufferKeyUpdates);
    }

    if (signalEditorListeners)
        editorListeners.call(&LumatoneEditor::EditorListener::selectionChanged, selection);
}

void LumatoneController::sendSelectionColours(const juce::Array<MappedLumatoneKey>& selection, bool signalEditorListeners, bool bufferKeyUpdates)
{
    for (auto mappedKey : selection)
    {
        sendKeyColourConfig(mappedKey.boardIndex + 1, mappedKey.keyIndex, (LumatoneKey)mappedKey, false, bufferKeyUpdates);
    }

    if (signalEditorListeners)
        editorListeners.call(&LumatoneEditor::EditorListener::selectionChanged, selection);
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

//=============================================================================
// Mid-level firmware functions

// Send note, channel, cc, and fader polarity data
void LumatoneController::sendKeyConfig(int boardId, int keyIndex, const LumatoneKey& keyData, bool signalEditorListeners, bool bufferKeyUpdates)
{
    // firmwareDriver.sendKeyFunctionParameters(boardId, keyIndex, keyData.noteNumber, keyData.channelNumber, keyData.keyType, keyData.ccFaderDefault);

    if (bufferKeyUpdates)
        updateBuffer.sendKeyConfig(boardId, keyIndex, keyData);
    else
        firmwareDriver.sendKeyFunctionParameters(boardId, keyIndex, keyData.noteNumber, keyData.channelNumber, keyData.keyType, keyData.ccFaderDefault);

    *getEditKey(boardId - 1, keyIndex) = keyData;
    
    if (signalEditorListeners)
        editorListeners.call(&LumatoneEditor::EditorListener::keyConfigChanged, boardId - 1, keyIndex, keyData);
}

void LumatoneController::sendKeyColourConfig(int boardId, int keyIndex, juce::Colour colour, bool signalEditorListeners, bool bufferKeyUpdates)
{
    // if (getLumatoneVersion() >= LumatoneFirmwareVersion::VERSION_1_0_11)
    //     firmwareDriver.sendKeyLightParameters(boardId, keyIndex, colour.getRed(), colour.getGreen(), colour.getBlue());
    // else
    //     firmwareDriver.sendKeyLightParameters_Version_1_0_0(boardId, keyIndex, colour.getRed() / 2, colour.getGreen() / 2, colour.getBlue() / 2);

    if (bufferKeyUpdates)
        updateBuffer.sendKeyColourConfig(boardId, keyIndex, colour);
    else
    {
        if (getLumatoneVersion() >= LumatoneFirmwareVersion::VERSION_1_0_11)
            firmwareDriver.sendKeyLightParameters(boardId, keyIndex, colour.getRed(), colour.getGreen(), colour.getBlue());
        else
            firmwareDriver.sendKeyLightParameters_Version_1_0_0(boardId, keyIndex, colour.getRed() / 2, colour.getGreen() / 2, colour.getBlue() / 2);
    }

    getEditKey(boardId - 1, keyIndex)->colour = colour;

    if (signalEditorListeners)
        editorListeners.call(&LumatoneEditor::EditorListener::keyColourChanged, boardId - 1, keyIndex, colour);
}

void LumatoneController::sendKeyColourConfig(int boardId, int keyIndex, const LumatoneKey& keyColourConfig, bool signalEditorListeners, bool bufferKeyUpdates)
{
    sendKeyColourConfig(boardId, keyIndex, keyColourConfig.colour, signalEditorListeners, bufferKeyUpdates);
}


// Send expression pedal sensivity
void LumatoneController::sendExpressionPedalSensivity(unsigned char value)
{
    setExpressionSensitivity(value);
    firmwareDriver.sendExpressionPedalSensivity(value); 
    editorListeners.call(&LumatoneEditor::EditorListener::expressionPedalSensitivityChanged, value);
}

// Send parametrization of foot controller
void LumatoneController::sendInvertFootController(bool value)
{
    setInvertExpression(value);
    firmwareDriver.sendInvertFootController(value);
    editorListeners.call(&LumatoneEditor::EditorListener::invertFootControllerChanged, value);
}

// Colour for macro button in active state
void LumatoneController::sendMacroButtonActiveColour(juce::String colourAsString)
{
    auto c = juce::Colour::fromString(colourAsString);
    if (getLumatoneVersion() >= LumatoneFirmwareVersion::VERSION_1_0_11)
        firmwareDriver.sendMacroButtonActiveColour(c.getRed(), c.getGreen(), c.getBlue());
    else
        firmwareDriver.sendMacroButtonActiveColour_Version_1_0_0(c.getRed(), c.getGreen(), c.getBlue());
    editorListeners.call(&LumatoneEditor::EditorListener::macroButtonActiveColourChagned, c);
}

// Colour for macro button in inactive state
void LumatoneController::sendMacroButtonInactiveColour(juce::String colourAsString)
{
    auto c = juce::Colour::fromString(colourAsString);
    if (getLumatoneVersion() >= LumatoneFirmwareVersion::VERSION_1_0_11)
        firmwareDriver.sendMacroButtonInactiveColour(c.getRed(), c.getGreen(), c.getBlue());
    else
        firmwareDriver.sendMacroButtonInactiveColour_Version_1_0_0(c.getRed(), c.getGreen(), c.getBlue());
    editorListeners.call(&LumatoneEditor::EditorListener::macroButtonInactiveColourChanged, c);
}

// Send parametrization of light on keystrokes
void LumatoneController::sendLightOnKeyStrokes(bool value)
{
    firmwareDriver.sendLightOnKeyStrokes(value);
    editorListeners.call(&LumatoneEditor::EditorListener::lightOnKeyStrokesChanged, value);
}

// Send a value for a velocity lookup table
void LumatoneController::sendVelocityConfig(const juce::uint8 velocityTable[])
{
    firmwareDriver.sendVelocityConfig(velocityTable);
    editorListeners.call(&LumatoneEditor::EditorListener::tableChanged, LumatoneConfigTable::TableType::velocityInterval, velocityTable, VELOCITYINTERVALTABLESIZE);
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
    editorListeners.call(&LumatoneEditor::EditorListener::tableChanged, LumatoneConfigTable::TableType::fader, faderTable, VELOCITYINTERVALTABLESIZE);
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
    editorListeners.call(&LumatoneEditor::EditorListener::tableChanged, LumatoneConfigTable::TableType::afterTouch, aftertouchTable, VELOCITYINTERVALTABLESIZE);
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
    editorListeners.call(&LumatoneEditor::EditorListener::tableChanged, LumatoneConfigTable::TableType::lumaTouch, lumatouchTable, VELOCITYINTERVALTABLESIZE);
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

void LumatoneController::setPeripheralChannels(PeripheralChannelSettings channelSettings)
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
    setInvertSustain(setInverted);

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

void LumatoneController::onConnectionConfirmed()
{
    waitingForTestResponse = false;
    currentDevicePairConfirmed = true;

    if (getLumatoneVersion() != LumatoneFirmwareVersion::VERSION_55_KEYS)
        sendGetFirmwareRevisionRequest();

    statusListeners.call(&LumatoneEditor::StatusListener::connectionStateChanged, ConnectionState::ONLINE);
    sendGetCompleteMappingRequest();
}

bool LumatoneController::loadLayoutFromFile(const juce::File& file)
{
    const bool loaded = LumatoneState::loadLayoutFromFile(file);
    if (loaded)
    {
        sendCompleteMapping(*mappingData);
    }

    return loaded;
}

void LumatoneController::addMidiListener(LumatoneMidiState::Listener* listener)
{
    eventManager->addListener(listener);
}

void LumatoneController::removeMidiListener(LumatoneMidiState::Listener* listener)
{
    eventManager->removeListener(listener);
}


// LumatoneEditor::FirmwareListener Implementation

void LumatoneController::serialIdentityReceived(const int* serialBytes)
{
    juce::String serialNumber = firmwareSupport.serialIdentityToString(serialBytes);
    DBG("Device serial is: " + serialNumber);

    setConnectedSerialNumber(serialNumber);

    if (waitingForTestResponse)
        onConnectionConfirmed();
}

void LumatoneController::firmwareRevisionReceived(FirmwareVersion version)
{
    setFirmwareVersion(version, true);
}

void LumatoneController::pingResponseReceived(unsigned int pingValue)
{
    if (waitingForTestResponse)
        onConnectionConfirmed();
}

void LumatoneController::octaveColourConfigReceived(int boardId, juce::uint8 rgbFlag, const int* colourData) 
{
    auto octaveSize = getOctaveBoardSize();
    auto numBoards = getNumBoards();

    int boardIndex = boardId - 1;

    for (int keyIndex = 0; keyIndex < octaveSize; keyIndex++)
    {
        LumatoneKey* keyData = getEditKey(boardIndex, keyIndex);
        auto newValue = colourData[keyIndex];

        if (rgbFlag == 0)
        {
            keyData->colour = juce::Colour(newValue, keyData->colour.getGreen(), keyData->colour.getBlue());
        }
        else if (rgbFlag == 1)
        {
            keyData->colour = juce::Colour(keyData->colour.getRed(), newValue, keyData->colour.getBlue());
        }
        else if (rgbFlag == 2)
        {
            keyData->colour = juce::Colour(keyData->colour.getRed(), keyData->colour.getGreen(), newValue);
        }
        else
        {
            jassertfalse;
        }
    }

    editorListeners.call(&LumatoneEditor::EditorListener::boardChanged, *getBoard(boardIndex));
};


void LumatoneController::octaveChannelConfigReceived(int boardId, const int* channelData)
{
    for (int keyIndex = 0; keyIndex < getOctaveBoardSize(); keyIndex++)
    {
        // Check channel values?
        getEditKey(boardId-1, keyIndex)->channelNumber = channelData[keyIndex];
    }

    editorListeners.call(&LumatoneEditor::EditorListener::boardChanged, *getBoard(boardId - 1));
}

void LumatoneController::octaveNoteConfigReceived(int boardId, const int* noteData)
{
    for (int keyIndex = 0; keyIndex < getOctaveBoardSize(); keyIndex++)
    {
        // Check note values?
        getEditKey(boardId - 1, keyIndex)->noteNumber = noteData[keyIndex];
    }

    editorListeners.call(&LumatoneEditor::EditorListener::boardChanged, *getBoard(boardId - 1));
}

void LumatoneController::keyTypeConfigReceived(int boardId, const int* keyTypeData)
{
    for (int keyIndex = 0; keyIndex < getOctaveBoardSize(); keyIndex++)
    {
        // Check note values?
        getEditKey(boardId - 1, keyIndex)->keyType = LumatoneKeyType(keyTypeData[keyIndex]);
    }

    editorListeners.call(&LumatoneEditor::EditorListener::boardChanged, *getBoard(boardId - 1));
}

//FirmwareSupport::Error LumatoneController::handlePingResponse(const juce::MidiMessage& midiMessage)
//{
//    unsigned int value = 0;
//    auto errorCode = firmwareDriver.unpackPingResponse(midiMessage, value);
//    
//    if (errorCode != FirmwareSupport::Error::noError)
//        return errorCode;
//
//    firmwareListeners.call(&LumatoneEditor::FirmwareListener::pingResponseReceived, lastTestDeviceResponded, value);
//
//    if (firmwareDriver.hasDevicesDefined() && !currentDevicePairConfirmed)
//    {
//        onConnectionConfirm(true);
//    }
//    
//    return errorCode;
//}

//
//void LumatoneController::changeListenerCallback(juce::ChangeBroadcaster* source)
//{
//    if (source == deviceMonitor.get())
//    {
//        int newInput = deviceMonitor->getConfirmedInputIndex();
//        int newOutput = deviceMonitor->getConfirmedOutputIndex();
//
//        if (newInput >= 0 && newOutput >= 0)
//        {
//            currentDevicePairConfirmed = false;
//            firmwareDriver.setMidiInput(newInput);
//            firmwareDriver.setMidiOutput(newOutput);
//
//            // if (editingMode == sysExSendingMode::firmwareUpdate)
//            // {
//                confirmAutoConnection();
//                return;
//            // }
//
//            // if (connectedSerialNumber.isEmpty())
//            //     sendGetSerialIdentityRequest();
//
//            // return;
//        }
//        
//        if (currentDevicePairConfirmed)
// 
//        {
//            // This should not get triggered if we are already disconnected
//            jassert(firmwareDriver.hasDevicesDefined());
//            onDisconnection();
//        }
//        else
//        {
//            // Something went wrong, but just try to continue connecting
//            DBG("Connection was tripped");
//            // Kludge - device monitor should be able to do this on it's own
//            if (deviceMonitor->willDetectDeviceIfDisconnected())
//                deviceMonitor->initializeDeviceDetection();
//        }
//    }
//}

//void LumatoneController::loadRandomMapping(int testTimeoutMs,  int maxIterations, int i)
//{
//    auto dir = juce::File::getSpecialLocation(juce::File::SpecialLocationType::userDocumentsDirectory).getChildFile("Lumatone Editor").getChildFile("Mappings");
//    auto mappings = dir.findChildFiles(juce::File::TypesOfFileToFind::findFiles, true);
//    auto numfiles = mappings.size();
//    auto r = juce::Random();
//        
//    auto fileIndex = r.nextInt(numfiles-1);
//    auto file = mappings[fileIndex];
//
//    if (file.exists() && file.hasFileExtension(".ltn"))
//    {
//        DBG("Found " + juce::String(numfiles) + " files, loading " + file.getFileName());
//        juce::MessageManager::callAsync([file]() { TerpstraSysExApplication::getApp().setCurrentFile(file); });
//    }
//    
////    if (i < maxIterations)
////        Timer::callAfterDelay(testTimeoutMs, [&]() { loadRandomMapping(testTimeoutMs, maxIterations, i + 1); });
////    else
////        DBG("Finished random mappings test.");
//}
