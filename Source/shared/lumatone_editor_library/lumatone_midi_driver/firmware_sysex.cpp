#include "firmware_sysex.h"

void LumatoneSysEx::fillManufacturerId(unsigned char* data)
{
    data[0] = MANUFACTURER_ID_0;
    data[1] = MANUFACTURER_ID_1;
    data[2] = MANUFACTURER_ID_2;
}

juce::MidiMessage LumatoneSysEx::createTerpstraSysEx(juce::uint8 boardIndex, juce::uint8 cmd, juce::uint8 data1, juce::uint8 data2, juce::uint8 data3, juce::uint8 data4)
{
    unsigned char sysExData[9];
    fillManufacturerId(sysExData);
    sysExData[3] = boardIndex;
    sysExData[4] = cmd;
    sysExData[5] = data1;
    sysExData[6] = data2;
    sysExData[7] = data3;
    sysExData[8] = data4;

    juce::MidiMessage msg = juce::MidiMessage::createSysExMessage(sysExData, 9);
    return msg;
}

// Create a SysEx message to send 8-bit color precision
juce::MidiMessage LumatoneSysEx::createExtendedKeyColourSysEx(juce::uint8 boardIndex, juce::uint8 cmd, juce::uint8 keyIndex, juce::uint8 redUpper, juce::uint8 redLower, juce::uint8 greenUpper, juce::uint8 greenLower, juce::uint8 blueUpper, juce::uint8 blueLower)
{
    unsigned char sysExData[12];
    fillManufacturerId(sysExData);
    sysExData[3] = boardIndex;
    sysExData[4] = cmd;
    sysExData[5] = keyIndex;
    sysExData[6] = redUpper;
    sysExData[7] = redLower;
    sysExData[8] = greenUpper;
    sysExData[9] = greenLower;
    sysExData[10] = blueUpper;
    sysExData[11] = blueLower;

    juce::MidiMessage msg = juce::MidiMessage::createSysExMessage(sysExData, 12);
    return msg;
}
juce::MidiMessage LumatoneSysEx::createExtendedKeyColourSysEx(juce::uint8 boardIndex, juce::uint8 cmd, juce::uint8 keyIndex, int red, int green, int blue)
{
    return createExtendedKeyColourSysEx(boardIndex, cmd, keyIndex, red >> 4, red & 0xf, green >> 4, green & 0xf, blue >> 4, blue & 0xf);
}

juce::MidiMessage LumatoneSysEx::createExtendedMacroColourSysEx(juce::uint8 cmd, juce::uint8 redUpper, juce::uint8 redLower, juce::uint8 greenUpper, juce::uint8 greenLower, juce::uint8 blueUpper, juce::uint8 blueLower)
{
    unsigned char sysExData[11];
    fillManufacturerId(sysExData);
    sysExData[3] = 0;
    sysExData[4] = cmd;
    sysExData[5] = redUpper;
    sysExData[6] = redLower;
    sysExData[7] = greenUpper;
    sysExData[8] = greenLower;
    sysExData[9] = blueUpper;
    sysExData[10] = blueLower;

    juce::MidiMessage msg = juce::MidiMessage::createSysExMessage(sysExData, 11);
    return msg;
}

juce::MidiMessage LumatoneSysEx::createExtendedMacroColourSysEx(juce::uint8 cmd, int red, int green, int blue)
{
    return createExtendedMacroColourSysEx(cmd, red >> 4, red & 0xf, green >> 4, green & 0xf, blue >> 4, blue & 0xf);
}

juce::MidiMessage LumatoneSysEx::createTableSysEx(juce::uint8 boardIndex, juce::uint8 cmd, juce::uint8 tableSize, const juce::uint8 table[])
{
    size_t msgSize = tableSize + 5;
    juce::Array<unsigned char> dataArray;
    dataArray.resize(msgSize);

    auto sysExData = dataArray.getRawDataPointer();
    fillManufacturerId(sysExData);

    sysExData[3] = '\0';
    sysExData[4] = cmd;

    memmove(&sysExData[5], table, sizeof(juce::uint8) * tableSize);

#if JUCE_DEBUG
    for (int i = 0; i < tableSize; i++)
        jassert(table[i] <= 0x7f);
#endif

    auto msg = juce::MidiMessage::createSysExMessage(sysExData, msgSize);
    return msg;
}

