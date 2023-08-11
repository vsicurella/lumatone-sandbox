/*
  ==============================================================================

    LumatoneEventManager.cpp
    Created: 5 Jun 2023 8:19:04pm
    Author:  Vincenzo

  ==============================================================================
*/

#include "LumatoneEventManager.h"


LumatoneEventManager::LumatoneEventManager(TerpstraMidiDriver& midiDriverIn, LumatoneState stateIn)
    : LumatoneMidiState(stateIn)
    , midiDriver(midiDriverIn)
{
    midiDriver.addMessageCollector(this);

    juce::MidiMessageCollector::reset(bufferReadTimeoutMs);
}

LumatoneEventManager::~LumatoneEventManager()
{
    midiDriver.removeMessageCollector(this);
}

//=============================================================================
// Communication and broadcasting

void LumatoneEventManager::midiMessageReceived(juce::MidiInput* source, const juce::MidiMessage& midiMessage)
{
    //bool connected = state.getConnectionState() >= ConnectionState::SEARCHING;
    //if (connected)
    //{
        if (midiMessage.isSysEx())
        {
            addMessageToQueue(midiMessage);
            readQueueSize.store(readQueueSize.load() + 1);

            if (!bufferReadRequested)
            {
                bufferReadRequested = true;
                startTimer(bufferReadTimeoutMs);
            }
        }
    //}

    if (!midiMessage.isSysEx())
    {
        processNextMidiEvent(midiMessage);
    }
}

void LumatoneEventManager::midiMessageSent(juce::MidiOutput* target, const juce::MidiMessage& midiMessage) { }

void LumatoneEventManager::midiSendQueueSize(int queueSize)
{
    sendQueueSize = queueSize;
}

//void LumatoneEventManager::generalLogMessage(juce::String textMessage, TerpstraMidiDriver::ErrorLevel errorLevel) { }

void LumatoneEventManager::noAnswerToMessage(juce::MidiInput* expectedDevice, const juce::MidiMessage& midiMessage)
{
    if (midiMessage.isSysEx())
    {
        //    callAfterDelay(bufferReadTimeoutMs, [&]() { firmwareListeners.call(&LumatoneEditor::FirmwareListener::noAnswerToCommand, midiMessage.getSysExData()[CMD_ID]); });

        //if (currentDevicePairConfirmed)
        //    statusListeners.call(&LumatoneEditor::StatusListener::connectionLost);
        //else
        //    statusListeners.call(&LumatoneEditor::StatusListener::connectionFailed);
    }
}

FirmwareSupport::Error LumatoneEventManager::handleOctaveConfigResponse(
    const juce::MidiMessage& midiMessage,
    std::function<FirmwareSupport::Error(const juce::MidiMessage&, int&, juce::uint8, int*)> unpackFunction,
    std::function<void(int, void*)> callbackFunctionIfNoError)
{
    int boardId = -1;
    int channelData[56];

    int boardSize = state.getOctaveBoardSize();
    auto errorCode = unpackFunction(midiMessage, boardId, boardSize, channelData);
    if (errorCode == FirmwareSupport::Error::noError)
    {
        callbackFunctionIfNoError(boardId, channelData);
    }

    return errorCode;
}

FirmwareSupport::Error LumatoneEventManager::handleTableConfigResponse(
    const juce::MidiMessage& midiMessage,
    std::function<FirmwareSupport::Error(const juce::MidiMessage&, int*)> unpackFunction,
    std::function<void(void*)> callbackFunctionIfNoError)
{
    int veloctiyData[128];
    auto errorCode = unpackFunction(midiMessage, veloctiyData);
    if (errorCode == FirmwareSupport::Error::noError)
        callbackFunctionIfNoError(veloctiyData);

    return errorCode;
}

FirmwareSupport::Error LumatoneEventManager::handleLEDConfigResponse(const juce::MidiMessage& midiMessage)
{
    int boardId = -1;
    int colourData[56];

    int cmd = firmwareSupport.getCommandNumber(midiMessage);
    FirmwareSupport::Error errorCode;

    auto version = state.getLumatoneVersion();
    auto boardSize = state.getOctaveBoardSize();

    // Use correct unpacking function
    if (version < LumatoneFirmwareVersion::VERSION_1_0_11)
    {
        errorCode = midiDriver.unpackGetLEDConfigResponse_Version_1_0_0(midiMessage, boardId, boardSize, colourData);
    }
    else
    {
        errorCode = midiDriver.unpackGetLEDConfigResponse(midiMessage, boardId, colourData);
    }

    if (errorCode == FirmwareSupport::Error::noError)
    {
        int colorCode = cmd - GET_RED_LED_CONFIG;
        firmwareListeners.call(&LumatoneEditor::FirmwareListener::octaveColourConfigReceived, boardId, colorCode, colourData);
    }

    return errorCode;
}

