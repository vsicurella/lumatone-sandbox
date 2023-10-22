/*
  ==============================================================================

    LumatoneFirmwareDriver.cpp
    Created: 20 Feb 2015 8:04:02pm
    Author:  hsstraub

  ==============================================================================
*/

#ifndef LUMATONE_FIRMWARE_DRIVER_H
#define LUMATONE_FIRMWARE_DRIVER_H

#include "lumatone_midi_driver.h"
#include "./firmware_sysex.h"
#include "./firmware_driver_listener.h"

// There are different race-condition issues between macOS and Windows. 
// This Driver may need to be redesigned, but for now this define is
// used for including a juce::MessageManagerLock on Windows & Linux, but not on macOS.

#define MIDI_DRIVER_USE_LOCK JUCE_WINDOWS //|| JUCE_LINUX

LumatoneFirmwareDriver::LumatoneFirmwareDriver(HostMode hostModeIn, int numBoardsIn)
    : hostMode(hostModeIn)
    , numBoards(numBoardsIn)
{
}     

LumatoneFirmwareDriver::~LumatoneFirmwareDriver()
{
    listeners.clear();
}

//============================================================================
// TerpstaMidiDriver::LumatoneFirmwareDriverListener helpers

void LumatoneFirmwareDriver::addDriverListener(LumatoneFirmwareDriverListener* collectorToAdd)
{ 
    listeners.add(collectorToAdd);
}

void LumatoneFirmwareDriver::removeDriverListener(LumatoneFirmwareDriverListener* collectorToRemove)
{
    listeners.remove(collectorToRemove);
}

void LumatoneFirmwareDriver::readNextBuffer(juce::MidiBuffer &nextBuffer)
{
    juce::ScopedTryLock l(hostLock);
    if (l.isLocked())
    {
        nextBuffer.swapWith(hostQueue);
        hostQueue.clear();
        hostQueueSize = 0;
    }
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
        juce::ScopedLock l(hostLock);
        hostQueue.addEvent(msg, hostQueueSize++);
    }
    break;
    }
}

void LumatoneFirmwareDriver::notifyMessageReceived(juce::MidiInput* source, const juce::MidiMessage& midiMessage)
{
// #if MIDI_DRIVER_USE_LOCK
//     const juce::MessageManagerLock lock;
// #endif
//     juce::MessageManager::callAsync([this, source, midiMessage]{
//         for (auto collector : listeners) collector->midiMessageReceived(source, midiMessage);
//     });
    listeners.call(&LumatoneFirmwareDriverListener::midiMessageReceived, source, midiMessage);
}

void LumatoneFirmwareDriver::notifyMessageSent(juce::MidiOutput* target, const juce::MidiMessage& midiMessage)
{
    // Currently unused
// #if MIDI_DRIVER_USE_LOCK
//     const juce::MessageManagerLock lock;
// #endif

//     juce::MessageManager::callAsync([this, target, midiMessage]{
//         for (auto collector : listeners) collector->midiMessageSent(target, midiMessage);
//     });
}

void LumatoneFirmwareDriver::notifySendQueueSize()
{
    auto size = sysexQueue.size();
    // for (auto collector : listeners) collector->midiSendQueueSize(size);
    listeners.call(&LumatoneFirmwareDriverListener::midiSendQueueSize, size);
}

// void LumatoneFirmwareDriver::notifyLogMessage(juce::String textMessage, ErrorLevel errorLevel)
// {
//     for (auto collector : listeners) collector->generalLogMessage(textMessage, errorLevel);
// }

void LumatoneFirmwareDriver::notifyNoAnswerToMessage(juce::MidiDeviceInfo expectedDevice, const juce::MidiMessage& midiMessage)
{
    // juce::MessageManager::callAsync([this, expectedDevice, midiMessage]{
    //     for (auto collector : listeners) collector->noAnswerToMessage(expectedDevice, midiMessage);
    // });
    listeners.call(&LumatoneFirmwareDriverListener::noAnswerToMessage, expectedDevice, midiMessage);
}

/*
==============================================================================
Single (mid-level) commands, firmware specific
*/

