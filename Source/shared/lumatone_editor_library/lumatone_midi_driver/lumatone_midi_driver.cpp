/*
  ==============================================================================

    LumatoneFirmwareDriver.cpp
    Created: 20 Feb 2015 8:04:02pm
    Author:  hsstraub

  ==============================================================================
*/

#include "lumatone_midi_driver.h"

// There are different race-condition issues between macOS and Windows. 
// This Driver may need to be redesigned, but for now this define is
// used for including a juce::MessageManagerLock on Windows & Linux, but not on macOS.

#define MIDI_DRIVER_USE_LOCK JUCE_WINDOWS //|| JUCE_LINUX

LumatoneFirmwareDriver::LumatoneFirmwareDriver(int numBoardsIn)
    : numBoards(numBoardsIn)
{
    if (JUCE_STANDALONE_APPLICATION)
        hostMode = HostMode::Driver;
    else
        hostMode = HostMode::Plugin;
}     

LumatoneFirmwareDriver::~LumatoneFirmwareDriver()
{
    collectors.clear();
}

//============================================================================
// TerpstaMidiDriver::Collector helpers

void LumatoneFirmwareDriver::addMessageCollector(Collector* collectorToAdd)
{
    collectors.addIfNotAlreadyThere(collectorToAdd);
}

void LumatoneFirmwareDriver::removeMessageCollector(Collector* collectorToRemove)
{
    collectors.removeFirstMatchingValue(collectorToRemove);
}

void LumatoneFirmwareDriver::readNextBuffer(juce::MidiBuffer &nextBuffer)
{
    juce::ScopedLock l(nextBufferQueue.getLock());

    for (int i = 0; i < nextBufferQueue.size(); i++)
    {
        auto msg = nextBufferQueue[i];
        nextBuffer.addEvent(msg, i);
    }

    nextBufferQueue.clear();
}

void LumatoneFirmwareDriver::sendMessageNow(const juce::MidiMessage &msg)
{
    switch (hostMode)
    {
    case HostMode::Driver:
        HajuMidiDriver::sendMessageNow(msg);
        break;
    case HostMode::Plugin:
    {
        juce::ScopedLock l(nextBufferQueue.getLock());
        nextBufferQueue.add(msg); 
    }
    break;
    }
}

void LumatoneFirmwareDriver::notifyMessageReceived(juce::MidiInput* source, const juce::MidiMessage& midiMessage)
{
#if MIDI_DRIVER_USE_LOCK
    const juce::MessageManagerLock lock;
#endif
    juce::MessageManager::callAsync([this, source, midiMessage]{
        for (auto collector : collectors) collector->midiMessageReceived(source, midiMessage);
    });
}

void LumatoneFirmwareDriver::notifyMessageSent(juce::MidiOutput* target, const juce::MidiMessage& midiMessage)
{
    // Currently unused
// #if MIDI_DRIVER_USE_LOCK
//     const juce::MessageManagerLock lock;
// #endif

//     juce::MessageManager::callAsync([this, target, midiMessage]{
//         for (auto collector : collectors) collector->midiMessageSent(target, midiMessage);
//     });
}

void LumatoneFirmwareDriver::notifySendQueueSize()
{
    auto size = sysexQueue.size();
    for (auto collector : collectors) collector->midiSendQueueSize(size);
}

// void LumatoneFirmwareDriver::notifyLogMessage(juce::String textMessage, ErrorLevel errorLevel)
// {
//     for (auto collector : collectors) collector->generalLogMessage(textMessage, errorLevel);
// }

void LumatoneFirmwareDriver::notifyNoAnswerToMessage(juce::MidiDeviceInfo expectedDevice, const juce::MidiMessage& midiMessage)
{
    juce::MessageManager::callAsync([this, expectedDevice, midiMessage]{
        for (auto collector : collectors) collector->noAnswerToMessage(expectedDevice, midiMessage);
    });
}

/*
==============================================================================
Single (mid-level) commands, firmware specific
*/

// CMD 00h: Send a single key's functionctional configuration
void LumatoneFirmwareDriver::sendKeyFunctionParameters(juce::uint8 boardIndex, juce::uint8 keyIndex, juce::uint8 noteOrCCNum, juce::uint8 midiChannel, juce::uint8 keyType, bool faderUpIsNull)
{
    DBG("SEND KEY FUNCTION REQUESTED " + juce::String(boardIndex) + "," + juce::String(keyIndex));
    // boardIndex is expected 1-based
    jassert(boardIndex > 0 && boardIndex <= numBoards);
    jassert(midiChannel > 0 && midiChannel <= 16);
    jassert(noteOrCCNum >= 0 && noteOrCCNum < 128);

    midiChannel = (midiChannel - 1) & 0xF;
    juce::uint8 typeByte = (faderUpIsNull << 4) | (keyType & 0x3);

    sendSysEx(boardIndex, CHANGE_KEY_NOTE, keyIndex, noteOrCCNum, midiChannel, typeByte);
}

// CMD 01h: Send a single key's LED channel intensities
void LumatoneFirmwareDriver::sendKeyLightParameters(juce::uint8 boardIndex, juce::uint8 keyIndex, juce::uint8 red, juce::uint8 green, juce::uint8 blue)
{
    DBG("SEND KEY COLOUR REQUESTED " + juce::String(boardIndex) + "," + juce::String(keyIndex));

    juce::MidiMessage msg = createExtendedKeyColourSysEx(boardIndex, SET_KEY_COLOUR, keyIndex, red, green, blue);

    sendMessageWithAcknowledge(msg);
}

// CMD 01h: Send a single key's LED channel intensities, three pairs of 4-bit values for each channel
void LumatoneFirmwareDriver::sendKeyLightParameters(juce::uint8 boardIndex, juce::uint8 keyIndex, juce::uint8 redUpper, juce::uint8 redLower, juce::uint8 greenUpper, juce::uint8 greenLower, juce::uint8 blueUpper, juce::uint8 blueLower)
{
    // boardIndex is expected 1-based
    jassert(boardIndex > 0 && boardIndex <= numBoards);

    if (redUpper   > 0xf) redUpper   &= 0xf;
    if (redLower   > 0xf) redLower   &= 0xf;
    if (greenUpper > 0xf) greenUpper &= 0xf;
    if (greenLower > 0xf) greenLower &= 0xf;
    if (blueUpper  > 0xf) blueUpper  &= 0xf;
    if (blueLower  > 0xf) blueLower  &= 0xf;
 
    juce::MidiMessage msg = createExtendedKeyColourSysEx(boardIndex, SET_KEY_COLOUR, keyIndex, redUpper, redLower, greenUpper, greenLower, blueUpper, blueLower);
}

// CMD 01h: Send a single key's LED channel intensities (pre-version 1.0.11)
void LumatoneFirmwareDriver::sendKeyLightParameters_Version_1_0_0(juce::uint8 boardIndex, juce::uint8 keyIndex, juce::uint8 red, juce::uint8 green, juce::uint8 blue)
{
    // boardIndex is expected 1-based
    jassert(boardIndex > 0 && boardIndex <= numBoards);

    // clip if exceed 0x7f
    if (red > 0x7f) red &= 0x7f;
    if (green > 0x7f) green &= 0x7f;
    if (blue > 0x7f) blue &= 0x7f;

    sendSysEx(boardIndex, SET_KEY_COLOUR, keyIndex, red, green, blue);
}