juce::Colour LumatoneSysEx::parseLedIntensity8Bit(const juce::uint8 *data)
{
    return juce::Colour::fromRGB(
        *data << 4 | *(data + 1),
        *(data + 3) << 4 | *(data + 2),
        *(data + 5) << 4 | *(data + 4)
    );
}

FirmwareSupport::Error LumatoneSysEx::isValid(const juce::MidiMessage &response, size_t numBytes)
{
    auto status = messageIsValidLumatoneResponse(response);
    if (status != FirmwareSupport::Error::noError)
        return status;

    status = responseIsExpectedLength(response, numBytes);
    if (status != FirmwareSupport::Error::noError)
        return status;

    return FirmwareSupport::Error::noError;
}

// Checks if message is a valid Lumatone firmware response and is expected length, then runs supplied unpacking function or returns an error code 
FirmwareSupport::Error LumatoneSysEx::unpackIfValid(const juce::MidiMessage& response, size_t numBytes, std::function<FirmwareSupport::Error(const juce::uint8*)> unpackFunction)
{
    auto status = isValid(response, numBytes);
    if (status != FirmwareSupport::Error::noError)
        return status;

    return unpackFunction(&response.getSysExData()[PAYLOAD_INIT]);
}

// Generic unpacking of octave data from a SysEx message
FirmwareSupport::Error LumatoneSysEx::unpackOctaveConfig(const juce::MidiMessage& msg, int& boardId, size_t numBytes, int* keyData, std::function<FirmwareSupport::Error(const juce::MidiMessage&, size_t, int*)> nBitUnpackFunction)
{
    auto status = nBitUnpackFunction(msg, numBytes, keyData);
    if (status != FirmwareSupport::Error::noError)
        return status;

    boardId = msg.getSysExData()[BOARD_IND];
    return status;
}

// Generic unpacking of 7-bit data from a SysEx message
FirmwareSupport::Error LumatoneSysEx::unpack7BitData(const juce::MidiMessage& msg, size_t numBytes, int* unpackedData)
{
    auto unpack = [&](const juce::uint8* payload) {
        for (int i = 0; i < numBytes; i++)
            unpackedData[i] = payload[i];
        return FirmwareSupport::Error::noError;
    };

    return unpackIfValid(msg, numBytes, unpack);
}

// Unpacking of octave-based 7-bit key configuration data
FirmwareSupport::Error LumatoneSysEx::unpack7BitOctaveData(const juce::MidiMessage& msg, int& boardId, juce::uint8 numKeys, int* keyData)
{
    return unpackOctaveConfig(msg, boardId, numKeys, keyData, [&](const juce::MidiMessage&, juce::uint8, int*) {
        return unpack7BitData(msg, numKeys, keyData);
    });
}

// Generic unpacking of 8-bit data from a SysEx message
FirmwareSupport::Error LumatoneSysEx::unpack8BitData(const juce::MidiMessage& msg, size_t numBytes, int* unpackedData)
{
    auto unpack = [&](const juce::uint8* payload) {
        auto numValues = numBytes / 2;
        for (int i = 0; i < numValues; i++)
            unpackedData[i] = (payload[(i * 2)] << 4) | (payload[(i * 2) + 1]);
        return FirmwareSupport::Error::noError;
    };

    return unpackIfValid(msg, numBytes, unpack);
}

// Unpacking of octave-based 8-bit data
FirmwareSupport::Error LumatoneSysEx::unpack8BitOctaveData(const juce::MidiMessage& msg, int& boardId, juce::uint8 numKeys, int* keyData)
{
    return unpackOctaveConfig(msg, boardId, numKeys, keyData, [&](const juce::MidiMessage&, size_t, int*) {
        return unpack8BitData(msg, numKeys * 2, keyData);
    });
}

// Generic unpacking of 12-bit data from a SysEx message, when packed with two 7-bit values
FirmwareSupport::Error LumatoneSysEx::unpack12BitDataFrom7Bit(const juce::MidiMessage& msg, size_t numBytes, int* unpackedData)
{
    auto unpack = [&](const juce::uint8* payload) {
        auto numValues = numBytes / 2;
        for (int i = 0; i < numValues; i++)
            unpackedData[i] = (payload[i * 2] << 6) | (payload[(i * 2) + 1]);
        return FirmwareSupport::Error::noError;
    };

    return unpackIfValid(msg, numBytes, unpack);
}