FirmwareSupport::Error LumatoneEventManager::handleChannelConfigResponse(const juce::MidiMessage& midiMessage)
{
    auto unpack = [&](const juce::MidiMessage& msg, int& boardId, juce::uint8 numKeys, int* data) {
        auto errorCode = midiDriver.unpackGetChannelConfigResponse(msg, boardId, numKeys, data);
        if (errorCode == FirmwareSupport::Error::noError)
            for (int i = 0; i < numKeys; i++)
                data[i]++; // MIDI Channels are 1-based
        return errorCode;
    };
    auto callback = [&](int boardId, void* data) { firmwareListeners.call(&LumatoneEditor::FirmwareListener::octaveChannelConfigReceived, boardId, (int*)data); };
    return handleOctaveConfigResponse(midiMessage, unpack, callback);
}

FirmwareSupport::Error LumatoneEventManager::handleNoteConfigResponse(const juce::MidiMessage& midiMessage)
{
    auto unpack = [&](const juce::MidiMessage& msg, int& boardId, juce::uint8 numKeys, int* data) {
        return midiDriver.unpackGetNoteConfigResponse(msg, boardId, numKeys, data);
    };
    auto callback = [&](int boardId, void* data) { firmwareListeners.call(&LumatoneEditor::FirmwareListener::octaveNoteConfigReceived, boardId, (int*)data); };
    return handleOctaveConfigResponse(midiMessage, unpack, callback);
}

FirmwareSupport::Error LumatoneEventManager::handleKeyTypeConfigResponse(const juce::MidiMessage& midiMessage)
{
    auto unpack = [&](const juce::MidiMessage& msg, int& boardId, juce::uint8 numKeys, int* data) {
        return midiDriver.unpackGetTypeConfigResponse(msg, boardId, numKeys, data);
    };
    auto callback = [&](int boardId, void* data) { firmwareListeners.call(&LumatoneEditor::FirmwareListener::keyTypeConfigReceived, boardId, (int*)data); };
    return handleOctaveConfigResponse(midiMessage, unpack, callback);
}

FirmwareSupport::Error LumatoneEventManager::handleVelocityConfigResponse(const juce::MidiMessage& midiMessage)
{
    auto unpack = [&](const juce::MidiMessage& msg, int* data) {
        return midiDriver.unpackGetVelocityConfigResponse(msg, data);
    };
    auto callback = [&](void* data) { firmwareListeners.call(&LumatoneEditor::FirmwareListener::velocityConfigReceived, (int*)data); };
    return handleTableConfigResponse(midiMessage, unpack, callback);
}

FirmwareSupport::Error LumatoneEventManager::handleAftertouchConfigResponse(const juce::MidiMessage& midiMessage)
{
    auto unpack = [&](const juce::MidiMessage& msg, int* data) {
        return midiDriver.unpackGetAftertouchConfigResponse(msg, data);
    };
    auto callback = [&](void* data) { firmwareListeners.call(&LumatoneEditor::FirmwareListener::aftertouchConfigReceived, (int*)data); };
    return handleTableConfigResponse(midiMessage, unpack, callback);
}

FirmwareSupport::Error LumatoneEventManager::handleVelocityIntervalConfigResponse(const juce::MidiMessage& midiMessage)
{
    auto unpack = [&](const juce::MidiMessage& msg, int* data) {
        return midiDriver.unpackGetVelocityIntervalConfigResponse(msg, data);
    };
    auto callback = [&](void* data) { firmwareListeners.call(&LumatoneEditor::FirmwareListener::velocityIntervalConfigReceived, (int*)data); };
    return handleTableConfigResponse(midiMessage, unpack, callback);
}