// CMD 02h: Save current configuration to specified preset index
void LumatoneFirmwareDriver::saveProgram(juce::uint8 presetNumber)
{
    jassert(presetNumber >= 0 && presetNumber < 10);
    sendSysEx(0, SAVE_PROGRAM, presetNumber, '\0', '\0', '\0');
}

// CMD 03h: Send expression pedal sensivity
void LumatoneFirmwareDriver::sendExpressionPedalSensivity(juce::uint8 value)
{
	jassert(value <= 0x7f);

	sendSysEx(0, SET_FOOT_CONTROLLER_SENSITIVITY, value, '\0', '\0', '\0');
}

// CMD 04h: Send parametrization of foot controller
void LumatoneFirmwareDriver::sendInvertFootController(bool value)
{
    sendSysExToggle(0, INVERT_FOOT_CONTROLLER, value);
}

// CMD 05h: Colour for macro button in active state, each value should be in range of 0x0-0xF and represents the upper and lower four bytes of each channel intensity
void LumatoneFirmwareDriver::sendMacroButtonActiveColour(juce::uint8 red, juce::uint8 green, juce::uint8 blue)
{
    juce::MidiMessage msg = createExtendedMacroColourSysEx(MACROBUTTON_COLOUR_ON, red, green, blue);
    sendMessageWithAcknowledge(msg);
}

// CMD 05h: Colour for macro button in active state, 3 pairs for 4-bit values for each LED channel
void LumatoneFirmwareDriver::sendMacroButtonActiveColour(juce::uint8 redUpper, juce::uint8 redLower, juce::uint8 greenUpper, juce::uint8 greenLower, juce::uint8 blueUpper, juce::uint8 blueLower)
{
    juce::MidiMessage msg = createExtendedMacroColourSysEx(MACROBUTTON_COLOUR_ON, redUpper, redLower, greenUpper, greenLower, blueUpper, blueLower);
    sendMessageWithAcknowledge(msg);
}

// CMD 05h: Colour for macro button in active state, each value should be in range of 0x00-0x7F (pre-version 1.0.11)
void LumatoneFirmwareDriver::sendMacroButtonActiveColour_Version_1_0_0(juce::uint8 red, juce::uint8 green, juce::uint8 blue)
{
    if (red   > 0x7f) red   &= 0x7f;
    if (green > 0x7f) green &= 0x7f;
    if (blue  > 0x7f) blue  &= 0x7f;

	sendSysEx(0, MACROBUTTON_COLOUR_ON, red, green, blue, '\0');
}

// CMD 06h: Colour for macro button in inactive state, each value should be in range of 0x0-0xF and represents the upper and lower four bytes of each channel intensity
void LumatoneFirmwareDriver::sendMacroButtonInactiveColour(int red, int green, int blue)
{
    juce::MidiMessage msg = createExtendedMacroColourSysEx(MACROBUTTON_COLOUR_OFF, red, green, blue);
    sendMessageWithAcknowledge(msg);
}

// CMD 05h: Colour for macro button in active state, 3 pairs for 4-bit values for each LED channel
void LumatoneFirmwareDriver::sendMacroButtonInactiveColour(juce::uint8 redUpper, juce::uint8 redLower, juce::uint8 greenUpper, juce::uint8 greenLower, juce::uint8 blueUpper, juce::uint8 blueLower)
{
    juce::MidiMessage msg = createExtendedMacroColourSysEx(MACROBUTTON_COLOUR_OFF, redUpper, redLower, greenUpper, greenLower, blueUpper, blueLower);
    sendMessageWithAcknowledge(msg);
}

// CMD 06h: Colour for macro button in inactive state, each value should be in range of 0x00-0x7F (pre-version 1.0.11)
void LumatoneFirmwareDriver::sendMacroButtonInactiveColour_Version_1_0_0(juce::uint8 red, juce::uint8 green, juce::uint8 blue)
{
    if (red   > 0x7f) red   &= 0x7f;
    if (green > 0x7f) green &= 0x7f;
    if (blue  > 0x7f) blue  &= 0x7f;

	sendSysEx(0, MACROBUTTON_COLOUR_OFF, red, green, blue, '\0');
}

// CMD 07h: Send parametrization of light on keystrokes
void LumatoneFirmwareDriver::sendLightOnKeyStrokes(bool value)
{
    sendSysExToggle(0, SET_LIGHT_ON_KEYSTROKES, value);
}

// CMD 08h: Send a value for a velocity lookup table
void LumatoneFirmwareDriver::sendVelocityConfig(const juce::uint8 velocityTable[])
{
    // Values are in reverse order (shortest ticks count is the highest velocity)
    juce::uint8 reversedTable[128];
    for (juce::uint8 x = 0; x < 128; x++)
    {
        reversedTable[x] = velocityTable[127 - x] & 0x7f;
    }

    auto msg = createTableSysEx(0, SET_VELOCITY_CONFIG, 128, reversedTable);
    sendMessageWithAcknowledge(msg);
}

// CMD 09h: Save velocity config to EEPROM
void LumatoneFirmwareDriver::saveVelocityConfig()
{            
    sendSysExRequest(0, SAVE_VELOCITY_CONFIG);
}

// CMD 0Ah: Reset velocity config to value from EEPROM
void LumatoneFirmwareDriver::resetVelocityConfig()
{
    sendSysExRequest(0, RESET_VELOCITY_CONFIG);
}

// CMD 0Bh: Adjust the internal fader look-up table (128 7-bit values)
void LumatoneFirmwareDriver::sendFaderConfig(const juce::uint8 faderTable[])
{
    juce::MidiMessage msg = createTableSysEx(0, SET_FADER_CONFIG, 128, faderTable);
    sendMessageWithAcknowledge(msg);
}

// CMD 0Ch: **DEPRECATED** Save the changes made to the fader look-up table
void LumatoneFirmwareDriver::saveFaderConfiguration()
{
    sendSysExRequest(0, SAVE_FADER_CONFIG);
}

// CMD 0Dh: Reset the fader lookup table back to its factory fader settings.
void LumatoneFirmwareDriver::resetFaderConfig()
{
    sendSysExRequest(0, RESET_FADER_CONFIG);
}

// CMD 0Eh: Enable or disable aftertouch functionality
void LumatoneFirmwareDriver::sendAfterTouchActivation(bool value)
{
    sendSysExToggle(0, SET_AFTERTOUCH_FLAG, value);
}

// CMD 0Fh: Initiate aftertouch calibration routine
void LumatoneFirmwareDriver::sendCalibrateAfterTouch()
{
    sendSysExRequest(0, CALIBRATE_AFTERTOUCH);
}

// CMD 10h: Adjust the internal aftertouch look-up table (size of 128)
void LumatoneFirmwareDriver::sendAftertouchConfig(const juce::uint8 aftertouchTable[])
{
    juce::MidiMessage msg = createTableSysEx(0, SET_AFTERTOUCH_CONFIG, 128, aftertouchTable);
    sendMessageWithAcknowledge(msg);
}

// CMD 11h: **DEPRECATED** Save the changes made to the aftertouch look-up table
void LumatoneFirmwareDriver::saveAftertouchConfig()
{
    sendSysExRequest(0, SAVE_AFTERTOUCH_CONFIG);
}

// CMD 12h: Reset the aftertouch lookup table back to its factory aftertouch settings.
void LumatoneFirmwareDriver::resetAftertouchConfig()
{
    sendSysExRequest(0, RESET_AFTERTOUCH_CONFIG);
}