// Generic unpacking of 12-bit data from a SysEx message, when packed with three 4-bit values
FirmwareSupport::Error LumatoneSysEx::unpack12BitDataFrom4Bit(const juce::MidiMessage& msg, size_t numBytes, int* unpackedData)
{
    auto unpack = [&](const juce::uint8* payload) {
        auto numValues = numBytes / 3;
        for (int i = 0; i < numValues; i++)
        {
            int start = i * 3;
            unpackedData[i] = (payload[start] << 8) | (payload[start + 1] << 4) | (payload[start + 2]);
        }
        return FirmwareSupport::Error::noError;
    };

    return unpackIfValid(msg, numBytes, unpack);
}

bool LumatoneSysEx::messageIsResponseToMessage(const juce::MidiMessage& answer, const juce::MidiMessage& originalMessage)
{
    // Only for SysEx messages
    if (answer.isSysEx() != originalMessage.isSysEx())
        return false;

    auto answerSysExData = answer.getSysExData();
    auto originalSysExData = originalMessage.getSysExData();

    // Manufacturer Id, board index, command coincide?
    if (answerSysExData[0] != originalSysExData[0] ||
        answerSysExData[1] != originalSysExData[1] ||
        answerSysExData[2] != originalSysExData[2] ||
        answerSysExData[3] != originalSysExData[3] ||
        answerSysExData[4] != originalSysExData[4])
    {
        return false;
    }
    else
    {
        return true;
    }
}

FirmwareSupport::Error LumatoneSysEx::messageIsValidLumatoneResponse(const juce::MidiMessage& midiMessage)
{
    if (!midiMessage.isSysEx())
        return FirmwareSupport::Error::messageIsNotSysEx;

    auto sysExData = midiMessage.getSysExData();

    if (midiMessage.getSysExDataSize() < PAYLOAD_INIT)
        return FirmwareSupport::Error::messageTooShort;

    if (!(sysExData[MANU_0] == MANUFACTURER_ID_0) &&
        !(sysExData[MANU_1] == MANUFACTURER_ID_1) &&
        !(sysExData[MANU_2] == MANUFACTURER_ID_2))
        return FirmwareSupport::Error::messageHasIncorrectManufacturerId;

    if (sysExData[BOARD_IND] > BOARD_OCT_5)
        return FirmwareSupport::Error::messageHasInvalidBoardIndex;

    if (sysExData[MSG_STATUS] > LumatoneFirmware::ReturnCode::ERROR)
        return FirmwareSupport::Error::messageHasInvalidStatusByte;

    return FirmwareSupport::Error::noError;
}

FirmwareSupport::Error LumatoneSysEx::responseIsExpectedLength(const juce::MidiMessage& midiMessage, size_t numPayloadBytes)
{
    auto size = midiMessage.getSysExDataSize();
    auto expected = numPayloadBytes + PAYLOAD_INIT;

    if (size == expected)
        return FirmwareSupport::Error::noError;

    else if (size < expected)
        return FirmwareSupport::Error::messageTooShort;

    else
        return FirmwareSupport::Error::messageTooLong;
}

// For CMD 13h response: unpacks 8-bit key data for red LED intensity. 112 bytes, lower and upper nibbles for 56 values
FirmwareSupport::Error LumatoneSysEx::unpackGetLEDConfigResponse(const juce::MidiMessage& response, int& boardId, int* keyData)
{
    // TODO: Maybe should define keys per octave somewhere
    return unpack8BitOctaveData(response, boardId, 56, keyData);
}

// For CMD 13h response: unpacks key data for red LED intensity. 55 or 56 bytes, each value must be multiplied by 5
FirmwareSupport::Error LumatoneSysEx::unpackGetLEDConfigResponse_Version_1_0_0(const juce::MidiMessage& response, int& boardId, juce::uint8 numKeys, int* keyData)
{
    // Custom unpacking function for intensity multiplication
    auto unpackFunction = [&](const juce::uint8* payload) {
        for (int i = 0; i < numKeys; i++)
            keyData[i] = ((int)payload[i] * 5) & 0xff;
        return FirmwareSupport::Error::noError;
    };

    return unpackOctaveConfig(response, boardId, numKeys, keyData, [&](const juce::MidiMessage&, juce::uint8, int*) {
        return unpackIfValid(response, numKeys, unpackFunction);
    });
}