FirmwareSupport::Error LumatoneEventManager::handleFaderConfigResponse(const juce::MidiMessage& midiMessage)
{
    auto unpack = [&](const juce::MidiMessage& msg, int* data) {
        return midiDriver.unpackGetFaderConfigResponse(msg, data);
    };
    auto callback = [&](void* data) { firmwareListeners.call(&LumatoneEditor::FirmwareListener::faderConfigReceived, (int*)data); };
    return handleTableConfigResponse(midiMessage, unpack, callback);
}

FirmwareSupport::Error LumatoneEventManager::handleFaderTypeConfigResponse(const juce::MidiMessage& midiMessage)
{
    auto unpack = [&](const juce::MidiMessage& msg, int& boardId, juce::uint8 numKeys, int* data) {
        return midiDriver.unpackGetTypeConfigResponse(msg, boardId, numKeys, data);
    };
    auto callback = [&](int boardId, void* data) { firmwareListeners.call(&LumatoneEditor::FirmwareListener::faderTypeConfigReceived, boardId, (int*)data); };
    return handleOctaveConfigResponse(midiMessage, unpack, callback);
}

FirmwareSupport::Error LumatoneEventManager::handleSerialIdentityResponse(const juce::MidiMessage& midiMessage)
{
    int serialBytes[6];
    auto errorCode = midiDriver.unpackGetSerialIdentityResponse(midiMessage, serialBytes);
    if (errorCode != FirmwareSupport::Error::noError)
        return errorCode;

    firmwareListeners.call(&LumatoneEditor::FirmwareListener::serialIdentityReceived, serialBytes);

    return errorCode;
}

FirmwareSupport::Error LumatoneEventManager::handleFirmwareRevisionResponse(const juce::MidiMessage& midiMessage)
{
    int major, minor, revision;
    auto errorCode = midiDriver.unpackGetFirmwareRevisionResponse(midiMessage, major, minor, revision);
    if (errorCode != FirmwareSupport::Error::noError)
        return errorCode;

    auto version = FirmwareVersion(major, minor, revision);
    DBG("Firmware version is: " + version.toString());

    firmwareListeners.call(&LumatoneEditor::FirmwareListener::firmwareRevisionReceived, version);

    return FirmwareSupport::Error::noError;
}

FirmwareSupport::Error LumatoneEventManager::handleLumatouchConfigResponse(const juce::MidiMessage& midiMessage)
{
    auto unpack = [&](const juce::MidiMessage& msg, int* data) {
        return midiDriver.unpackGetLumatouchConfigResponse(msg, data);
    };
    auto callback = [&](void* data) { firmwareListeners.call(&LumatoneEditor::FirmwareListener::lumatouchConfigReceived, (int*)data); };
    return handleTableConfigResponse(midiMessage, unpack, callback);
}

FirmwareSupport::Error LumatoneEventManager::handlePingResponse(const juce::MidiMessage& midiMessage)
{
    unsigned int value = 0;
    auto errorCode = midiDriver.unpackPingResponse(midiMessage, value);

    if (errorCode != FirmwareSupport::Error::noError)
        return errorCode;

    firmwareListeners.call(&LumatoneEditor::FirmwareListener::pingResponseReceived, value);

    return errorCode;
}

FirmwareSupport::Error LumatoneEventManager::handleGetPeripheralChannelResponse(const juce::MidiMessage& midiMessage)
{
    auto channelSettings = PeripheralChannelSettings();
    auto errorCode = midiDriver.unpackGetPeripheralChannelsResponse(midiMessage,
        channelSettings.pitchWheel,
        channelSettings.modWheel,
        channelSettings.expressionPedal,
        channelSettings.sustainPedal
    );

    firmwareListeners.call(&LumatoneEditor::FirmwareListener::peripheralMidiChannelsReceived, channelSettings);

    return errorCode;
}

FirmwareSupport::Error LumatoneEventManager::handleGetPresetFlagsResponse(const juce::MidiMessage& midiMessage)
{
    auto presetFlags = PresetFlags();
    auto errorCode = midiDriver.unpackGetPresetFlagsResponse(midiMessage,
        presetFlags.expressionPedalInverted,
        presetFlags.lightsOnKeystroke,
        presetFlags.polyphonicAftertouch,
        presetFlags.sustainPedalInverted
    );

    firmwareListeners.call(&LumatoneEditor::FirmwareListener::presetFlagsReceived, presetFlags);

    return errorCode;
}