// CMD 13h: Read back the current red intensity of all the keys of the target board.
void LumatoneFirmwareDriver::sendRedLEDConfigRequest(juce::uint8 boardIndex)
{
    sendSysExRequest(boardIndex, GET_RED_LED_CONFIG);
}

// CMD 14h: Read back the current green intensity of all the keys of the target board.
void LumatoneFirmwareDriver::sendGreenLEDConfigRequest(juce::uint8 boardIndex)
{
    sendSysExRequest(boardIndex, GET_GREEN_LED_CONFIG);
}

// CMD 15h: Read back the current blue intensity of all the keys of the target board.
void LumatoneFirmwareDriver::sendBlueLEDConfigRequest(juce::uint8 boardIndex)
{
    sendSysExRequest(boardIndex, GET_BLUE_LED_CONFIG);
}

// CMD 16h: Read back the current channel configuration of all the keys of the target board.
void LumatoneFirmwareDriver::sendChannelConfigRequest(juce::uint8 boardIndex)
{
    sendSysExRequest(boardIndex, GET_CHANNEL_CONFIG);
}

// CMD 17h: Read back the current note configuration of all the keys of the target board.
void LumatoneFirmwareDriver::sendNoteConfigRequest(juce::uint8 boardIndex)
{
    sendSysExRequest(boardIndex, GET_NOTE_CONFIG);
}

// CMD 18h: Read back the current key type configuration of all the keys of the target board.
void LumatoneFirmwareDriver::sendKeyTypeConfigRequest(juce::uint8 boardIndex)
{
    sendSysExRequest(boardIndex, GET_KEYTYPE_CONFIG);
}

// CMD 19h: Read back the maximum threshold of all the keys of the target board.
void LumatoneFirmwareDriver::sendMaxFaderThresholdRequest(juce::uint8 boardIndex)
{
    sendSysExRequest(0, GET_MAX_THRESHOLD);
}

// CMD 1Ah: Read back the maximum threshold of all the keys of the target board
void LumatoneFirmwareDriver::sendMinFaderThresholdRequest(juce::uint8 boardIndex)
{
    sendSysExRequest(0, GET_MIN_THRESHOLD);
}

// CMD 1Bh: Read back the aftertouch maximum threshold of all the keys of the target board
void LumatoneFirmwareDriver::sendMaxAftertouchThresholdRequest(juce::uint8 boardIndex)
{
    sendSysExRequest(0, GET_AFTERTOUCH_MAX);
}

// CMD 1Ch: Get back flag whether or not each key of target board meets minimum threshold
void LumatoneFirmwareDriver::sendKeyValidityParametersRequest(juce::uint8 boardIndex)
{
    sendSysExRequest(boardIndex, GET_KEY_VALIDITY);
}

// CMD 1Dh: Read back the current velocity look up table of the keyboard.
void LumatoneFirmwareDriver::sendVelocityConfigRequest()
{
    sendSysExRequest(0, GET_VELOCITY_CONFIG);
}

// CMD 1Eh: Read back the current fader look up table of the keyboard.
void LumatoneFirmwareDriver::sendFaderConfigRequest()
{
    sendSysExRequest(0, GET_FADER_CONFIG);
}

// CMD 1Fh: Read back the current aftertouch look up table of the keyboard.
void LumatoneFirmwareDriver::sendAftertouchConfigRequest()
{
    sendSysExRequest(0, GET_AFTERTOUCH_CONFIG);
}

// CMD 20h: Set the velocity interval table, 127 12-bit values
void LumatoneFirmwareDriver::sendVelocityIntervalConfig(const int velocityIntervalTable[])
{
    const int payloadSize = 254;
    juce::uint8 formattedTable[payloadSize];

    // Interval table contains 127 values!
	for (juce::uint8 i = 0; i < VELOCITYINTERVALTABLESIZE; i++)
    {
        formattedTable[2*i]     = (velocityIntervalTable[i] >> 6) & 0x3f;
        formattedTable[1 + 2*i] = velocityIntervalTable[i] & 0x3f;
    }

	juce::MidiMessage msg = createTableSysEx(0, SET_VELOCITY_INTERVALS, payloadSize, formattedTable);
	sendMessageWithAcknowledge(msg);
}

// CMD 21h: Sead back the velocity interval table
void LumatoneFirmwareDriver::sendVelocityIntervalConfigRequest()
{
    sendSysExRequest(0, GET_VELOCITY_INTERVALS);
}

// CMD 22h: Read back the fader type of all keys on the targeted board.
void LumatoneFirmwareDriver::sendFaderTypeConfigRequest(juce::uint8 boardIndex)
{
    sendSysExRequest(boardIndex, GET_FADER_TYPE_CONFIGURATION);
}

// CMD 23h: This command is used to read back the serial identification number of the keyboard.
void LumatoneFirmwareDriver::sendGetSerialIdentityRequest(int sendToTestDevice)
{
    if (sendToTestDevice < 0)
        sendSysEx(0, GET_SERIAL_IDENTITY, TEST_ECHO, '\0', '\0', '\0');
    else
        sendTestMessageNow(sendToTestDevice, createTerpstraSysEx(0, GET_SERIAL_IDENTITY, TEST_ECHO, '\0', '\0', '\0'));
}

// CMD 24h: Initiate the key calibration routine; each pair of macro buttons  
// on each octave must be pressed to return to normal state
void LumatoneFirmwareDriver::sendCalibrateKeys()
{
    sendSysExRequest(0, CALIBRATE_KEYS);
}

// CMD 25h: Pass in true to enter Demo Mode, or false to exit
void LumatoneFirmwareDriver::startDemoMode(bool turnOn)
{
    sendSysExToggle(0, DEMO_MODE, turnOn);
}

// CMD 26h: Initiate the pitch and mod wheel calibration routine, pass in false to stop
void LumatoneFirmwareDriver::sendCalibratePitchModWheel(bool startCalibration, int testOutputIndex)
{
    if (testOutputIndex < 0)
        sendSysExToggle(0, CALIBRATE_PITCH_MOD_WHEEL, startCalibration);
    else
        sendTestMessageNow(testOutputIndex, createTerpstraSysEx(0, CALIBRATE_PITCH_MOD_WHEEL, startCalibration, TEST_ECHO, '\0', '\0'));
}

// CMD 27h: Set the sensitivity value of the mod wheel, 0x01 to 0x07f
void LumatoneFirmwareDriver::setModWheelSensitivity(juce::uint8 sensitivity)
{
    if (sensitivity > 0x7f) sensitivity &= 0x7f;  // Restrict to upper bound
    if (sensitivity < 0x01) sensitivity  = 0x01;  // Restrict to lower bound

    sendSysEx(0, SET_MOD_WHEEL_SENSITIVITY, sensitivity, '\0', '\0', '\0');
}

// CMD 28h: Set the sensitivity value of the pitch bend wheel between 0x01 and 0x3FFF
void LumatoneFirmwareDriver::setPitchBendSensitivity(int sensitivity)
{
    if (sensitivity > 0x3fff) sensitivity &= 0x3fff; // Restrict to upper bound
    if (sensitivity < 0x0001) sensitivity  = 0x0001;  // Restrict to lower bound

    sendSysEx(0, SET_PITCH_WHEEL_SENSITIVITY, sensitivity >> 7, sensitivity & 0x7f, '\0',  '\0');
}