// For CMD 16h response: unpacks channel data for note configuration. 55 or 56 bytes
FirmwareSupport::Error LumatoneSysEx::unpackGetChannelConfigResponse(const juce::MidiMessage& response, int& boardId, juce::uint8 numKeys, int* keyData)
{
    return unpack7BitOctaveData(response, boardId, numKeys, keyData);
}

// For CMD 17h response: unpacks 7-bit key data for note configuration. 55 or 56 bytes
FirmwareSupport::Error LumatoneSysEx::unpackGetNoteConfigResponse(const juce::MidiMessage& response, int& boardId, juce::uint8 numKeys, int* keyData)
{
    return unpack7BitOctaveData(response, boardId, numKeys, keyData);
}

// For CMD 18h response: unpacks 7-bit key type data for key configuration. 55 or 56 bytes
FirmwareSupport::Error LumatoneSysEx::unpackGetTypeConfigResponse(const juce::MidiMessage& response, int& boardId, juce::uint8 numKeys, int* keyData)
{
    return unpack7BitOctaveData(response, boardId, numKeys, keyData);
}

// For CMD 19h response: unpacks 8-bit key data for maximums of adc threshold. 55 or 56 bytes
FirmwareSupport::Error LumatoneSysEx::unpackGetKeyMaxThresholdsResponse(const juce::MidiMessage& response, int& boardId, juce::uint8 numKeys, int* keyData)
{
    return unpack8BitData(response, numKeys, keyData);
}

// For CMD 1Ah response: unpacks 8-bit key data for minimums of adc threshold. 55 or 56 bytes
FirmwareSupport::Error LumatoneSysEx::unpackGetKeyMinThresholdsResponse(const juce::MidiMessage& response, int& boardId, juce::uint8 numKeys, int* keyData)
{
    return unpack8BitOctaveData(response, boardId, numKeys, keyData);
}

// For CMD 1Bh response: unpacks 8-bit key data for maximums of adc threshold for aftertouch triggering. 55 or 56 bytes
FirmwareSupport::Error LumatoneSysEx::unpackGetAftertouchMaxThresholdsResponse(const juce::MidiMessage& response, int& boardId, juce::uint8 numKeys, int* keyData)
{
    return unpack8BitOctaveData(response, boardId, numKeys, keyData);
}

// For CMD 1Ch response: unpacks boolean key validity data for board, whether or not each key meets threshold specs
FirmwareSupport::Error LumatoneSysEx::unpackGetKeyValidityResponse(const juce::MidiMessage& response, int& boardId, juce::uint8 numKeys, bool* keyValidityData)
{
    // TODO: perhaps define boolean unpacking, or combine with 7-bit somehow

    // Custom unpacking function for boolean array
    auto unpackFunction = [&](const juce::uint8* payload) {
        for (int i = 0; i < numKeys; i++)
            keyValidityData[i] = (bool)payload[i];
        return FirmwareSupport::Error::noError;
    };

    auto status = unpackIfValid(response, numKeys, unpackFunction);
    if (status != FirmwareSupport::Error::noError)
        return status;

    boardId = response.getSysExData()[BOARD_IND];
    return status;
}

// For CMD 1Dh response: unpacks 7-bit velocity configuration of keyboard, 128 bytes
FirmwareSupport::Error LumatoneSysEx::unpackGetVelocityConfigResponse(const juce::MidiMessage& response, int* velocityData)
{
    return unpack7BitData(response, 128, velocityData);
}

// For CMD 1Eh response: unpacks 7-bit fader configuration of keyboard, 128 bytes
FirmwareSupport::Error LumatoneSysEx::unpackGetFaderConfigResponse(const juce::MidiMessage& response, int* faderData)
{
    return unpack7BitData(response, 128, faderData);
}

// For CMD 1Fh response: unpacks 7-bit aftertouch configuration of keyboard, 128 bytes
FirmwareSupport::Error LumatoneSysEx::unpackGetAftertouchConfigResponse(const juce::MidiMessage& response, int* aftertouchData)
{
    return unpack7BitData(response, 128, aftertouchData);
}

