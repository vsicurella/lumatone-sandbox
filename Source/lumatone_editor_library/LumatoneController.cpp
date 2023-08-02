/*
  ==============================================================================

    LumatoneCommandManager.cpp
    Created: 17 Apr 2021 10:17:31pm
    Author:  Vincenzo

  ==============================================================================
*/

#include "LumatoneController.h"


LumatoneController::LumatoneController(juce::ValueTree state, TerpstraMidiDriver& midiDriverIn, juce::UndoManager* undoManager)
    : LumatoneState(state, undoManager),
        midiDriver(midiDriverIn),
        //errorVisualizer(TerpstraSysExApplication::getApp().getLookAndFeel()),
        readQueueSize(0)
{
    reset(bufferReadSize);
    midiDriver.addMessageCollector(this);
    
    eventManager = std::make_unique<LumatoneEventManager>(midiDriver, *this);
    eventManager->addFirmwareListener(this);
}

LumatoneController::~LumatoneController()
{
    midiDriver.removeMessageCollector(this);
}

void LumatoneController::connectionFailed()
{
    statusListeners.call(&LumatoneEditor::StatusListener::connectionFailed);
}

void LumatoneController::connectionEstablished(juce::String inputDeviceId, juce::String outputDeviceId)
{
    statusListeners.call(&LumatoneEditor::StatusListener::connectionEstablished, inputDeviceId, outputDeviceId);
}

void LumatoneController::connectionLost()
{
    statusListeners.call(&LumatoneEditor::StatusListener::connectionLost);
}

//
//void LumatoneController::setSysExSendingMode(sysExSendingMode newMode)
//{
//    // Might be worth it to provide further context of commands, such as which ones are mutable or not
//    // And use an automated system for determining whether or not to send a message in live or offline mode
//
//    if (newMode != editingMode)
//    {
//        editingMode = newMode;
//        if (editingMode == sysExSendingMode::offlineEditor)
//        {
//            midiDriver.clearMIDIMessageBuffer();	// ToDo remove only SysEx messages (leave NoteOn/NoteOff)?
//            stopTimer();
//        }
//    }
//    
//    editorListeners.call(&LumatoneEditor::EditorListener::editorModeChanged, newMode);
//    midiDriver.restrictToRequestMessages(editingMode == sysExSendingMode::offlineEditor);
//}

void LumatoneController::setMidiInput(int deviceIndex, bool test)
{
    const bool changed = midiDriver.getMidiInputIndex() != deviceIndex;

    midiDriver.setMidiInput(deviceIndex);

    if (changed)
        currentDevicePairConfirmed = false;

    if (test && deviceIndex >= 0)
        testCurrentDeviceConnection();
}

void LumatoneController::setMidiOutput(int deviceIndex, bool test)
{
    const bool changed = midiDriver.getMidiOutputIndex() != deviceIndex;

    midiDriver.setMidiOutput(deviceIndex);

    if (changed)
        currentDevicePairConfirmed = false;

    if (test && deviceIndex >= 0)
        testCurrentDeviceConnection();
}

bool LumatoneController::performUndoableAction(juce::UndoableAction* undoableAction, bool newTransaction, juce::String actionName)
{
    if (undoManager == nullptr || undoableAction == nullptr)
        return false;

    if (newTransaction)
        undoManager->beginNewTransaction();

    undoManager->perform(undoableAction, actionName);

    return true;
}

void LumatoneController::refreshAvailableMidiDevices() 
{ 
    if (midiDriver.refreshDeviceLists() && midiDriver.testIsIncomplete())
        midiDriver.openAvailableDevicesForTesting();
}

void LumatoneController::midiMessageReceived(juce::MidiInput* source, const juce::MidiMessage& midiMessage)
{
    if (midiMessage.isSysEx())
    {

    }
}

void LumatoneController::noAnswerToMessage(juce::MidiInput* expectedDevice, const juce::MidiMessage& midiMessage)
{
    if (midiMessage.isSysEx())
    {
        if (currentDevicePairConfirmed)
            statusListeners.call(&LumatoneEditor::StatusListener::connectionLost);
        else
            statusListeners.call(&LumatoneEditor::StatusListener::connectionFailed);
    }
}



/*
==============================================================================
Combined (hi-level) commands
*/