// CMD 29h: Set abs. distance from max value to trigger CA-004 submodule key events, ranging from 0x00 to 0xFE
void LumatoneFirmwareDriver::setKeyMaximumThreshold(juce::uint8 boardIndex, juce::uint8 maxThreshold, juce::uint8 aftertouchMax)
{
    if (maxThreshold  > 0xfe) maxThreshold  &= 0xfe;
    if (aftertouchMax > 0xfe) aftertouchMax &= 0xfe;
    sendSysEx(boardIndex, SET_KEY_MAX_THRESHOLD, maxThreshold >> 4, maxThreshold & 0xf, aftertouchMax >> 4, aftertouchMax & 0xf);
}

// CMD 2Ah: Set abs. distance from min value to trigger CA-004 submodule key events, ranging from 0x00 to 0xFE
void LumatoneFirmwareDriver::setKeyMinimumThreshold(juce::uint8 boardIndex, juce::uint8 minThresholdHigh, juce::uint8 minThresholdLow)
{
    if (minThresholdHigh > 0xfe) minThresholdHigh &= 0xfe;
    if (minThresholdLow  > 0xfe) minThresholdLow  &= 0xfe;
    sendSysEx(boardIndex, SET_KEY_MIN_THRESHOLD, minThresholdHigh >> 4, minThresholdHigh & 0xf, minThresholdLow >> 4, minThresholdLow & 0xf);
}

// CMD 2Bh: Set the sensitivity for CC events, ranging from 0x00 to 0xFE
void LumatoneFirmwareDriver::setFaderKeySensitivity(juce::uint8 boardIndex, juce::uint8 sensitivity)
{
    if (sensitivity > 0xfe) sensitivity &= 0xfe;
    sendSysEx(boardIndex, SET_KEY_FADER_SENSITIVITY, sensitivity >> 4, sensitivity & 0xf, '\0', '\0');
}

// CMD 2Ch: Set the target board sensitivity for aftertouch events, ranging from 0x00 to 0xFE
void LumatoneFirmwareDriver::setAftertouchKeySensitivity(juce::uint8 boardIndex, juce::uint8 sensitivity)
{
    if (sensitivity > 0xfe) sensitivity &= 0xfe;
    sendSysEx(boardIndex, SET_KEY_AFTERTOUCH_SENSITIVITY, sensitivity >> 4, sensitivity & 0xf, '\0', '\0');
}

// CMD 2Dh: Adjust the Lumatouch table, a 128 byte array with value of 127 being a key fully pressed
void LumatoneFirmwareDriver::setLumatouchConfig(const juce::uint8 lumatouchTable[])
{
    juce::MidiMessage msg = createTableSysEx(0, SET_LUMATOUCH_CONFIG, 128, lumatouchTable);
    sendMessageWithAcknowledge(msg);
}

// CMD 2Eh: **DEPRECATED** Save Lumatouch table changes
void LumatoneFirmwareDriver::saveLumatoneConfig()
{
    sendSysExRequest(0, SAVE_LUMATOUCH_CONFIG);
}

// CMD 2Fh: Reset the Lumatouch table back to factory settings
void LumatoneFirmwareDriver::resetLumatouchConfig()
{
    sendSysExRequest(0, RESET_LUMATOUCH_CONFIG);
}

// CMD 30h: Read back the Lumatouch table
void LumatoneFirmwareDriver::sendLumatouchConfigRequest()
{
    sendSysExRequest(0, GET_LUMATOUCH_CONFIG);
}

// CMD 31h: This command is used to read back the current Lumatone firmware revision.
void LumatoneFirmwareDriver::sendGetFirmwareRevisionRequest(int sendToTestDevice)
{
    if (sendToTestDevice < 0)
        sendSysExRequest(0, GET_FIRMWARE_REVISION);
    else
        sendTestMessageNow(sendToTestDevice, createTerpstraSysEx(0, GET_FIRMWARE_REVISION, TEST_ECHO, '\0', '\0', '\0'));
}

// CMD 32h: Set the thresold from key's min value to trigger CA - 004 submodule CC events, ranging from 0x00 to 0xFE
void LumatoneFirmwareDriver::setCCActiveThreshold(juce::uint8 boardIndex, juce::uint8 sensitivity)
{
    if (sensitivity > 0xfe) sensitivity &= 0xfe;
    sendSysEx(boardIndex, SET_CC_ACTIVE_THRESHOLD, sensitivity >> 4, sensitivity & 0xf, '\0', '\0');
}

// CMD 33h: Echo the payload, 0x00-0x7f, for use in connection monitoring
// the first 7-bit value is reserved for echo differentiation
void LumatoneFirmwareDriver::ping(juce::uint8 value1, juce::uint8 value2, juce::uint8 value3, int sendToTestDevice)
{
    if (value1 > 0x7f) value1 &= 0x7f;
    if (value2 > 0x7f) value2 &= 0x7f;
    if (value3 > 0x7f) value3 &= 0x7f;

    if (sendToTestDevice < 0)
        sendSysEx(0, LUMA_PING, TEST_ECHO, value1, value2, value3);
    else
    {
        sendTestMessageNow(sendToTestDevice, createTerpstraSysEx(0, LUMA_PING, TEST_ECHO, value1, value2, value3));
    }
}

// CMD 33h: Echo the payload, 0x00-0x7f, for use in connection monitoring
unsigned int LumatoneFirmwareDriver::ping(unsigned int value, int sendToTestDevice)
{
    value &= 0xFFFFFFF; // Limit 28-bits
    ping((value >> 14) & 0x7f, (value >> 7) & 0x7f, value & 0x7f, sendToTestDevice);
    return value;
}

// CMD 34h: Reset the thresholds for events and sensitivity for CC & aftertouch on the target board
void LumatoneFirmwareDriver::resetBoardThresholds(juce::uint8 boardIndex)
{
    sendSysExRequest(boardIndex, RESET_BOARD_THRESHOLDS);
}

// CMD 35h: Enable/disable key sampling over SSH for the target key and board
void LumatoneFirmwareDriver::setKeySampling(juce::uint8 boardIndex, juce::uint8 keyIndex, bool turnSamplingOn)
{
    sendSysExToggle(boardIndex, SET_KEY_SAMPLING, turnSamplingOn);
}

// CMD 36h: Set thresholds for the pitch and modulation wheel to factory settings
void LumatoneFirmwareDriver::resetWheelsThresholds()
{
    sendSysExRequest(0, RESET_WHEELS_THRESHOLD);
}

// CMD 37h: Set the bounds from the calibrated zero adc value of the pitch wheel, 0x00 to 0x7f
void LumatoneFirmwareDriver::setPitchWheelZeroThreshold(juce::uint8 threshold)
{
    if (threshold > 0x7f) threshold &= 0x7f;
    sendSysEx(0, SET_PITCH_WHEEL_CENTER_THRESHOLD, threshold, '\0', '\0', '\0');
}

// CMD 38h: Pass in true to initiate the expression pedal calibration routine, or false to stop
void LumatoneFirmwareDriver::calibrateExpressionPedal(bool startCalibration)
{
    sendSysExToggle(0, CALLIBRATE_EXPRESSION_PEDAL, startCalibration);
}

// CMD 39h: Reset expression pedal minimum and maximum bounds to factory settings
void LumatoneFirmwareDriver::resetExpressionPedalBounds()
{
    sendSysExRequest(0, RESET_EXPRESSION_PEDAL_BOUNDS);
}

// CMD 3Ah: Retrieve the threshold values of target board
void LumatoneFirmwareDriver::getBoardThresholdValues(juce::uint8 boardIndex)
{
    sendSysExRequest(boardIndex, GET_BOARD_THRESHOLD_VALUES);
}