// For CMD 21h response: unpacks 12-bit velocity interval configuration of keyboard, 254 bytes encoding 127 values
FirmwareSupport::Error LumatoneSysEx::unpackGetVelocityIntervalConfigResponse(const juce::MidiMessage& response, int* intervalData)
{
    return unpack12BitDataFrom7Bit(response, 254, intervalData);
}

// For CMD 22h response: unpacks 7-bit fader type configuration of board, 56 bytes
FirmwareSupport::Error LumatoneSysEx::unpackGetFaderConfigResponse(const juce::MidiMessage& response, int& boardId, juce::uint8 numKeys, int* faderData)
{
    return unpackOctaveConfig(response, boardId, numKeys, faderData, [&](const juce::MidiMessage&, size_t, int*) {
        return unpack7BitData(response, numKeys, faderData);
    });
}

// For CMD 23h response: unpacks serial ID number of keyboard, 12 7-bit values encoding 6 bytes
FirmwareSupport::Error LumatoneSysEx::unpackGetSerialIdentityResponse(const juce::MidiMessage& response, int* serialBytes)
{
    // Check for echo first
    auto sysExData = response.getSysExData();
    if (sysExData[MSG_STATUS] == TEST_ECHO)
        return FirmwareSupport::Error::messageIsAnEcho;
    
    auto errorCode = FirmwareSupport::Error::noError;

    if (response.getSysExDataSize() == 18)
        errorCode = unpack8BitData(response, 12, serialBytes);
    else if (response.getSysExDataSize() < 18)
    {
        // Early firmware versions don't send serial payload
        errorCode = messageIsValidLumatoneResponse(response);
        if (errorCode == FirmwareSupport::Error::noError)
        {
            for (juce::uint8 i = 0; i < 6; i++)
                serialBytes[i] = 0;
        }
    }
    else
        errorCode = FirmwareSupport::Error::messageTooLong;

    return errorCode;
}

// For CMD 30h response: unpacks 7-bit Lumatouch configuration of keyboard, 128 bytes
FirmwareSupport::Error LumatoneSysEx::unpackGetLumatouchConfigResponse(const juce::MidiMessage& response, int* lumatouchData)
{
    return unpack7BitData(response, 128, lumatouchData);
}

// For CMD 31h response: unpacks firmware revision running on the keyboard
FirmwareSupport::Error LumatoneSysEx::unpackGetFirmwareRevisionResponse(const juce::MidiMessage& response, int& majorVersion, int& minorVersion, int& revision)
{
    auto status = messageIsValidLumatoneResponse(response);
    if (status != FirmwareSupport::Error::noError)
        return status;

    status = responseIsExpectedLength(response, 3);
    if (status != FirmwareSupport::Error::noError)
        return status;

    auto payload = &response.getSysExData()[PAYLOAD_INIT];
    majorVersion = payload[0];
    minorVersion = payload[1];
    revision     = payload[2];

    return status;
}

// For CMD 33h response: echo payload
// FirmwareSupport::Error LumatoneSysEx::unpackPingResponse(const juce::MidiMessage& response, int& value1, int& value2, int& value3, int& value4)
// {
//     auto status = messageIsValidLumatoneResponse(response);
//     if (status != FirmwareSupport::Error::noError)
//         return status;

//     status = responseIsExpectedLength(response, 4);
//     if (status != FirmwareSupport::Error::noError)
//         return status;

//     auto payload = &response.getSysExData()[PAYLOAD_INIT];
//     value1 = payload[0];
//     value2 = payload[1];
//     value3 = payload[2];
//     value4 = payload[3];

//     return status;
// }

// For CMD 33h response: echo payload
FirmwareSupport::Error LumatoneSysEx::unpackPingResponse(const juce::MidiMessage& response, unsigned int& value)
{
    // Check for echo first
    auto sysExData = response.getSysExData();
    if (sysExData[MSG_STATUS] == TEST_ECHO)
        return FirmwareSupport::Error::messageIsAnEcho;
    
    auto status = messageIsValidLumatoneResponse(response);
    if (status != FirmwareSupport::Error::noError)
        return status;

    status = responseIsExpectedLength(response, 4);
    if (status != FirmwareSupport::Error::noError)
        return status;

    auto payload = &sysExData[PAYLOAD_INIT];
    value = (payload[1] << 14) | (payload[2] << 7) | (payload[3]);

    return status;
}