void LumatoneController::sendAllParamsOfBoard(int boardIndex, const LumatoneBoard* boardData, bool signalEditorListeners)
{
    *getEditBoard(boardIndex) = *boardData;

    if (getLumatoneVersion() >= LumatoneFirmwareVersion::VERSION_1_0_11)
    {
        for (int keyIndex = 0; keyIndex < getOctaveBoardSize(); keyIndex++)
        {
            auto key = &boardData->theKeys[keyIndex];
            midiDriver.sendKeyFunctionParameters(boardIndex, keyIndex, key->noteNumber, key->channelNumber, key->keyType & 0x3);
            midiDriver.sendKeyLightParameters(boardIndex, keyIndex, key->colour.getRed(), key->colour.getGreen(), key->colour.getBlue());
        }
    }
    else
    {
        for (int keyIndex = 0; keyIndex < getOctaveBoardSize(); keyIndex++)
        {
            auto key = &boardData->theKeys[keyIndex];
            midiDriver.sendKeyFunctionParameters(boardIndex, keyIndex, key->noteNumber, key->channelNumber, key->keyType & 0x3);
            midiDriver.sendKeyLightParameters_Version_1_0_0(boardIndex, keyIndex, key->colour.getRed() / 2, key->colour.getGreen() / 2, key->colour.getBlue() / 2);
        }

        if (signalEditorListeners)
            editorListeners.call(&LumatoneEditor::EditorListener::boardChanged, *boardData);
    }
}

void LumatoneController::sendCompleteMapping(LumatoneLayout mappingData)
{
    for (int boardIndex = 1; boardIndex <= NUMBEROFBOARDS; boardIndex++)
        sendAllParamsOfBoard(boardIndex, mappingData.getBoard(boardIndex - 1));

    editorListeners.call(&LumatoneEditor::EditorListener::completeMappingLoaded, mappingData);
}

void LumatoneController::sendGetMappingOfBoardRequest(int boardIndex)
{
    getRedLEDConfig(boardIndex);
    getGreenLEDConfig(boardIndex);
    getBlueLEDConfig(boardIndex);
    getChannelConfig(boardIndex);
    getNoteConfig(boardIndex);
    getKeyTypeConfig(boardIndex);
    getFaderTypeConfig(boardIndex);
}

void LumatoneController::sendGetCompleteMappingRequest()
{
    for (int boardIndex = 1; boardIndex <= NUMBEROFBOARDS; boardIndex++)
        sendGetMappingOfBoardRequest(boardIndex);
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
    if (!midiDriver.testIsIncomplete())
    {
        midiDriver.openAvailableDevicesForTesting();
    }

    int value = (pingId < 0)
        ? deviceIndex
        : pingId &= 0xFFFFFFF;

    if (getLumatoneVersion() >= LumatoneFirmwareVersion::VERSION_1_0_9)
    {
        midiDriver.ping(value, deviceIndex);
    }
    else
    {
        midiDriver.sendGetSerialIdentityRequest(deviceIndex);
    }

    lastTestDeviceSent = deviceIndex;
    waitingForTestResponse = true;

    return value;
}