// CMD 3Bh: Retrieve the sensitivity values of target board
void LumatoneFirmwareDriver::getBoardSensitivityValues(juce::uint8 boardIndex)
{
    sendSysExRequest(boardIndex, GET_BOARD_SENSITIVITY_VALUES);
}

// CMD 3Ch: Set the MIDI channels for peripheral controllers
void LumatoneFirmwareDriver::setPeripheralChannels(juce::uint8 pitchWheelChannel, juce::uint8 modWheelChannel, juce::uint8 expressionChannel, juce::uint8 sustainChannel)
{
    pitchWheelChannel -= 1;
    modWheelChannel -= 1;
    expressionChannel -= 1;
    sustainChannel -= 1;

    // For now set default channel to 1
    if (pitchWheelChannel > 0xf) pitchWheelChannel &= 0x00;
    if (modWheelChannel   > 0xf) modWheelChannel   &= 0x00;
    if (expressionChannel > 0xf) expressionChannel &= 0x00;
    if (sustainChannel    > 0xf) sustainChannel    &= 0x00;

    sendSysEx(0, SET_PERIPHERAL_CHANNELS, pitchWheelChannel, modWheelChannel, expressionChannel, sustainChannel);
}

// CMD 3Dh: Retrieve the MIDI channels for peripheral controllers
void LumatoneFirmwareDriver::getPeripheralChannels()
{
    sendSysExRequest(0, GET_PERIPHERAL_CHANNELS);
}

// CMD 3Fh: Set the 8-bit aftertouch trigger delay value, the time between a note on event and the initialization of aftertouch events
void LumatoneFirmwareDriver::setAfterTouchTriggerDelay(juce::uint8 boardIndex, juce::uint8 aftertouchTriggerValue)
{
    setAfterTouchTriggerDelay(boardIndex, aftertouchTriggerValue >> 4, aftertouchTriggerValue & 0xf);
}

// CMD 3Fh: Set the 8-bit aftertouch trigger delay value, the time between a note on event and the initialization of aftertouch events
void LumatoneFirmwareDriver::setAfterTouchTriggerDelay(juce::uint8 boardIndex, juce::uint8 triggerValueUpperNibble, juce::uint8 triggerValueLowerNibble)
{
    sendSysEx(boardIndex, SET_AFTERTOUCH_TRIGGER_DELAY, triggerValueUpperNibble, triggerValueLowerNibble, '\0', '\0');
}

// CMD 40h: Retrieve the aftertouch trigger delay of the given board
void LumatoneFirmwareDriver::sendGetAftertouchTriggerDelayRequest(juce::uint8 boardIndex)
{
    sendSysExRequest(boardIndex, GET_AFTERTOUCH_TRIGGER_DELAY);
}

// CMD 41h: Set the Lumatouch note-off delay value, an 11-bit integer representing the amount of 1.1ms ticks before
// sending a note-off event after a Lumatone-configured key is released. 
void LumatoneFirmwareDriver::setLumatouchNoteOffDelay(juce::uint8 boardIndex, int delayValue)
{
    setLumatouchNoteOffDelay(boardIndex,
        (delayValue >> 8) & 0xf,
        (delayValue >> 4) & 0xf,
        delayValue & 0xf
    );
}

void LumatoneFirmwareDriver::setLumatouchNoteOffDelay(juce::uint8 boardIndex, juce::uint8 valueBits8_11, juce::uint8 valueBits4_7, juce::uint8 valueBits0_3)
{
    sendSysEx(boardIndex, SET_LUMATOUCH_NOTE_OFF_DELAY, valueBits8_11, valueBits4_7, valueBits0_3, '\0');
}

// CMD 42h: Retrieve the note-off delay value of the given board
void LumatoneFirmwareDriver::sendGetLumatouchNoteOffDelayRequest(juce::uint8 boardIndex)
{
    sendSysExRequest(boardIndex, GET_LUMATOUCH_NOTE_OFF_DELAY);
}

// CMD 43h: Set expression pedal ADC threshold value, a 12-bit integer
void LumatoneFirmwareDriver::setExpressionPedalADCThreshold(int thresholdValue)
{
    setExpressionPedalADCThreshold(
        (thresholdValue >> 8) & 0xf,
        (thresholdValue >> 4) & 0xf,
        thresholdValue & 0xf
    );
}

// CMD 43h: Set expression pedal ADC threshold value with 3 4-bit integers representing a 12-bit integer
void LumatoneFirmwareDriver::setExpressionPedalADCThreshold(juce::uint8 valueBits8_12, juce::uint8 valueBits4_7, juce::uint8 valueBits0_3)
{
    sendSysEx(0, SET_EXPRESSION_PEDAL_THRESHOLD, valueBits8_12, valueBits4_7, valueBits0_3, '\0');
}

// CMD 44h: Get the current expression pedal ADC threshold value
void LumatoneFirmwareDriver::sendGetExpressionPedalADCThresholdRequest()
{
    sendSysExRequest(0, GET_EXPRESSION_PEDAL_THRESHOLD);
}

// CMD 45h: Configure the on/off settings of the sustain pedal
void LumatoneFirmwareDriver::sendInvertSustainPedal(bool setInverted)
{
    sendSysExToggle(0, INVERT_SUSTAIN_PEDAL, setInverted);
}

// CMD 46h: Replace current presets with factory presets
void LumatoneFirmwareDriver::sendResetDefaultPresetsRequest(int presetIndex)
{
    presetIndex %= 10;
    sendSysEx(0, RESET_DEFAULT_PRESETS, presetIndex, '\0', '\0', '\0');
}

// CMD 47h: Read back the currently configured preset flags of expression & sustain inversion,
// plus light-on-keystroke and polyphonic aftertouch
void LumatoneFirmwareDriver::sendGetPresetFlagsReset()
{
    sendSysExRequest(0, GET_PRESET_FLAGS);
}

// For CMD 48h response: get expression pedal sensitivity
void LumatoneFirmwareDriver::sendGetExpressionPedalSensitivity()
{
    sendSysExRequest(0, GET_EXPRESSION_PEDAL_SENSITIVIY);
}

/*
==============================================================================
Low-level SysEx calls
*/

void LumatoneFirmwareDriver::fillManufacturerId(unsigned char* data) const
{
    data[0] = MANUFACTURER_ID_0;
    data[1] = MANUFACTURER_ID_1;
    data[2] = MANUFACTURER_ID_2;
}

juce::MidiMessage LumatoneFirmwareDriver::createTerpstraSysEx(juce::uint8 boardIndex, juce::uint8 cmd, juce::uint8 data1, juce::uint8 data2, juce::uint8 data3, juce::uint8 data4) const
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
juce::MidiMessage LumatoneFirmwareDriver::createExtendedKeyColourSysEx(juce::uint8 boardIndex, juce::uint8 cmd, juce::uint8 keyIndex, juce::uint8 redUpper, juce::uint8 redLower, juce::uint8 greenUpper, juce::uint8 greenLower, juce::uint8 blueUpper, juce::uint8 blueLower) const
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
juce::MidiMessage LumatoneFirmwareDriver::createExtendedKeyColourSysEx(juce::uint8 boardIndex, juce::uint8 cmd, juce::uint8 keyIndex, int red, int green, int blue) const
{
    return createExtendedKeyColourSysEx(boardIndex, cmd, keyIndex, red >> 4, red & 0xf, green >> 4, green & 0xf, blue >> 4, blue & 0xf);
}