// For CMD 3Ah response: retrieve all 8-bit threshold values of a certain board
FirmwareSupport::Error LumatoneSysEx::unpackGetBoardThresholdValuesResponse(const juce::MidiMessage& response, int& boardId, int& minHighThreshold, int& minLowThreshold, int& maxThreshold, int& aftertouchThreshold, int& ccThreshold)
{
    const short NUM_UNPACKED = 5;
    int unpackedData[NUM_UNPACKED];
    auto status = unpack8BitOctaveData(response, boardId, NUM_UNPACKED, unpackedData);
    if (status != FirmwareSupport::Error::noError)
        return status;

    minHighThreshold     = unpackedData[0];
    minLowThreshold      = unpackedData[1];
    maxThreshold         = unpackedData[2];
    aftertouchThreshold  = unpackedData[3];
    ccThreshold          = unpackedData[4];

    return status;
}

// For CMD 3Bh response: retrieve all threshold values of a certain board
FirmwareSupport::Error LumatoneSysEx::unpackGetBoardSensitivityValuesResponse(const juce::MidiMessage& response, int& boardId, int& ccSensitivity, int& aftertouchSensitivity)
{
    const short NUM_UNPACKED = 2;
    int unpackedData[NUM_UNPACKED];
    auto status = unpack8BitOctaveData(response, boardId, NUM_UNPACKED, unpackedData);
    if (status != FirmwareSupport::Error::noError)
        return status;

    ccSensitivity           = unpackedData[0];
    aftertouchSensitivity   = unpackedData[1];

    return status;
}

// For CMD 3Dh response: retrieve all threshold values of a certain board
FirmwareSupport::Error LumatoneSysEx::unpackGetPeripheralChannelsResponse(const juce::MidiMessage& response, int& pitchWheelChannel, int& modWheelChannel, int& expressionChannel, int& sustainPedalChannel)
{
    const short NUM_UNPACKED = 4;
    int unpackedData[NUM_UNPACKED];
    auto status = unpack7BitData(response, NUM_UNPACKED, unpackedData);
    if (status != FirmwareSupport::Error::noError)
        return status;

    pitchWheelChannel   = unpackedData[0] + 1;
    modWheelChannel     = unpackedData[1] + 1;
    expressionChannel   = unpackedData[2] + 1;
    sustainPedalChannel = unpackedData[3] + 1;

    return status;
}

// For CMD 3Eh response: read back the calibration mode of the message
FirmwareSupport::Error LumatoneSysEx::unpackPeripheralCalibrationMode(const juce::MidiMessage& response, int& calibrationMode)
{
    // Other errors will be caught in corresponding mode unpacking
    int msgSize = response.getSysExDataSize();
    int expectedSize = PAYLOAD_INIT + 15;
    
    if (msgSize < expectedSize)
        return FirmwareSupport::Error::messageTooShort;
    
    else if (msgSize > expectedSize)
        return FirmwareSupport::Error::messageTooLong;

    auto sysExData = response.getSysExData();
    calibrationMode = sysExData[CALIB_MODE];

    return FirmwareSupport::Error::noError;
}

// For CMD 3Eh response: retrieve 12-bit expression pedal calibration status values in respective mode, automatically sent every 100ms
FirmwareSupport::Error LumatoneSysEx::unpackExpressionPedalCalibrationPayload(const juce::MidiMessage& response, int& minBound, int& maxBound, bool& valid)
{
    const short NUM_UNPACKED = 15; // Actually two + boolean, but this message always returns 15-byte payload
    int unpackedData[NUM_UNPACKED];
    auto status = unpack12BitDataFrom4Bit(response, 15, unpackedData);
    if (status != FirmwareSupport::Error::noError)
        return status;

    minBound = unpackedData[0];
    maxBound = unpackedData[1];
    valid = response.getSysExData()[PAYLOAD_INIT + 3];
    
    return status;
}