// CMD 00h: Send a single key's functionctional configuration
void LumatoneFirmwareDriver::sendKeyFunctionParameters(juce::uint8 boardIndex, juce::uint8 keyIndex, juce::uint8 noteOrCCNum, juce::uint8 midiChannel, juce::uint8 keyType, bool faderUpIsNull)
{
    // DBG("SEND KEY FUNCTION REQUESTED " + juce::String(boardIndex) + "," + juce::String(keyIndex));
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
    // DBG("SEND KEY COLOUR REQUESTED " + juce::String(boardIndex) + "," + juce::String(keyIndex));

    juce::MidiMessage msg = LumatoneSysEx::createExtendedKeyColourSysEx(boardIndex, SET_KEY_COLOUR, keyIndex, red, green, blue);

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
 
    juce::MidiMessage msg = LumatoneSysEx::createExtendedKeyColourSysEx(boardIndex, SET_KEY_COLOUR, keyIndex, redUpper, redLower, greenUpper, greenLower, blueUpper, blueLower);
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
    juce::MidiMessage msg = LumatoneSysEx::createExtendedMacroColourSysEx(MACROBUTTON_COLOUR_ON, red, green, blue);
    sendMessageWithAcknowledge(msg);
}

// CMD 05h: Colour for macro button in active state, 3 pairs for 4-bit values for each LED channel
void LumatoneFirmwareDriver::sendMacroButtonActiveColour(juce::uint8 redUpper, juce::uint8 redLower, juce::uint8 greenUpper, juce::uint8 greenLower, juce::uint8 blueUpper, juce::uint8 blueLower)
{
    juce::MidiMessage msg = LumatoneSysEx::createExtendedMacroColourSysEx(MACROBUTTON_COLOUR_ON, redUpper, redLower, greenUpper, greenLower, blueUpper, blueLower);
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
    juce::MidiMessage msg = LumatoneSysEx::createExtendedMacroColourSysEx(MACROBUTTON_COLOUR_OFF, red, green, blue);
    sendMessageWithAcknowledge(msg);
}

// CMD 05h: Colour for macro button in active state, 3 pairs for 4-bit values for each LED channel
void LumatoneFirmwareDriver::sendMacroButtonInactiveColour(juce::uint8 redUpper, juce::uint8 redLower, juce::uint8 greenUpper, juce::uint8 greenLower, juce::uint8 blueUpper, juce::uint8 blueLower)
{
    juce::MidiMessage msg = LumatoneSysEx::createExtendedMacroColourSysEx(MACROBUTTON_COLOUR_OFF, redUpper, redLower, greenUpper, greenLower, blueUpper, blueLower);
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

    auto msg = LumatoneSysEx::createTableSysEx(0, SET_VELOCITY_CONFIG, 128, reversedTable);
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
    juce::MidiMessage msg = LumatoneSysEx::createTableSysEx(0, SET_FADER_CONFIG, 128, faderTable);
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
    juce::MidiMessage msg = LumatoneSysEx::createTableSysEx(0, SET_AFTERTOUCH_CONFIG, 128, aftertouchTable);
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

	juce::MidiMessage msg = LumatoneSysEx::createTableSysEx(0, SET_VELOCITY_INTERVALS, payloadSize, formattedTable);
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
        sendTestMessageNow(sendToTestDevice, LumatoneSysEx::createTerpstraSysEx(0, GET_SERIAL_IDENTITY, TEST_ECHO, '\0', '\0', '\0'));
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
        sendTestMessageNow(testOutputIndex, LumatoneSysEx::createTerpstraSysEx(0, CALIBRATE_PITCH_MOD_WHEEL, startCalibration, TEST_ECHO, '\0', '\0'));
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
    juce::MidiMessage msg = LumatoneSysEx::createTableSysEx(0, SET_LUMATOUCH_CONFIG, 128, lumatouchTable);
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
        sendTestMessageNow(sendToTestDevice, LumatoneSysEx::createTerpstraSysEx(0, GET_FIRMWARE_REVISION, TEST_ECHO, '\0', '\0', '\0'));
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
        sendTestMessageNow(sendToTestDevice, LumatoneSysEx::createTerpstraSysEx(0, LUMA_PING, TEST_ECHO, value1, value2, value3));
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
    juce::MidiMessage msg = LumatoneSysEx::createTerpstraSysEx(boardIndex, cmd, data1, data2, data3, data4);
    sendMessageWithAcknowledge(msg);
}

// Send a SysEx message without parameters
void LumatoneFirmwareDriver::sendSysExRequest(juce::uint8 boardIndex, juce::uint8 cmd)
{
    unsigned char sysExData[9];
    LumatoneSysEx::fillManufacturerId(sysExData);
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
    LumatoneSysEx::fillManufacturerId(sysExData);
    sysExData[3] = boardIndex;
    sysExData[4] = cmd;
    sysExData[5] = turnStateOn;
    sysExData[6] = '\0';
    sysExData[7] = '\0';
    sysExData[8] = '\0';
    auto msg = juce::MidiMessage::createSysExMessage(sysExData, 9);
    sendMessageWithAcknowledge(msg);
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

    if (hostMode == HostMode::Driver && getMidiInputIndex() < 0)
    {
        DBG("No juce::MidiInput open to send message to.");
        // notifyMessageSent(midiOutput, message);
    }
    else
    {
        // Add message to queue first. The oldest message in queue will be sent.
        {
            juce::ScopedLock l(sysexQueue.getLock());
			sysexQueue.add(message);
            notifySendQueueSize();
        }

        // If there is no message waiting for acknowledge: send oldest message of queue
       	if (timerType == TimerType::checkQueue || !hasMsgWaitingForAck)
        {
            stopTimer();
            sendOldestMessageInQueue();
        }
    }
}