juce::MidiMessage LumatoneFirmwareDriver::createExtendedMacroColourSysEx(juce::uint8 cmd, juce::uint8 redUpper, juce::uint8 redLower, juce::uint8 greenUpper, juce::uint8 greenLower, juce::uint8 blueUpper, juce::uint8 blueLower) const
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

juce::MidiMessage LumatoneFirmwareDriver::createExtendedMacroColourSysEx(juce::uint8 cmd, int red, int green, int blue) const
{
    return createExtendedMacroColourSysEx(cmd, red >> 4, red & 0xf, green >> 4, green & 0xf, blue >> 4, blue & 0xf);
}

juce::MidiMessage LumatoneFirmwareDriver::createTableSysEx(juce::uint8 boardIndex, juce::uint8 cmd, juce::uint8 tableSize, const juce::uint8 table[])
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

void LumatoneFirmwareDriver::sendSysEx(juce::uint8 boardIndex, juce::uint8 cmd, juce::uint8 data1, juce::uint8 data2, juce::uint8 data3, juce::uint8 data4, bool overrideEditMode)
{
    switch (hostMode)
    {
    case HostMode::Driver:
        if (getMidiInputIndex() < 0)
        {
            return;
        }
        break;
    default:
        break;
    }

    jassert(boardIndex < 0x6 && data1 <= 0x7f && data2 <= 0x7f && data3 <= 0x7f && data4 <= 0x7f);
    juce::MidiMessage msg = createTerpstraSysEx(boardIndex, cmd, data1, data2, data3, data4);
    sendMessageWithAcknowledge(msg);
}

// Send a SysEx message without parameters
void LumatoneFirmwareDriver::sendSysExRequest(juce::uint8 boardIndex, juce::uint8 cmd)
{
    unsigned char sysExData[9];
    fillManufacturerId(sysExData);
    sysExData[3] = boardIndex;
    sysExData[4] = cmd;
    sysExData[5] = '\0';
    sysExData[6] = '\0';
    sysExData[7] = '\0';
    sysExData[8] = '\0';
    auto msg = juce::MidiMessage::createSysExMessage(sysExData, 9);
    sendMessageWithAcknowledge(msg);
}

void LumatoneFirmwareDriver::sendSysExToggle(juce::uint8 boardIndex, juce::uint8 cmd, bool turnStateOn)
{
    unsigned char sysExData[9];
    fillManufacturerId(sysExData);
    sysExData[3] = boardIndex;
    sysExData[4] = cmd;
    sysExData[5] = turnStateOn;
    sysExData[6] = '\0';
    sysExData[7] = '\0';
    sysExData[8] = '\0';
    auto msg = juce::MidiMessage::createSysExMessage(sysExData, 9);
    sendMessageWithAcknowledge(msg);
}

// Checks if message is a valid Lumatone firmware response and is expected length, then runs supplied unpacking function or returns an error code 
FirmwareSupport::Error LumatoneFirmwareDriver::unpackIfValid(const juce::MidiMessage& response, size_t numBytes, std::function<FirmwareSupport::Error(const juce::uint8*)> unpackFunction)
{
    auto status = messageIsValidLumatoneResponse(response);
    if (status != FirmwareSupport::Error::noError)
        return status;

    status = responseIsExpectedLength(response, numBytes);
    if (status != FirmwareSupport::Error::noError)
        return status;

    return unpackFunction(&response.getSysExData()[PAYLOAD_INIT]);
}

// Generic unpacking of octave data from a SysEx message
FirmwareSupport::Error LumatoneFirmwareDriver::unpackOctaveConfig(const juce::MidiMessage& msg, int& boardId, size_t numBytes, int* keyData, std::function<FirmwareSupport::Error(const juce::MidiMessage&, size_t, int*)> nBitUnpackFunction)
{
    auto status = nBitUnpackFunction(msg, numBytes, keyData);
    if (status != FirmwareSupport::Error::noError)
        return status;

    boardId = msg.getSysExData()[BOARD_IND];
    return status;
}

// Generic unpacking of 7-bit data from a SysEx message
FirmwareSupport::Error LumatoneFirmwareDriver::unpack7BitData(const juce::MidiMessage& msg, size_t numBytes, int* unpackedData)
{
    auto unpack = [&](const juce::uint8* payload) {
        for (int i = 0; i < numBytes; i++)
            unpackedData[i] = payload[i];
        return FirmwareSupport::Error::noError;
    };

    return unpackIfValid(msg, numBytes, unpack);
}

// Unpacking of octave-based 7-bit key configuration data
FirmwareSupport::Error LumatoneFirmwareDriver::unpack7BitOctaveData(const juce::MidiMessage& msg, int& boardId, juce::uint8 numKeys, int* keyData)
{
    return unpackOctaveConfig(msg, boardId, numKeys, keyData, [&](const juce::MidiMessage&, juce::uint8, int*) {
        return unpack7BitData(msg, numKeys, keyData);
    });
}

// Generic unpacking of 8-bit data from a SysEx message
FirmwareSupport::Error LumatoneFirmwareDriver::unpack8BitData(const juce::MidiMessage& msg, size_t numBytes, int* unpackedData)
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
FirmwareSupport::Error LumatoneFirmwareDriver::unpack8BitOctaveData(const juce::MidiMessage& msg, int& boardId, juce::uint8 numKeys, int* keyData)
{
    return unpackOctaveConfig(msg, boardId, numKeys, keyData, [&](const juce::MidiMessage&, size_t, int*) {
        return unpack8BitData(msg, numKeys * 2, keyData);
    });
}

// Generic unpacking of 12-bit data from a SysEx message, when packed with two 7-bit values
FirmwareSupport::Error LumatoneFirmwareDriver::unpack12BitDataFrom7Bit(const juce::MidiMessage& msg, size_t numBytes, int* unpackedData)
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
FirmwareSupport::Error LumatoneFirmwareDriver::unpack12BitDataFrom4Bit(const juce::MidiMessage& msg, size_t numBytes, int* unpackedData)
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
bool LumatoneFirmwareDriver::messageIsResponseToMessage(const juce::MidiMessage& answer, const juce::MidiMessage& originalMessage)
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

FirmwareSupport::Error LumatoneFirmwareDriver::messageIsValidLumatoneResponse(const juce::MidiMessage& midiMessage)
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

    if (sysExData[MSG_STATUS] > TerpstraMIDIAnswerReturnCode::ERROR)
        return FirmwareSupport::Error::messageHasInvalidStatusByte;

    return FirmwareSupport::Error::noError;
}

FirmwareSupport::Error LumatoneFirmwareDriver::responseIsExpectedLength(const juce::MidiMessage& midiMessage, size_t numPayloadBytes)
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
FirmwareSupport::Error LumatoneFirmwareDriver::unpackGetLEDConfigResponse(const juce::MidiMessage& response, int& boardId, int* keyData)
{
    // TODO: Maybe should define keys per octave somewhere
    return unpack8BitOctaveData(response, boardId, 56, keyData);
}

// For CMD 13h response: unpacks key data for red LED intensity. 55 or 56 bytes, each value must be multiplied by 5
FirmwareSupport::Error LumatoneFirmwareDriver::unpackGetLEDConfigResponse_Version_1_0_0(const juce::MidiMessage& response, int& boardId, juce::uint8 numKeys, int* keyData)
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
FirmwareSupport::Error LumatoneFirmwareDriver::unpackGetChannelConfigResponse(const juce::MidiMessage& response, int& boardId, juce::uint8 numKeys, int* keyData)
{
    return unpack7BitOctaveData(response, boardId, numKeys, keyData);
}