// For CMD 3Eh response: retrieve 12-bit pitch & mod wheel calibration status values in respective mode, automatically sent every 100ms
FirmwareSupport::Error LumatoneSysEx::unpackWheelsCalibrationPayload(const juce::MidiMessage& response, int& centerPitch, int& minPitch, int& maxPitch, int& minMod, int& maxMod)
{
    const short NUM_UNPACKED = 15;
    int unpackedData[NUM_UNPACKED];
    auto status = unpack12BitDataFrom4Bit(response, NUM_UNPACKED, unpackedData);
    if (status != FirmwareSupport::Error::noError)
        return status;

    centerPitch = unpackedData[0];
    minPitch    = unpackedData[1];
    maxPitch    = unpackedData[2];
    minMod      = unpackedData[3];
    maxMod      = unpackedData[4];

    return status;
}

// For CMD 40h response: retrieve aftertouch trigger delay of a certain board
FirmwareSupport::Error LumatoneSysEx::unpackGetAftertouchTriggerDelayResponse(const juce::MidiMessage& response, int& boardId, int& triggerDelay)
{
    const short NUM_UNPACKED = 1;
    int unpackedData[NUM_UNPACKED];
    auto status = unpack8BitData(response, NUM_UNPACKED, unpackedData);
    if (status != FirmwareSupport::Error::noError)
        return status;

    triggerDelay = unpackedData[0];
    return status;
}

// For CMD 42h response: retrieve 12-bit Lumatouch note off delay of a certain board
FirmwareSupport::Error LumatoneSysEx::unpackGetLumatouchNoteOffDelayResponse(const juce::MidiMessage& response, int& boardId, int& delay)
{
    auto status = messageIsValidLumatoneResponse(response);
    if (status != FirmwareSupport::Error::noError)
        return status;

    status = responseIsExpectedLength(response, 3);
    if (status != FirmwareSupport::Error::noError)
        return status;

    auto payload = &response.getSysExData()[PAYLOAD_INIT];

    delay = (payload[0] << 8) | (payload[1] << 4) | payload[2];

    return status;
}

// For CMD 44h response: retrieve 12-bit expression pedal adc threshold, a 12-bit value
FirmwareSupport::Error LumatoneSysEx::unpackGetExpressionPedalThresholdResponse(const juce::MidiMessage& response, int& thresholdValue)
{
    auto status = messageIsValidLumatoneResponse(response);
    if (status != FirmwareSupport::Error::noError)
        return status;

    status = responseIsExpectedLength(response, 3);
    if (status != FirmwareSupport::Error::noError)
        return status;

    auto payload = &response.getSysExData()[PAYLOAD_INIT];

    thresholdValue = (payload[0] << 8) | (payload[1] << 4) | payload[2];

    return status;
}

// For CMD 47h response: retrieve preset flags
FirmwareSupport::Error LumatoneSysEx::unpackGetPresetFlagsResponse(const juce::MidiMessage& response, bool& expressionInverted, bool& lightsOnKeystroke, bool& aftertouchOn, bool& sustainInverted)
{
    const short NUM_UNPACKED = 4;
    int unpackedData[NUM_UNPACKED];
    auto status = unpack7BitData(response, NUM_UNPACKED, unpackedData);
    if (status != FirmwareSupport::Error::noError)
        return status;

    expressionInverted  = unpackedData[0];
    lightsOnKeystroke   = unpackedData[1];
    aftertouchOn        = unpackedData[2];
    sustainInverted     = unpackedData[3];

    return status;
}

// For CMD 48h response: get expression pedal sensitivity
FirmwareSupport::Error LumatoneSysEx::unpackGetExpressionPedalSensitivityResponse(const juce::MidiMessage& response, int& sensitivity)
{
    const short NUM_UNPACKED = 1;
    int unpackedData[NUM_UNPACKED];
    auto status = unpack7BitData(response, NUM_UNPACKED, unpackedData);
    if (status != FirmwareSupport::Error::noError)
        return status;

    sensitivity = unpackedData[0];

    return status;
}

FirmwareSupport::Error LumatoneSysEx::unpackGetMacroLightIntensityResponse(const juce::MidiMessage &response, juce::Colour &activeColour, juce::Colour &inactiveColour)
{
    auto status = isValid(response, 12);
    if (status != FirmwareSupport::Error::noError)
        return status;

    const juce::uint8* payload = &response.getSysExData()[PAYLOAD_INIT];
    activeColour = parseLedIntensity8Bit(payload);
    inactiveColour = parseLedIntensity8Bit(payload + 6); 

    return FirmwareSupport::Error::noError;
}
