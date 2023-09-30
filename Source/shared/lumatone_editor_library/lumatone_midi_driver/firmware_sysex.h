/*
  ==============================================================================

    firmware_sysex.h
    Created: 30 Sep 2023
    Author:  Vincenzo

  ==============================================================================
*/

#ifndef LUMATONE_FIRMWARE_SYSEX_H
#define LUMATONE_FIRMWARE_SYSEX_H

#include <JuceHeader.h>
#include "firmware_support.h"

struct LumatoneSysEx
{

// Fill a buffer with 3-bytes representing the manufacturer's ID
static void fillManufacturerId(unsigned char* data);

// Create a SysEx message with standardized length
static juce::MidiMessage createTerpstraSysEx(juce::uint8 boardIndex, juce::uint8 cmd, juce::uint8 data1, juce::uint8 data2, juce::uint8 data3, juce::uint8 data4);

// Create a SysEx message to send 8-bit colour precision to a Lumatone key
static juce::MidiMessage createExtendedKeyColourSysEx(juce::uint8 boardIndex, juce::uint8 cmd, juce::uint8 keyIndex, juce::uint8 redUpper, juce::uint8 redLower, juce::uint8 greenUpper, juce::uint8 greenLower, juce::uint8 blueUpper, juce::uint8 blueLower);
static juce::MidiMessage createExtendedKeyColourSysEx(juce::uint8 boardIndex, juce::uint8 cmd, juce::uint8 keyIndex, int red, int green, int blue);

// Create a SysEx message to send 8-bit colour precision to macro buttons
static juce::MidiMessage createExtendedMacroColourSysEx(juce::uint8 cmd, juce::uint8 redUpper, juce::uint8 redLower, juce::uint8 greenUpper, juce::uint8 greenLower, juce::uint8 blueUpper, juce::uint8 blueLower);
static juce::MidiMessage createExtendedMacroColourSysEx(juce::uint8 cmd, int red, int green, int blue);

// Create a SysEx message encoding a table with a defined size
static juce::MidiMessage createTableSysEx(juce::uint8 boardIndex, juce::uint8 cmd, juce::uint8 tableSize, const juce::uint8 table[]);


// Checks if message is a valid Lumatone firmware response and is expected length, then runs supplied unpacking function or returns an error code 
static FirmwareSupport::Error unpackIfValid(const juce::MidiMessage& response, size_t numBytes, std::function<FirmwareSupport::Error(const juce::uint8*)> unpackFunction);

// Generic unpacking of octave data from a SysEx message
static FirmwareSupport::Error unpackOctaveConfig(const juce::MidiMessage& msg, int& boardId, size_t numBytes, int* keyData, std::function<FirmwareSupport::Error(const juce::MidiMessage&, size_t, int*)> nBitUnpackFunction);

// Generic unpacking of 7-bit data from a SysEx message
static FirmwareSupport::Error unpack7BitData(const juce::MidiMessage& msg, size_t numBytes, int* unpackedData);

// Unpacking of octave-based 7-bit key configuration data
static FirmwareSupport::Error unpack7BitOctaveData(const juce::MidiMessage& msg, int& boardId, juce::uint8 numKeys, int* keyData);

// Generic unpacking of 8-bit data from a SysEx message
static FirmwareSupport::Error unpack8BitData(const juce::MidiMessage& msg, size_t numBytes, int* unpackedData);

// Unpacking of octave-based 8-bit data
static FirmwareSupport::Error unpack8BitOctaveData(const juce::MidiMessage& msg, int& boardId, juce::uint8 numKeys, int* keyData);

// Generic unpacking of 12-bit data from a SysEx message, when packed with two 7-bit values
static FirmwareSupport::Error unpack12BitDataFrom7Bit(const juce::MidiMessage& msg, size_t numBytes, int* unpackedData);

// Generic unpacking of 12-bit data from a SysEx message, when packed with three 4-bit values
static FirmwareSupport::Error unpack12BitDataFrom4Bit(const juce::MidiMessage& msg, size_t numBytes, int* unpackedData);


// Message is SysEx message with Lumatone Inc. manufacturer ID, valid board ID byte, and acknowledged status
static FirmwareSupport::Error messageIsValidLumatoneResponse(const juce::MidiMessage& midiMessage);

// Determines if response is too short, too long, or expected
static FirmwareSupport::Error responseIsExpectedLength(const juce::MidiMessage& midiMessage, size_t numPayloadBytes);

//============================================================================
// Specific commands for unpacking responses, returns error code

// For CMD 13h response: unpacks 8-bit key data for red LED intensity. 112 bytes, lower and upper nibbles for 56 values
static FirmwareSupport::Error unpackGetLEDConfigResponse(const juce::MidiMessage& response, int& boardId, int* keyData);

// For CMD 13h response: unpacks 7-bit key data for red LED intensity. 56 bytes, each value must be multiplied by 5
static FirmwareSupport::Error unpackGetLEDConfigResponse_Version_1_0_0(const juce::MidiMessage& response, int& boardId, juce::uint8 numKeys, int* keyData);

// For CMD 16h response: unpacks 7-bit channel data for note configuration. 55 or 56 bytes
static FirmwareSupport::Error unpackGetChannelConfigResponse(const juce::MidiMessage& response, int& boardId, juce::uint8 numKeys, int* keyData);

// For CMD 17h response: unpacks 7-bit key data for note configuration. 55 or 56 bytes
static FirmwareSupport::Error unpackGetNoteConfigResponse(const juce::MidiMessage& response, int& boardId, juce::uint8 numKeys, int* keyData);

// For CMD 18h response: unpacks 7-bit key type data for key configuration. 55 or 56 bytes
static FirmwareSupport::Error unpackGetTypeConfigResponse(const juce::MidiMessage& response, int& boardId, juce::uint8 numKeys, int* keyData);

// For CMD 19h response: unpacks 8-bit key data for maximums of adc threshold. 55 or 56 bytes
static FirmwareSupport::Error unpackGetKeyMaxThresholdsResponse(const juce::MidiMessage& response, int& boardId, juce::uint8 numKeys, int* keyData);

// For CMD 1Ah response: unpacks 8-bit key data for minimums of adc threshold. 55 or 56 bytes
static FirmwareSupport::Error unpackGetKeyMinThresholdsResponse(const juce::MidiMessage& response, int& boardId, juce::uint8 numKeys, int* keyData);

// For CMD 1Bh response: unpacks 8-bit key data for maximums of adc threshold for aftertouch triggering. 55 or 56 bytes
static FirmwareSupport::Error unpackGetAftertouchMaxThresholdsResponse(const juce::MidiMessage& response, int& boardId, juce::uint8 numKeys, int* keyData);

// For CMD 1Ch response: unpacks boolean key validity data for board, whether or not each key meets threshold specs
static FirmwareSupport::Error unpackGetKeyValidityResponse(const juce::MidiMessage& response, int& boardId, juce::uint8 numKeys, bool* keyValidityData);

// For CMD 1Dh response: unpacks 7-bit velocity configuration of keyboard, 128 bytes
static FirmwareSupport::Error unpackGetVelocityConfigResponse(const juce::MidiMessage& response, int* velocityData);

// For CMD 1Eh response: unpacks 7-bit fader configuration of keyboard, 128 bytes
static FirmwareSupport::Error unpackGetFaderConfigResponse(const juce::MidiMessage& response, int* faderData);

// For CMD 1Fh response: unpacks 7-bit aftertouch configuration of keyboard, 128 bytes
static FirmwareSupport::Error unpackGetAftertouchConfigResponse(const juce::MidiMessage& response, int* aftertouchData);

// For CMD 21h response: unpacks 12-bit velocity interval configuration of keyboard, 254 bytes encoding 127 values
static FirmwareSupport::Error unpackGetVelocityIntervalConfigResponse(const juce::MidiMessage& response, int* intervalData);

// For CMD 22h response: unpacks 7-bit fader type configuration of board, 56 bytes
static FirmwareSupport::Error unpackGetFaderConfigResponse(const juce::MidiMessage& response, int& boardId, juce::uint8 numKeys, int* faderData);

// For CMD 23h response: unpacks serial ID number of keyboard, 12 7-bit values encoding 6 bytes
static FirmwareSupport::Error unpackGetSerialIdentityResponse(const juce::MidiMessage& response, int* serialBytes);

// For CMD 30h response: unpacks 7-bit Lumatouch configuration of keyboard, 128 bytes
static FirmwareSupport::Error unpackGetLumatouchConfigResponse(const juce::MidiMessage& response, int* lumatouchData);

// For CMD 31h response: unpacks firmware revision running on the keyboard
static FirmwareSupport::Error unpackGetFirmwareRevisionResponse(const juce::MidiMessage& response, int& majorVersion, int& minorVersion, int& revision);

// For CMD 33h response: echo payload
// static FirmwareSupport::Error unpackPingResponse(const juce::MidiMessage& response, int& value1, int& value2, int& value3, int& value4);
static FirmwareSupport::Error unpackPingResponse(const juce::MidiMessage& response, unsigned int& value);

// For CMD 3Ah response: retrieve all 8-bit threshold values of a certain board
static FirmwareSupport::Error unpackGetBoardThresholdValuesResponse(const juce::MidiMessage& response, int& boardId, int& minHighThreshold, int& minLowThreshold, int& maxThreshold, int& aftertouchThreshold, int& ccThreshold);

// For CMD 3Bh response: retrieve all threshold values of a certain board
static FirmwareSupport::Error unpackGetBoardSensitivityValuesResponse(const juce::MidiMessage& response, int& boardId, int& ccSensitivity, int& aftertouchSensitivity);

// For CMD 3Dh response: retrieve MIDI channels of which peripherals are configured
static FirmwareSupport::Error unpackGetPeripheralChannelsResponse(const juce::MidiMessage& response, int& pitchWheelChannel, int& modWheelChannel, int& expressionChannel, int& sustainPedalChannel);

// For CMD 3Eh response: read back the calibration mode of the message
static FirmwareSupport::Error unpackPeripheralCalibrationMode(const juce::MidiMessage& response, int& calibrationMode);

// For CMD 3Eh response: retrieve 12-bit expression pedal calibration status values in respective mode, automatically sent every 100ms
static FirmwareSupport::Error unpackExpressionPedalCalibrationPayload(const juce::MidiMessage& response, int& minBound, int& maxBound, bool& valid);

// For CMD 3Eh response: retrieve 12-bit pitch & mod wheel calibration status values in respective mode, automatically sent every 100ms
static FirmwareSupport::Error unpackWheelsCalibrationPayload(const juce::MidiMessage& response, int& centerPitch, int& minPitch, int& maxPitch, int& minMod, int& maxMod);

// For CMD 40h response: retrieve 8-bit aftertouch trigger delay of a certain board
static FirmwareSupport::Error unpackGetAftertouchTriggerDelayResponse(const juce::MidiMessage& response, int& boardId, int& triggerDelay);

// For CMD 42h response: retrieve 12-bit Lumatouch note off delay of a certain board
static FirmwareSupport::Error unpackGetLumatouchNoteOffDelayResponse(const juce::MidiMessage& response, int& boardId, int& delay);

// For CMD 44h response: retrieve 12-bit expression pedal adc threshold
static FirmwareSupport::Error unpackGetExpressionPedalThresholdResponse(const juce::MidiMessage& response, int& thresholdValue);

// For CMD 47h response: retrieve preset flags
static FirmwareSupport::Error unpackGetPresetFlagsResponse(const juce::MidiMessage& response, bool& expressionInverted, bool& lightsOnKeystroke, bool& aftertouchOn, bool& sustainInverted);

// For CMD 48h response: get expression pedal sensitivity
static FirmwareSupport::Error unpackGetExpressionPedalSensitivityResponse(const juce::MidiMessage& response, int& sensitivity);

// Message is an answer to a sent message yes/no
static bool messageIsResponseToMessage(const juce::MidiMessage& answer, const juce::MidiMessage& originalMessage);
};

#endif