// For CMD 17h response: unpacks 7-bit key data for note configuration. 55 or 56 bytes
FirmwareSupport::Error LumatoneFirmwareDriver::unpackGetNoteConfigResponse(const juce::MidiMessage& response, int& boardId, juce::uint8 numKeys, int* keyData)
{
    return unpack7BitOctaveData(response, boardId, numKeys, keyData);
}

// For CMD 18h response: unpacks 7-bit key type data for key configuration. 55 or 56 bytes
FirmwareSupport::Error LumatoneFirmwareDriver::unpackGetTypeConfigResponse(const juce::MidiMessage& response, int& boardId, juce::uint8 numKeys, int* keyData)
{
    return unpack7BitOctaveData(response, boardId, numKeys, keyData);
}

// For CMD 19h response: unpacks 8-bit key data for maximums of adc threshold. 55 or 56 bytes
FirmwareSupport::Error LumatoneFirmwareDriver::unpackGetKeyMaxThresholdsResponse(const juce::MidiMessage& response, int& boardId, juce::uint8 numKeys, int* keyData)
{
    return unpack8BitData(response, numKeys, keyData);
}

// For CMD 1Ah response: unpacks 8-bit key data for minimums of adc threshold. 55 or 56 bytes
FirmwareSupport::Error LumatoneFirmwareDriver::unpackGetKeyMinThresholdsResponse(const juce::MidiMessage& response, int& boardId, juce::uint8 numKeys, int* keyData)
{
    return unpack8BitOctaveData(response, boardId, numKeys, keyData);
}

// For CMD 1Bh response: unpacks 8-bit key data for maximums of adc threshold for aftertouch triggering. 55 or 56 bytes
FirmwareSupport::Error LumatoneFirmwareDriver::unpackGetAftertouchMaxThresholdsResponse(const juce::MidiMessage& response, int& boardId, juce::uint8 numKeys, int* keyData)
{
    return unpack8BitOctaveData(response, boardId, numKeys, keyData);
}

// For CMD 1Ch response: unpacks boolean key validity data for board, whether or not each key meets threshold specs
FirmwareSupport::Error LumatoneFirmwareDriver::unpackGetKeyValidityResponse(const juce::MidiMessage& response, int& boardId, juce::uint8 numKeys, bool* keyValidityData)
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
FirmwareSupport::Error LumatoneFirmwareDriver::unpackGetVelocityConfigResponse(const juce::MidiMessage& response, int* velocityData)
{
    return unpack7BitData(response, 128, velocityData);
}

// For CMD 1Eh response: unpacks 7-bit fader configuration of keyboard, 128 bytes
FirmwareSupport::Error LumatoneFirmwareDriver::unpackGetFaderConfigResponse(const juce::MidiMessage& response, int* faderData)
{
    return unpack7BitData(response, 128, faderData);
}

// For CMD 1Fh response: unpacks 7-bit aftertouch configuration of keyboard, 128 bytes
FirmwareSupport::Error LumatoneFirmwareDriver::unpackGetAftertouchConfigResponse(const juce::MidiMessage& response, int* aftertouchData)
{
    return unpack7BitData(response, 128, aftertouchData);
}

// For CMD 21h response: unpacks 12-bit velocity interval configuration of keyboard, 254 bytes encoding 127 values
FirmwareSupport::Error LumatoneFirmwareDriver::unpackGetVelocityIntervalConfigResponse(const juce::MidiMessage& response, int* intervalData)
{
    return unpack12BitDataFrom7Bit(response, 254, intervalData);
}

// For CMD 22h response: unpacks 7-bit fader type configuration of board, 56 bytes
FirmwareSupport::Error LumatoneFirmwareDriver::unpackGetFaderConfigResponse(const juce::MidiMessage& response, int& boardId, juce::uint8 numKeys, int* faderData)
{
    return unpackOctaveConfig(response, boardId, numKeys, faderData, [&](const juce::MidiMessage&, size_t, int*) {
        return unpack7BitData(response, numKeys, faderData);
    });
}

// For CMD 23h response: unpacks serial ID number of keyboard, 12 7-bit values encoding 6 bytes
FirmwareSupport::Error LumatoneFirmwareDriver::unpackGetSerialIdentityResponse(const juce::MidiMessage& response, int* serialBytes)
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
FirmwareSupport::Error LumatoneFirmwareDriver::unpackGetLumatouchConfigResponse(const juce::MidiMessage& response, int* lumatouchData)
{
    return unpack7BitData(response, 128, lumatouchData);
}

// For CMD 31h response: unpacks firmware revision running on the keyboard
FirmwareSupport::Error LumatoneFirmwareDriver::unpackGetFirmwareRevisionResponse(const juce::MidiMessage& response, int& majorVersion, int& minorVersion, int& revision)
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
FirmwareSupport::Error LumatoneFirmwareDriver::unpackPingResponse(const juce::MidiMessage& response, int& value1, int& value2, int& value3, int& value4)
{
    auto status = messageIsValidLumatoneResponse(response);
    if (status != FirmwareSupport::Error::noError)
        return status;

    status = responseIsExpectedLength(response, 4);
    if (status != FirmwareSupport::Error::noError)
        return status;

    auto payload = &response.getSysExData()[PAYLOAD_INIT];
    value1 = payload[0];
    value2 = payload[1];
    value3 = payload[2];
    value4 = payload[3];

    return status;
}

// For CMD 33h response: echo payload
FirmwareSupport::Error LumatoneFirmwareDriver::unpackPingResponse(const juce::MidiMessage& response, unsigned int& value)
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
FirmwareSupport::Error LumatoneFirmwareDriver::unpackGetBoardThresholdValuesResponse(const juce::MidiMessage& response, int& boardId, int& minHighThreshold, int& minLowThreshold, int& maxThreshold, int& aftertouchThreshold, int& ccThreshold)
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
FirmwareSupport::Error LumatoneFirmwareDriver::unpackGetBoardSensitivityValuesResponse(const juce::MidiMessage& response, int& boardId, int& ccSensitivity, int& aftertouchSensitivity)
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
FirmwareSupport::Error LumatoneFirmwareDriver::unpackGetPeripheralChannelsResponse(const juce::MidiMessage& response, int& pitchWheelChannel, int& modWheelChannel, int& expressionChannel, int& sustainPedalChannel)
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
FirmwareSupport::Error LumatoneFirmwareDriver::unpackPeripheralCalibrationMode(const juce::MidiMessage& response, int& calibrationMode)
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
FirmwareSupport::Error LumatoneFirmwareDriver::unpackExpressionPedalCalibrationPayload(const juce::MidiMessage& response, int& minBound, int& maxBound, bool& valid)
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
FirmwareSupport::Error LumatoneFirmwareDriver::unpackWheelsCalibrationPayload(const juce::MidiMessage& response, int& centerPitch, int& minPitch, int& maxPitch, int& minMod, int& maxMod)
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
FirmwareSupport::Error LumatoneFirmwareDriver::unpackGetAftertouchTriggerDelayResponse(const juce::MidiMessage& response, int& boardId, int& triggerDelay)
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
FirmwareSupport::Error LumatoneFirmwareDriver::unpackGetLumatouchNoteOffDelayResponse(const juce::MidiMessage& response, int& boardId, int& delay)
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
FirmwareSupport::Error LumatoneFirmwareDriver::unpackGetExpressionPedalThresholdResponse(const juce::MidiMessage& response, int& thresholdValue)
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
FirmwareSupport::Error LumatoneFirmwareDriver::unpackGetPresetFlagsResponse(const juce::MidiMessage& response, bool& expressionInverted, bool& lightsOnKeystroke, bool& aftertouchOn, bool& sustainInverted)
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
FirmwareSupport::Error LumatoneFirmwareDriver::unpackGetExpressionPedalSensitivityResponse(const juce::MidiMessage& response, int& sensitivity)
{
    const short NUM_UNPACKED = 1;
    int unpackedData[NUM_UNPACKED];
    auto status = unpack7BitData(response, NUM_UNPACKED, unpackedData);
    if (status != FirmwareSupport::Error::noError)
        return status;

    sensitivity = unpackedData[0];

    return status;
}