FirmwareSupport::Error LumatoneEventManager::handleGetExpressionPedalSensitivityResponse(const juce::MidiMessage& midiMessage)
{
    int sensitivity = 127;
    auto errorCode = midiDriver.unpackGetExpressionPedalSensitivityResponse(midiMessage, sensitivity);

    firmwareListeners.call(&LumatoneEditor::FirmwareListener::expressionPedalSensitivityReceived, sensitivity);

    return errorCode;
}

FirmwareSupport::Error LumatoneEventManager::handlePeripheralCalibrationData(const juce::MidiMessage& midiMessage)
{
    int mode = -1;
    auto errorCode = midiDriver.unpackPeripheralCalibrationMode(midiMessage, mode);

    if (errorCode != FirmwareSupport::Error::noError)
        return errorCode;

    switch (mode)
    {
    case PeripheralCalibrationDataMode::ExpressionPedal:
        errorCode = handleExpressionPedalCalibrationData(midiMessage);
        break;
    case PeripheralCalibrationDataMode::PitchAndModWheels:
        errorCode = handleWheelsCalibrationData(midiMessage);
        break;
    default:
        errorCode = FirmwareSupport::Error::messageIsNotResponseToCommand;
    }

    if (errorCode != FirmwareSupport::Error::noError)
        return errorCode;

    return errorCode;
}

FirmwareSupport::Error LumatoneEventManager::handleExpressionPedalCalibrationData(const juce::MidiMessage& midiMessage)
{
    int minBound = 0;
    int maxBound = 0;
    bool isValid = false;

    auto errorCode = midiDriver.unpackExpressionPedalCalibrationPayload(midiMessage, minBound, maxBound, isValid);

    firmwareListeners.call(&LumatoneEditor::FirmwareListener::pedalCalibrationDataReceived, minBound, maxBound, isValid);

    return errorCode;
}

FirmwareSupport::Error LumatoneEventManager::handleWheelsCalibrationData(const juce::MidiMessage& midiMessage)
{
    WheelsCalibrationData calibrationData;
    auto errorCode = midiDriver.unpackWheelsCalibrationPayload(midiMessage,
        calibrationData.centerPitch,
        calibrationData.minPitch,
        calibrationData.maxPitch,
        calibrationData.minMod,
        calibrationData.maxMod
    );

    firmwareListeners.call(&LumatoneEditor::FirmwareListener::wheelsCalibrationDataReceived, calibrationData);
    return errorCode;
}

void LumatoneEventManager::handleMidiDriverError(FirmwareSupport::Error errorToHandle, int commandReceived)
{
    switch (errorToHandle)
    {
    case FirmwareSupport::Error::noError:
    case FirmwareSupport::Error::messageIsAnEcho:
    case FirmwareSupport::Error::commandNotImplemented:
        return;

    case FirmwareSupport::Error::messageHasInvalidStatusByte:
        return;

    default:
        DBG("ERROR from command " + juce::String::toHexString(commandReceived) + ": " + firmwareSupport.errorToString(errorToHandle));
    }

    jassertfalse;
}


FirmwareSupport::Error LumatoneEventManager::getBufferErrorCode(const juce::uint8* sysExData)
{
    switch (sysExData[MSG_STATUS])
    {
    case TerpstraMIDIAnswerReturnCode::NACK:  // Not recognized
        //errorVisualizer.setErrorLevel(
        //    TerpstraMidiDriver::ErrorLevel::error,
        //    "Not Recognized");
        return FirmwareSupport::Error::unknownCommand;
        break;

    case TerpstraMIDIAnswerReturnCode::ACK:  // Acknowledged, OK
        //errorVisualizer.setErrorLevel(
        //    TerpstraMidiDriver::ErrorLevel::noError,
        //    "Ack");
        break;

    case TerpstraMIDIAnswerReturnCode::BUSY: // Controller busy
        //errorVisualizer.setErrorLevel(
        //    TerpstraMidiDriver::ErrorLevel::warning,
        //    "Busy");
        return FirmwareSupport::Error::deviceIsBusy;
        break;

    case TerpstraMIDIAnswerReturnCode::ERROR:    // Error
        //errorVisualizer.setErrorLevel(
        //    TerpstraMidiDriver::ErrorLevel::error,
        //    "Error from device");
        return FirmwareSupport::Error::externalError;
        break;

    case TEST_ECHO:
        return FirmwareSupport::Error::messageIsAnEcho;
        break;

    default:
        //errorVisualizer.setErrorLevel(
        //    TerpstraMidiDriver::ErrorLevel::noError,
        //    "");
        break;
    }

    return FirmwareSupport::Error::noError;
}