void LumatoneFirmwareDriver::sendOldestMessageInQueue()
{
    if (sysexQueue.isEmpty())
        return;

    // jassert(timerType == TimerType::checkQueue);
    jassert(!isTimerRunning());
    jassert(!hasMsgWaitingForAck);

    currentMsgWaitingForAck = sysexQueue[0];     // oldest element in buffer
    hasMsgWaitingForAck = true;
    receivedAnswer = false;

    {
        juce::ScopedLock l(sysexQueue.getLock());
        sysexQueue.remove(0);                        // remove from buffer
    }
    
    notifySendQueueSize();
    sendCurrentMessage();
}

void LumatoneFirmwareDriver::sendCurrentMessage()
{
    jassert(!isTimerRunning());
    jassert(hasMsgWaitingForAck);

// #if JUCE_DEBUG
//     if (currentMsgWaitingForAck.isSysEx())
//     {
//         auto sysExData = currentMsgWaitingForAck.getSysExData();
//         for (int i = 0; i < currentMsgWaitingForAck.getSysExDataSize(); i++)
//             jassert(sysExData[i] <= 0x7f);
//     }
// #endif

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
    {
        if (source)
            DBG("RCVD: " + message.getDescription() + "; from " + source->getName());
        else
            DBG("RCVD: " + message.getDescription() + "; called by processor");
    }
#endif

    juce::MessageManager::callAsync([=]() { notifyMessageReceived(source, message); });

    if (!hasMsgWaitingForAck)
        return;

    jassert(timerType == TimerType::waitForAnswer);
 
    // Check whether received message is an answer to the previously sent one
    if (LumatoneSysEx::messageIsResponseToMessage(message, currentMsgWaitingForAck))
    {
        // Answer has come, we can stop the timer
        stopTimer();
        
        receivedAnswer = true;

        // Check answer state (error yes/no)
        auto answerState = message.getSysExData()[MSG_STATUS];

        // if answer state is "busy": resend message after a little delay
        if (answerState == LumatoneFirmware::ReturnCode::BUSY)
        {
            // Start delay timer, after which message will be sent again
            timerType = TimerType::delayWhileDeviceBusy;
            DBG("Starting Busy Timer");
            startTimer(busyTimeDelayInMilliseconds);
        }
        else
        {
            // In case of error, NACK: ?
            // For now: Remove from buffer in any case
            hasMsgWaitingForAck = false;

            // If there are more messages waiting in the queue: send the next one
            timerType = TimerType::checkQueue;
            startTimer(checkQueueTimerDelayInMilliseconds);
        }
    }

    // Other incoming messages are ignored
}

void LumatoneFirmwareDriver::timerCallback()
{
    jassert(
           ((hasMsgWaitingForAck == false) && (receivedAnswer == true))
        || ((hasMsgWaitingForAck == true) && (receivedAnswer == false))
        );

    stopTimer();

    if (timerType == TimerType::waitForAnswer)
    {
        // Rare race condition
        if (receivedAnswer)
        {
            DBG("DRIVER: Got answer in timercallback, bailing no answer!");
            return;
        }

        // For now: Remove from buffer, try to send next one
        hasMsgWaitingForAck = false;

        // No answer came from MIDI input
		
        DBG("DRIVER: NO ANSWER");
        notifyNoAnswerToMessage(getMidiInputInfo(), currentMsgWaitingForAck);

        sendOldestMessageInQueue();
    }
    else if (timerType == TimerType::delayWhileDeviceBusy)
    {
        // Resend current message and start waiting for answer again
        sendCurrentMessage();
    }
    else if (timerType == TimerType::checkQueue)
    {
        jassert(!hasMsgWaitingForAck);
        sendOldestMessageInQueue();
    }
    else
        jassertfalse;
}

void LumatoneFirmwareDriver::clearMIDIMessageBuffer()
{
    stopTimer();
    hasMsgWaitingForAck = false;

    {
        juce::ScopedLock l(sysexQueue.getLock());
        sysexQueue.clear();
    }

    {
        juce::ScopedLock l(hostLock);
        hostQueue.clear();
    }

    notifySendQueueSize();
}

#endif