void LumatoneController::testCurrentDeviceConnection()
{
    // On confirmed connection send connection listener message
    if (midiDriver.hasDevicesDefined())
    {
        waitingForTestResponse = true;

        if (getLumatoneVersion() >= LumatoneFirmwareVersion::VERSION_1_0_9)
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
void LumatoneController::sendKeyParam(int boardId, int keyIndex, LumatoneKey keyData)
{    
    // Default CC polarity = 1, Inverted CC polarity = 0
    sendKeyConfig(boardId, keyIndex, keyData);
    sendKeyColourConfig(boardId, keyIndex, keyData);

    editorListeners.call(&LumatoneEditor::EditorListener::keyChanged, boardId - 1, keyIndex, keyData);
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
void LumatoneController::sendKeyConfig(int boardId, int keyIndex, const LumatoneKey& keyData, bool signalEditorListeners)
{
    *getEditKey(boardId - 1, keyIndex) = keyData;
    midiDriver.sendKeyFunctionParameters(boardId, keyIndex, keyData.noteNumber, keyData.channelNumber, keyData.keyType, keyData.ccFaderDefault);

    if (signalEditorListeners)
        editorListeners.call(&LumatoneEditor::EditorListener::keyConfigChanged, boardId - 1, keyIndex, keyData);
}

void LumatoneController::sendKeyColourConfig(int boardId, int keyIndex, juce::Colour colour, bool signalEditorListeners)
{
    getEditKey(boardId - 1, keyIndex)->colour = colour;

    if (getLumatoneVersion() >= LumatoneFirmwareVersion::VERSION_1_0_11)
        midiDriver.sendKeyLightParameters(boardId, keyIndex, colour.getRed(), colour.getGreen(), colour.getBlue());
    else
        midiDriver.sendKeyLightParameters_Version_1_0_0(boardId, keyIndex, colour.getRed() / 2, colour.getGreen() / 2, colour.getBlue() / 2);

    if (signalEditorListeners)
        editorListeners.call(&LumatoneEditor::EditorListener::keyColourChanged, boardId - 1, keyIndex, colour);
}

void LumatoneController::sendKeyColourConfig(int boardId, int keyIndex, const LumatoneKey& keyColourConfig, bool signalEditorListeners)
{
    sendKeyColourConfig(boardId, keyIndex, keyColourConfig.colour, signalEditorListeners);
}


// Send expression pedal sensivity
void LumatoneController::sendExpressionPedalSensivity(unsigned char value)
{
    setExpressionSensitivity(value);
    midiDriver.sendExpressionPedalSensivity(value); 
    editorListeners.call(&LumatoneEditor::EditorListener::expressionPedalSensitivityChanged, value);
}

// Send parametrization of foot controller
void LumatoneController::sendInvertFootController(bool value)
{
    setInvertExpression(value);
    midiDriver.sendInvertFootController(value);
    editorListeners.call(&LumatoneEditor::EditorListener::invertFootControllerChanged, value);
}

// Colour for macro button in active state
void LumatoneController::sendMacroButtonActiveColour(juce::String colourAsString)
{
    auto c = juce::Colour::fromString(colourAsString);
    if (getLumatoneVersion() >= LumatoneFirmwareVersion::VERSION_1_0_11)
        midiDriver.sendMacroButtonActiveColour(c.getRed(), c.getGreen(), c.getBlue());
    else
        midiDriver.sendMacroButtonActiveColour_Version_1_0_0(c.getRed(), c.getGreen(), c.getBlue());
    editorListeners.call(&LumatoneEditor::EditorListener::macroButtonActiveColourChagned, c);
}

// Colour for macro button in inactive state
void LumatoneController::sendMacroButtonInactiveColour(juce::String colourAsString)
{
    auto c = juce::Colour::fromString(colourAsString);
    if (getLumatoneVersion() >= LumatoneFirmwareVersion::VERSION_1_0_11)
        midiDriver.sendMacroButtonInactiveColour(c.getRed(), c.getGreen(), c.getBlue());
    else
        midiDriver.sendMacroButtonInactiveColour_Version_1_0_0(c.getRed(), c.getGreen(), c.getBlue());
    editorListeners.call(&LumatoneEditor::EditorListener::macroButtonInactiveColourChanged, c);
}

// Send parametrization of light on keystrokes
void LumatoneController::sendLightOnKeyStrokes(bool value)
{
    midiDriver.sendLightOnKeyStrokes(value);
    editorListeners.call(&LumatoneEditor::EditorListener::lightOnKeyStrokesChanged, value);
}

// Send a value for a velocity lookup table
void LumatoneController::sendVelocityConfig(const juce::uint8 velocityTable[])
{
    midiDriver.sendVelocityConfig(velocityTable);
    editorListeners.call(&LumatoneEditor::EditorListener::tableChanged, LumatoneConfigTable::TableType::velocityInterval, velocityTable, VELOCITYINTERVALTABLESIZE);
}

// Save velocity config to EEPROM
void LumatoneController::saveVelocityConfig()
{
    midiDriver.saveVelocityConfig();
}

void LumatoneController::resetVelocityConfig()
{
    midiDriver.resetVelocityConfig();
}

void LumatoneController::setFaderConfig(const juce::uint8 faderTable[])
{
    midiDriver.sendFaderConfig(faderTable);
    editorListeners.call(&LumatoneEditor::EditorListener::tableChanged, LumatoneConfigTable::TableType::fader, faderTable, VELOCITYINTERVALTABLESIZE);
}

void LumatoneController::resetFaderConfig()
{
    midiDriver.resetFaderConfig();
}

void LumatoneController::setAftertouchEnabled(bool value)
{
    midiDriver.sendAfterTouchActivation(value);
}

void LumatoneController::startCalibrateAftertouch()
{
    midiDriver.sendCalibrateAfterTouch();
}

void LumatoneController::setAftertouchConfig(const juce::uint8 aftertouchTable[])
{
    midiDriver.sendAftertouchConfig(aftertouchTable);
    editorListeners.call(&LumatoneEditor::EditorListener::tableChanged, LumatoneConfigTable::TableType::afterTouch, aftertouchTable, VELOCITYINTERVALTABLESIZE);
}

void LumatoneController::resetAftertouchConfig()
{
    midiDriver.resetAftertouchConfig();
}

void LumatoneController::setVelocityIntervalConfig(const int velocityIntervalTable[])
{
    midiDriver.sendVelocityIntervalConfig(velocityIntervalTable);
}

void LumatoneController::getRedLEDConfig(int boardIndex)
{
    midiDriver.sendRedLEDConfigRequest(boardIndex);
}

void LumatoneController::getGreenLEDConfig(int boardIndex)
{
    midiDriver.sendGreenLEDConfigRequest(boardIndex);
}

void LumatoneController::getBlueLEDConfig(int boardIndex)
{
    midiDriver.sendBlueLEDConfigRequest(boardIndex);
}

void LumatoneController::getChannelConfig(int boardIndex)
{
    midiDriver.sendChannelConfigRequest(boardIndex);
}

void LumatoneController::getNoteConfig(int boardIndex)
{
    midiDriver.sendNoteConfigRequest(boardIndex);
}

void LumatoneController::getKeyTypeConfig(int boardIndex)
{
    midiDriver.sendKeyTypeConfigRequest(boardIndex);
}

void LumatoneController::sendVelocityConfigRequest()
{
    midiDriver.sendVelocityConfigRequest();
}

void LumatoneController::sendFaderConfigRequest()
{
    midiDriver.sendFaderConfigRequest();
}

void LumatoneController::sendAftertouchConfigRequest()
{
    midiDriver.sendAftertouchConfigRequest();
}

void LumatoneController::sendVelocityIntervalConfigRequest()
{
    midiDriver.sendVelocityIntervalConfigRequest();
}

void LumatoneController::getFaderTypeConfig(int boardIndex)
{
    midiDriver.sendFaderTypeConfigRequest(boardIndex);
}

// This command is used to read back the serial identification number of the keyboard.
void LumatoneController::sendGetSerialIdentityRequest()
{
    midiDriver.sendGetSerialIdentityRequest();
}

void LumatoneController::startCalibrateKeys()
{
    midiDriver.sendCalibrateKeys();
}

void LumatoneController::setCalibratePitchModWheel(bool startCalibration)
{
    midiDriver.sendCalibratePitchModWheel(startCalibration);
}

void LumatoneController::setLumatouchConfig(const juce::uint8 lumatouchTable[])
{
    midiDriver.setLumatouchConfig(lumatouchTable);
    editorListeners.call(&LumatoneEditor::EditorListener::tableChanged, LumatoneConfigTable::TableType::lumaTouch, lumatouchTable, VELOCITYINTERVALTABLESIZE);
}

void LumatoneController::resetLumatouchConfig()
{
    midiDriver.resetLumatouchConfig();
}

void LumatoneController::getLumatouchConfig()
{
    midiDriver.sendLumatouchConfigRequest();
}

// This command is used to read back the current Lumatone firmware revision.
void LumatoneController::sendGetFirmwareRevisionRequest()
{
    midiDriver.sendGetFirmwareRevisionRequest();
}

// Send a value for the Lumatone to echo back; used for auto device connection and monitoring
int LumatoneController::pingLumatone(juce::uint8 pingId)
{
    return midiDriver.ping(pingId);
}

// Set MIDI Channels of peripheral controllers, pitch & mod wheels, expression & sustain pedals
void LumatoneController::setPeripheralChannels(int pitchWheelChannel, int modWheelChannel, int expressionChannel, int sustainChannel)
{
    if (firmwareSupport.versionAcknowledgesCommand(getLumatoneVersion(), SET_PERIPHERAL_CHANNELS))
        midiDriver.setPeripheralChannels(pitchWheelChannel, modWheelChannel, expressionChannel, sustainChannel);
}

void LumatoneController::setPeripheralChannels(PeripheralChannelSettings channelSettings)
{
    setPeripheralChannels(channelSettings.pitchWheel, channelSettings.modWheel, channelSettings.expressionPedal, channelSettings.sustainPedal);
}

// Get MIDI Channels of peripheral controllers, pitch & mod wheels, expression & sustain pedals
void LumatoneController::getPeripheralChannels()
{
    if (firmwareSupport.versionAcknowledgesCommand(getLumatoneVersion(), GET_PERIPHERAL_CHANNELS))
        midiDriver.getPeripheralChannels();
}

void LumatoneController::invertSustainPedal(bool setInverted)
{
    setInvertSustain(setInverted);

    if (firmwareSupport.versionAcknowledgesCommand(getLumatoneVersion(), INVERT_SUSTAIN_PEDAL))
        midiDriver.sendInvertSustainPedal(setInverted);
}

void LumatoneController::resetPresetsToFactoryDefault()
{
    if (firmwareSupport.versionAcknowledgesCommand(getLumatoneVersion(), RESET_DEFAULT_PRESETS))
    {
        for (int i = 0; i < 10; i++)
            midiDriver.sendResetDefaultPresetsRequest(i);
    }
}

void LumatoneController::resetPresetToFactoryDefault(int presetIndex)
{
    midiDriver.sendResetDefaultPresetsRequest(presetIndex);
}

// Get interaction flags of current preset
void LumatoneController::requestPresetFlags()
{
    if (firmwareSupport.versionAcknowledgesCommand(getLumatoneVersion(), GET_PRESET_FLAGS))
        midiDriver.sendGetPresetFlagsReset();
}

// Get sensitivity setting of expression pedal
void LumatoneController::requestExpressionPedalSensitivity()
{
    if (firmwareSupport.versionAcknowledgesCommand(getLumatoneVersion(), GET_EXPRESSION_PEDAL_SENSITIVIY))
        midiDriver.sendGetExpressionPedalSensitivity();
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

void LumatoneController::addMidiListener(juce::MidiKeyboardStateListener* listener) const
{
    eventManager->addListener(listener);
}

void LumatoneController::removeMidiListener(juce::MidiKeyboardStateListener* listener) const
{
    eventManager->removeListener(listener);
}


// LumatoneEditor::FirmwareListener Implementation

void LumatoneController::serialIdentityReceived(const int* serialBytes)
{
    juce::String serialNumber = firmwareSupport.serialIdentityToString(serialBytes);
    DBG("Device serial is: " + serialNumber);

    setConnectedSerialNumber(serialNumber);
}

void LumatoneController::firmwareRevisionReceived(FirmwareVersion version)
{
    setFirmwareVersion(version, true);
}

void LumatoneController::octaveColourConfigReceived(int boardId, juce::uint8 rgbFlag, const int* colourData) 
{
    auto octaveSize = getOctaveBoardSize();
    auto numBoards = getNumBoards();

    for (int keyIndex = 0; keyIndex < octaveSize; keyIndex++)
    {
        LumatoneKey* keyData = getEditKey(boardId - 1, keyIndex);
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

    editorListeners.call(&LumatoneEditor::EditorListener::boardChanged, *getBoard(boardId - 1));
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


//FirmwareSupport::Error LumatoneController::handlePingResponse(const juce::MidiMessage& midiMessage)
//{
//    unsigned int value = 0;
//    auto errorCode = midiDriver.unpackPingResponse(midiMessage, value);
//    
//    if (errorCode != FirmwareSupport::Error::noError)
//        return errorCode;
//
//    firmwareListeners.call(&LumatoneEditor::FirmwareListener::pingResponseReceived, lastTestDeviceResponded, value);
//
//    if (midiDriver.hasDevicesDefined() && !currentDevicePairConfirmed)
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
//            midiDriver.setMidiInput(newInput);
//            midiDriver.setMidiOutput(newOutput);
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
//            jassert(midiDriver.hasDevicesDefined());
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

//void LumatoneController::confirmAutoConnection()
//{
//    if (midiDriver.hasDevicesDefined() && !currentDevicePairConfirmed)
//    {
//        if (connectedSerialNumber.isEmpty())
//        {
//            sendGetSerialIdentityRequest();
//        }
//        else
//        {
//            sendGetFirmwareRevisionRequest();
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