FirmwareSupport::Error LumatoneEventManager::handleBufferCommand(const juce::MidiMessage& midiMessage)
{
    auto sysExData = midiMessage.getSysExData();
    unsigned int cmd = sysExData[CMD_ID];

    switch (cmd)
    {
    case GET_RED_LED_CONFIG:
        return handleLEDConfigResponse(midiMessage);

    case GET_GREEN_LED_CONFIG:
        return handleLEDConfigResponse(midiMessage);

    case GET_BLUE_LED_CONFIG:
        return handleLEDConfigResponse(midiMessage);

    case GET_CHANNEL_CONFIG:
        return handleChannelConfigResponse(midiMessage);

    case GET_NOTE_CONFIG:
        return handleNoteConfigResponse(midiMessage);

    case GET_KEYTYPE_CONFIG:
        return handleKeyTypeConfigResponse(midiMessage);

        // TODO

    case GET_VELOCITY_CONFIG:
        return handleVelocityConfigResponse(midiMessage);

    case GET_FADER_TYPE_CONFIGURATION:
        return handleFaderTypeConfigResponse(midiMessage);

    case GET_SERIAL_IDENTITY:
        return handleSerialIdentityResponse(midiMessage);

    case CALIBRATE_PITCH_MOD_WHEEL:
        firmwareListeners.call(&LumatoneEditor::FirmwareListener::calibratePitchModWheelAnswer, (TerpstraMIDIAnswerReturnCode)sysExData[MSG_STATUS]);
        return FirmwareSupport::Error::noError;

    case GET_LUMATOUCH_CONFIG:
        return handleLumatouchConfigResponse(midiMessage);

    case GET_FIRMWARE_REVISION:
        return handleFirmwareRevisionResponse(midiMessage);

    case LUMA_PING:
        return handlePingResponse(midiMessage);

    case GET_PERIPHERAL_CHANNELS:
        return handleGetPeripheralChannelResponse(midiMessage);

    case PERIPHERAL_CALBRATION_DATA:
        return handlePeripheralCalibrationData(midiMessage);

    case GET_PRESET_FLAGS:
        return handleGetPresetFlagsResponse(midiMessage);

    case GET_EXPRESSION_PEDAL_SENSITIVIY:
        return handleGetExpressionPedalSensitivityResponse(midiMessage);

    case SET_VELOCITY_CONFIG:
        DBG("Send layout complete.");
        // loadRandomMapping(1000, 1); // uncomment for test sequence
        return FirmwareSupport::Error::noError;

    default:
        jassert(sysExData[MSG_STATUS] == TerpstraMIDIAnswerReturnCode::ACK);
        if (midiMessage.getRawDataSize() <= 8)
        {
            // Simple confirmation
            return FirmwareSupport::Error::noError;
        }
        else
        {
            // Returned data
            DBG("WARNING: UNIMPLEMENTED RESPONSE HANDLING CMD " + juce::String(sysExData[CMD_ID]));
            return FirmwareSupport::Error::commandNotImplemented;
        }
    }

    return FirmwareSupport::Error::unknownCommand;
}

void LumatoneEventManager::timerCallback()
{
    stopTimer();

    juce::MidiBuffer readBuffer;
    removeNextBlockOfMessages(readBuffer, bufferReadSize);
    for (auto event : readBuffer)
    {
        auto midiMessage = event.getMessage();
        DBG("READ: " + midiMessage.getDescription());
        if (midiMessage.isSysEx())
        {
            auto sysExData = midiMessage.getSysExData();
            auto cmd = sysExData[CMD_ID];

            auto errorCode = getBufferErrorCode(sysExData);
            handleMidiDriverError(errorCode, cmd);

            if (sysExData[MSG_STATUS] == 1)
            {
                errorCode = handleBufferCommand(midiMessage);
                handleMidiDriverError(errorCode, cmd);
            }
        }

        // Ignore non-sysex messages
    }

    auto bufferSize = juce::jlimit(0, 999999, readQueueSize.load() - bufferReadSize);
    readQueueSize.store(bufferSize);

    if (bufferSize != 0)
        startTimer(bufferReadTimeoutMs);

    bufferReadRequested = false;
}