void LumatoneFirmwareDriver::sendTestMessageNow(int outputDeviceIndex, const juce::MidiMessage &message)
{
    switch (hostMode)
    {
    case HostMode::Driver:
        HajuMidiDriver::sendTestMessageNow(outputDeviceIndex, message);
        break;
    case HostMode::Plugin:
        sendMessageNow(message);
        break;
    }
}

void LumatoneFirmwareDriver::sendMessageWithAcknowledge(const juce::MidiMessage &message)
{
    // Prevent certain messages from being sent
    if (onlySendRequestMessages && message.isSysEx())
    {
        auto sysExData = message.getSysExData();
        if (   sysExData[CMD_ID] == CHANGE_KEY_NOTE
            || sysExData[CMD_ID] == SET_KEY_COLOUR
            || sysExData[CMD_ID] == SET_VELOCITY_CONFIG
            || sysExData[CMD_ID] == SET_FADER_CONFIG
            || sysExData[CMD_ID] == SET_AFTERTOUCH_CONFIG
            || sysExData[CMD_ID] == SET_VELOCITY_INTERVALS
            || sysExData[CMD_ID] == SET_LUMATOUCH_CONFIG)
        {
            return;
        }
    }

//    jassert(midiInput != nullptr);
    if (hostMode == HostMode::Driver && getMidiInputIndex() < 0)
    {
        DBG("No juce::MidiInput open to send message to.");
//        sendMessageNow(message);

	    // Notify listeners
		// const juce::MessageManagerLock mmLock;
		// this->listeners.call(&Listener::midiMessageSent, message);
//        notifyMessageSent(midiOutput, message);
    }
    else
    {
        // Add message to queue first. The oldest message in queue will be sent.
		{
            juce::ScopedLock l(sysexQueue.getLock());
			sysexQueue.add(message);
			// const juce::MessageManagerLock mmLock;
			// this->listeners.call(&Listener::midiSendQueueSize, sysexQueue.size());
            notifySendQueueSize();
		}

        // If there is no message waiting for acknowledge: send oldest message of queue
       	if (!isTimerRunning() && !hasMsgWaitingForAck)
        {
            sendOldestMessageInQueue();
        }
    }
}

void LumatoneFirmwareDriver::sendOldestMessageInQueue()
{
    if (!sysexQueue.isEmpty())
    {
        jassert(!isTimerRunning());
        jassert(!hasMsgWaitingForAck);

        currentMsgWaitingForAck = sysexQueue[0];     // oldest element in buffer
        hasMsgWaitingForAck = true;
		sysexQueue.remove(0);                        // remove from buffer
        
        // const juce::MessageManagerLock mmLock;
        // this->listeners.call(&Listener::midiSendQueueSize, sysexQueue.size());
        notifySendQueueSize();

        sendCurrentMessage();
    }
}

void LumatoneFirmwareDriver::sendCurrentMessage()
{
    jassert(!isTimerRunning());
    jassert(hasMsgWaitingForAck);

#if JUCE_DEBUG
    if (currentMsgWaitingForAck.isSysEx())
    {
        auto sysExData = currentMsgWaitingForAck.getSysExData();
        for (int i = 0; i < currentMsgWaitingForAck.getSysExDataSize(); i++)
            jassert(sysExData[i] <= 0x7f);
    }
#endif

    sendMessageNow(currentMsgWaitingForAck);        // send it

    // Notify listeners
    DBG("SENT: " + currentMsgWaitingForAck.getDescription());
    // const juce::MessageManagerLock mmLock;
    // this->listeners.call(&Listener::midiMessageSent, currentMsgWaitingForAck);
    // notifyMessageSent(midiOutput, currentMsgWaitingForAck);

    timerType = TimerType::waitForAnswer;
    startTimer(receiveTimeoutInMilliseconds);       // Start waiting for answer
}

void LumatoneFirmwareDriver::handleIncomingMidiMessage(juce::MidiInput* source, const juce::MidiMessage& message)
{
#if JUCE_DEBUG
    if (message.isSysEx())
        DBG("RCVD: " + message.getDescription());
#endif

    //const juce::MessageManagerLock mmLock;
    //this->listeners.call(&Listener::midiMessageReceived, source, message);
    notifyMessageReceived(source, message);

    // Check whether received message is an answer to the previously sent one
    if (hasMsgWaitingForAck && messageIsResponseToMessage(message, currentMsgWaitingForAck))
    {
        jassert(timerType == TimerType::waitForAnswer);

        // Answer has come, we can stop the timer
        stopTimer();

        // Check answer state (error yes/no)
        auto answerState = message.getSysExData()[5];
        
        // This would be nice but we can't be sure the state is demo mode
//        if (answerState == TerpstraMIDIAnswerReturnCode::STATE)
//        {
//            // turn demo mode off
//            startDemoMode(false);
//        }

        // if answer state is "busy": resend message after a little delay
        if (answerState == TerpstraMIDIAnswerReturnCode::BUSY)
        {
            // Start delay timer, after which message will be sent again
            timerType = TimerType::delayWhileDeviceBusy;
            startTimer(busyTimeDelayInMilliseconds);
        }
        else
        {
            // In case of error, NACK: ?
            // For now: Remove from buffer in any case
            hasMsgWaitingForAck = false;

            // If there are more messages waiting in the queue: send the next one
            sendOldestMessageInQueue();
        }
    }

    // Other incoming messages are ignored
}

void LumatoneFirmwareDriver::timerCallback()
{
    stopTimer();

    if (timerType == TimerType::waitForAnswer)
    {
        // For now: Remove from buffer, try to send next one
        hasMsgWaitingForAck = false;

        // No answer came from MIDI input
		
        DBG("DRIVER: NO ANSWER");
        //const juce::MessageManagerLock mmLock;
        // listeners.call(&Listener::generalLogMessage, "No answer from device", HajuErrorVisualizer::ErrorLevel::error);
        // listeners.call(&Listener::noAnswerToMessage, currentMsgWaitingForAck);
        // notifyLogMessage("No answer from device", ErrorLevel::error);
        notifyNoAnswerToMessage(getMidiInputInfo(), currentMsgWaitingForAck);
    

        sendOldestMessageInQueue();
    }
    else if (timerType == TimerType::delayWhileDeviceBusy)
    {
        // Resend current message and start waiting for answer again
        sendCurrentMessage();
    }
    else
        jassertfalse;
}

void LumatoneFirmwareDriver::clearMIDIMessageBuffer()
{ 
    sysexQueue.clear();
    hasMsgWaitingForAck = false;
    stopTimer();
    // this->listeners.call(&Listener::midiSendQueueSize, 0); 
    notifySendQueueSize();
}
