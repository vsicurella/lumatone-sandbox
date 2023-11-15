/*
  ==============================================================================

    TerpstraMidiDriver.h
    Created: 20 Feb 2015 8:04:02pm
    Author:  hsstraub

  ==============================================================================
*/

/** BEGIN_JUCE_MODULE_DECLARATION

	ID:             lumatone_midi_driver
	vendor:         vsicurella
	version:        0.0.1
	name:           Lumatone MIDI Library
	description:
	website:
	license:        BSD-0

	dependencies:	juce_core, juce_audio_basics, juce_audio_devices, juce_events

	END_JUCE_MODULE_DECLARATION
*/

#ifndef LUMATONE_DRIVER_H
#define LUMATONE_DRIVER_H

//[Headers]     -- You can add your own extra header files here --
#include <JuceHeader.h>

#include "./midi_driver.h"
#include "./firmware_driver_listener.h"

// #include "../../debug/LumatoneSandboxLogger.h"

#define DEFAULT_NUM_BOARDS 5

//[/Headers]

/*
==============================================================================
Connection to midi, sending SysEx parameters to keyboard
==============================================================================
*/
class LumatoneFirmwareDriver : public HajuMidiDriver
							, public juce::Timer
							// , private LumatoneSandboxLogger
{
public:
	enum class HostMode
	{
		Driver = 0, // For applications, open and manage devices with HajuMidiDriver
		Plugin // Hosted in a DAW as a plugin - MIDI messages are received through DAW's buffer	
	};

private:
	enum class TimerType
	{
		waitForAnswer,
		delayWhileDeviceBusy,
		checkQueue
	};

public:

	enum class ErrorLevel
	{
		noError = 0,
		warning = 1,
		error = 2
	};

private:
	// Helper callbacks for notifying Collectors
	void notifyMessageReceived(juce::MidiInput* source, const juce::MidiMessage& midiMessage);
	void notifyMessageSent(juce::MidiOutput* target, const juce::MidiMessage& midiMessage);
	void notifySendQueueSize();
	// void notifyLogMessage(juce::String textMessage, ErrorLevel errorLevel);
    void notifyNoAnswerToMessage(juce::MidiDeviceInfo expectedDevice, const juce::MidiMessage& midiMessage);
//	void notifyTestMessageReceived(int testInputIndex, const juce::MidiMessage& midiMessage);


public:
	LumatoneFirmwareDriver(HostMode hostModeIn, int numBoards=DEFAULT_NUM_BOARDS);
	~LumatoneFirmwareDriver();

	HostMode getHostMode() const { return hostMode; }

    void addDriverListener(LumatoneFirmwareDriverListener* collectorToAdd);
    void removeDriverListener(LumatoneFirmwareDriverListener* collectorToRemove);

	void readNextBuffer(juce::MidiBuffer& nextBuffer);

	void restrictToRequestMessages(bool testMessagesOnly) { onlySendRequestMessages = testMessagesOnly; }

	bool isWaitingForResponse() const { return hasMsgWaitingForAck; }

	// Low-level send MIDI message in a host dependent way
	void sendMessageNow(const juce::MidiMessage& msg);

	//============================================================================
	// Single (mid-level) commands, firmware specific

	// CMD 00h: Send a single key's functionctional configuration
	void sendKeyFunctionParameters(juce::uint8 boardIndex, juce::uint8 keyIndex, juce::uint8 noteOrCCNum, juce::uint8 midiChannel, juce::uint8 keyType, bool faderUpIsNull = true);

	// CMD 01h: Send a single key's LED channel intensities, three 8-bit values
	void sendKeyLightParameters(juce::uint8 boardIndex, juce::uint8 keyIndex, juce::uint8 red, juce::uint8 green, juce::uint8 blue);
	// CMD 01h: Send a single key's LED channel intensities, three pairs of 4-bit values for each channel
	void sendKeyLightParameters(juce::uint8 boardIndex, juce::uint8 keyIndex, juce::uint8 redUpper, juce::uint8 redLower, juce::uint8 greenUpper, juce::uint8 greenLower, juce::uint8 blueUpper, juce::uint8 blueLower);
	// CMD 01h: Send a single key's LED channel intensities, three 7-bit values
	void sendKeyLightParameters_Version_1_0_0(juce::uint8 boardIndex, juce::uint8 keyIndex, juce::uint8 red, juce::uint8 green, juce::uint8 blue);

	// CMD 02h: Save current configuration to specified preset index
	void saveProgram(juce::uint8 presetNumber);

	// CMD 03h: Send expression pedal sensivity
	void sendExpressionPedalSensivity(juce::uint8 value);

	// CMD 04h: Send parametrization of foot controller
	void sendInvertFootController(bool value);

	// CMD 05h: Colour for macro button in active state, 3 8-bit values
	void sendMacroButtonActiveColour(juce::uint8 red, juce::uint8 green, juce::uint8 blue);
	// CMD 05h: Colour for macro button in active state, 3 pairs for 4-bit values for each LED channel
	void sendMacroButtonActiveColour(juce::uint8 redUpper, juce::uint8 redLower, juce::uint8 greenUpper, juce::uint8 greenLower, juce::uint8 blueUpper, juce::uint8 blueLower);
	// CMD 05h: Colour for macro button in active state, 3 7-bit values
	void sendMacroButtonActiveColour_Version_1_0_0(juce::uint8 red, juce::uint8 green, juce::uint8 blue);

	// CMD 06h: Colour for macro button in inactive state
	void sendMacroButtonInactiveColour(int red, int green, int blue);
	// CMD 05h: Colour for macro button in active state, 3 pairs for 4-bit values for each LED channel
	void sendMacroButtonInactiveColour(juce::uint8 redUpper, juce::uint8 redLower, juce::uint8 greenUpper, juce::uint8 greenLower, juce::uint8 blueUpper, juce::uint8 blueLower);
	// CMD 05h: Colour for macro button in active state, 3 7-bit values
	void sendMacroButtonInactiveColour_Version_1_0_0(juce::uint8 red, juce::uint8 green, juce::uint8 blue);

	// CMD 07h: Send parametrization of light on keystrokes
	void sendLightOnKeyStrokes(bool value);

	// CMD 08h: Send a value for a velocity lookup table (128 7-bit values)
	void sendVelocityConfig(const juce::uint8 velocityTable[]);

	// CMD 09h: Save velocity config to EEPROM
	void saveVelocityConfig();

	// CMD 0Ah: Reset velocity config to value from EEPROM
	void resetVelocityConfig();

	// CMD 0Bh: Adjust the internal fader look-up table (size of 128)
	void sendFaderConfig(const juce::uint8 faderTable[]);

	// CMD 0Ch: **DEPRECATED** Save the changes made to the fader look-up table
	void saveFaderConfiguration();

	// CMD 0Dh: Reset the fader lookup table back to its factory fader settings.
	void resetFaderConfig();

	// CMD 0Eh: Enable or disable aftertouch functionality
	void sendAfterTouchActivation(bool value);

	// CMD 0Fh: Initiate aftertouch calibration routine
	void sendCalibrateAfterTouch();

	// CMD 10h: Adjust the internal aftertouch look-up table (size of 128)
	void sendAftertouchConfig(const juce::uint8 aftertouchTable[]);

	// CMD 11h: **DEPRECATED** Save the changes made to the aftertouch look-up table
	void saveAftertouchConfig();

	// CMD 12h: Reset the aftertouch lookup table back to its factory aftertouch settings.
	void resetAftertouchConfig();

	// CMD 13h: Read back the current red intensity of all the keys of the target board.
	void sendRedLEDConfigRequest(juce::uint8 boardIndex);

	// CMD 14h: Read back the current green intensity of all the keys of the target board.
	void sendGreenLEDConfigRequest(juce::uint8 boardIndex);

	// CMD 15h: Read back the current blue intensity of all the keys of the target board.
	void sendBlueLEDConfigRequest(juce::uint8 boardIndex);

	// CMD 16h: Read back the current channel configuration of all the keys of the target board.
	void sendChannelConfigRequest(juce::uint8 boardIndex);

	// CMD 17h: Read back the current note configuration of all the keys of the target board.
	void sendNoteConfigRequest(juce::uint8 boardIndex);

	// CMD 18h: Read back the current key type configuration of all the keys of the target board.
	void sendKeyTypeConfigRequest(juce::uint8 boardIndex);

	// CMD 19h: Read back the maximum fader (note on) threshold of all the keys of the target board.
	void sendMaxFaderThresholdRequest(juce::uint8 boardIndex);

	// CMD 1Ah: Read back the maximum fader (note on) threshold of all the keys of the target board
	void sendMinFaderThresholdRequest(juce::uint8 boardIndex);

	// CMD 1Bh: Read back the aftertouch maximum threshold of all the keys of the target board
	void sendMaxAftertouchThresholdRequest(juce::uint8 boardIndex);

	// CMD 1Ch: Get back flag whether or not each key of target board meets minimum threshold
	void sendKeyValidityParametersRequest(juce::uint8 boardIndex);
	
	// CMD 1Dh: Read back the current velocity look up table of the keyboard.
	void sendVelocityConfigRequest();

	// CMD 1Eh: Read back the current fader look up table of the keyboard.
	void sendFaderConfigRequest();

	// CMD 1Fh: Read back the current aftertouch look up table of the keyboard.
	void sendAftertouchConfigRequest();

	// CMD 20h: Set the velocity interval table, 127 12-bit values (up to 0x7fff
    void sendVelocityIntervalConfig(const int velocityIntervalTable[]);

	// CMD 21h: Sead back the velocity interval table
	void sendVelocityIntervalConfigRequest();

	// CMD 22h: Read back the fader type of all keys on the targeted board.
	void sendFaderTypeConfigRequest(juce::uint8 boardIndex);

	// CMD 23h: This command is used to read back the serial identification number of the keyboard.
	void sendGetSerialIdentityRequest(int sendToTestDevice = -1);

	// CMD 24h: Initiate the key calibration routine; each pair of macro buttons  
	// on each octave must be pressed to return to normal state
	void sendCalibrateKeys();

	// CMD 25h: Pass in true to enter Demo Mode, or false to exit
	void startDemoMode(bool turnOn);

	// CMD 26h: Initiate the pitch and mod wheel calibration routine, pass in false to stop
	void sendCalibratePitchModWheel(bool startCalibration, int testOutputIndex = -1);

	// CMD 27h: Set the sensitivity value of the mod wheel, 0x01 to 0x07f
	void setModWheelSensitivity(juce::uint8 sensitivity);

	// CMD 28h: Set the sensitivity value of the pitch bend wheel between 0x01 and 0x3FFF
	void setPitchBendSensitivity(int sensitivity);

	// CMD 29h: Set abs. distance from max value to trigger CA-004 submodule key events, ranging from 0x00 to 0xFE
	void setKeyMaximumThreshold(juce::uint8 boardIndex, juce::uint8 maxThreshold, juce::uint8 aftertouchMax);

	// CMD 2Ah: Set abs. distance from min value to trigger CA-004 submodule key events, ranging from 0x00 to 0xFE
	void setKeyMinimumThreshold(juce::uint8 boardIndex, juce::uint8 minThresholdHigh, juce::uint8 minThresholdLow);

	// CMD 2Bh: Set the sensitivity for CC events, ranging from 0x00 to 0xFE
	void setFaderKeySensitivity(juce::uint8 boardIndex, juce::uint8 sensitivity);

	// CMD 2Ch: Set the target board sensitivity for aftertouch events, ranging from 0x00 to 0xFE
	void setAftertouchKeySensitivity(juce::uint8 boardIndex, juce::uint8 sensitivity);

	// CMD 2Dh: Adjust the Lumatouch table, 127 7-bit values, where 127 is a result of a fully pressed key
	void setLumatouchConfig(const juce::uint8 lumatouchTable[]);

	// CMD 2Eh: **DEPRECATED** Save Lumatouch table changes
	void saveLumatoneConfig();

	// CMD 2Fh: Reset the Lumatouch table back to factory settings
	void resetLumatouchConfig();

	// CMD 30h: Read back the Lumatouch table
	void sendLumatouchConfigRequest();

	// CMD 31h: This command is used to read back the current Lumatone firmware revision.
	// The firmware version format is in the form {Major}.{Minor}.{Revision}
	// If the board has not been initialized, the Beaglebone will contain a firmware revision of 0.0.0 for the board
	void sendGetFirmwareRevisionRequest(int sendToTestDevice = -1);

	// CMD 32h: Set the thresold from key�s min value to trigger CA - 004 submodule CC events, ranging from 0x00 to 0xFE
	void setCCActiveThreshold(juce::uint8 boardIndex, juce::uint8 sensitivity);

	// CMD 33h: Echo the payload, 3 7-bit values, for use in connection monitoring
    // the first 7-bit value is reserved for echo differentiation
	void ping(juce::uint8 value1, juce::uint8 value2, juce::uint8 value3, int sendToTestDevice = -1);
	unsigned int ping(unsigned int value, int sendToTestDevice = -1);

	// CMD 34h: Reset the thresholds for events and sensitivity for CC & aftertouch on the target board
	void resetBoardThresholds(juce::uint8 boardIndex);

	// CMD 35h: Enable/disable key sampling over SSH for the target key and board
	void setKeySampling(juce::uint8 boardIndex, juce::uint8 keyIndex, bool turnSamplingOn);

	// CMD 36h: Set thresholds for the pitch and modulation wheel to factory settings
	void resetWheelsThresholds();

	// CMD 37h: Set the bounds from the calibrated zero adc value of the pitch wheel, 0x00 to 0x7f
	void setPitchWheelZeroThreshold(juce::uint8 threshold);

	// CMD 38h: Pass in true to initiate the expression pedal calibration routine, or false to stop
	void calibrateExpressionPedal(bool startCalibration);

	// CMD 39h: Reset expression pedal minimum and maximum bounds to factory settings
	void resetExpressionPedalBounds();

	// CMD 3Ah: Retrieve the threshold values of target board
	void getBoardThresholdValues(juce::uint8 boardIndex);

	// CMD 3Bh: Retrieve the sensitivity values of target board
	void getBoardSensitivityValues(juce::uint8 boardIndex);

	// CMD 3Ch: Set the MIDI channels for peripheral controllers
	void setPeripheralChannels(juce::uint8 pitchWheelChannel, juce::uint8 modWheelChannel, juce::uint8 expressionChannel, juce::uint8 sustainChannel);

	// CMD 3Dh: Retrieve the MIDI channels for peripheral controllers
	void getPeripheralChannels();

	// CMD 3Eh: This command is only used for automatic responses containing calibration data every 100ms while in calibration mode
	// Sending a SysEx message containing this command will result in a NACK response

	// CMD 3Fh: Set the 8-bit aftertouch trigger delay value, the time between a note on event and the initialization of aftertouch events
	void setAfterTouchTriggerDelay(juce::uint8 boardIndex, juce::uint8 aftertouchTriggerValue);
	void setAfterTouchTriggerDelay(juce::uint8 boardIndex, juce::uint8 triggerValueUpperNibble, juce::uint8 triggerValueLowerNibble);

	// CMD 40h: Retrieve the aftertouch trigger delay of the given board
	void sendGetAftertouchTriggerDelayRequest(juce::uint8 boardIndex);

	// CMD 41h: Set the Lumatouch note-off delay value, an 11-bit integer representing the amount of 1.1ms ticks before
	// sending a note-off event after a Lumatone-configured key is released. 
	void setLumatouchNoteOffDelay(juce::uint8 boardIndex, int delayValue);
	void setLumatouchNoteOffDelay(juce::uint8 boardIndex, juce::uint8 valueBits8_11, juce::uint8 valueBits4_7, juce::uint8 valueBits0_3);

	// CMD 42h: Retrieve the note-off delay value of the given board
	void sendGetLumatouchNoteOffDelayRequest(juce::uint8 boardIndex);

	// CMD 43h: Set expression pedal ADC threshold value, a 12-bit integer
	void setExpressionPedalADCThreshold(int thresholdValue);
	// CMD 43h: Set expression pedal ADC threshold value with 3 4-bit integers representing a 12-bit integer
	void setExpressionPedalADCThreshold(juce::uint8 valueBits8_12, juce::uint8 valueBits4_7, juce::uint8 valueBits0_3);

	// CMD 44h: Get the current expression pedal ADC threshold value
	void sendGetExpressionPedalADCThresholdRequest();

	// CMD 45h: Configure the on/off settings of the sustain pedal
	void sendInvertSustainPedal(bool setInverted);

	// CMD 46h: Replace a certain preset with the factory preset
	void sendResetDefaultPresetsRequest(int presetIndex);

	// CMD 47h: Read back the currently configured preset flags of expression & sustain inversion,
	// plus light-on-keystroke and polyphonic aftertouch
	void sendGetPresetFlagsReset();

	// For CMD 48h response: get expression pedal sensitivity
	void sendGetExpressionPedalSensitivity();
	
	// TODO CMD 49h-4Eh

	//============================================================================
	// Implementation of bidirectional communication with acknowledge messages

	// MIDI input callback: handle acknowledge messages
	void handleIncomingMidiMessage(juce::MidiInput* source, const juce::MidiMessage& message) override;

	// Handle timeout
	void timerCallback() override;


	//============================================================================

	// Clear MIDI message buffer
	void clearMIDIMessageBuffer();

private:

	// Send a message now without confirming it's a Lumatone
	void sendTestMessageNow(int outputDeviceIndex, const juce::MidiMessage& message);

	// Low-level SysEx message sending
	void sendMessageWithAcknowledge(const juce::MidiMessage& message);

	// Send the oldest message in queue and start waiting for answer
	void sendOldestMessageInQueue();

	// Send the message marked as current and start waiting for answer
    void sendCurrentMessage();

    // Send a SysEx message with standardized length
	void sendSysEx(juce::uint8 boardIndex, juce::uint8 cmd, juce::uint8 data1, juce::uint8 data2, juce::uint8 data3, juce::uint8 data4, bool overrideEditMode = false);

	// Send a SysEx message without parameters
	void sendSysExRequest(juce::uint8 boardIndex, juce::uint8 cmd);

	// Send a SysEx message to toggle a state
	void sendSysExToggle(juce::uint8 boardIndex, juce::uint8 cmd, bool turnStateOn);

	// Attributes
protected:
	juce::ListenerList<LumatoneFirmwareDriverListener> listeners;

private:

	HostMode hostMode;
	juce::PluginHostType host;

	juce::CriticalSection hostLock;
	juce::MidiBuffer hostQueue;
	int hostQueueSize = 0;

    juce::MidiMessage currentMsgWaitingForAck;
	bool hasMsgWaitingForAck = false;

	juce::Array<juce::MidiMessage, juce::CriticalSection> sysexQueue;

	// Used for device detection and "Offline" mode (no messages that mutate board data)
	bool      onlySendRequestMessages = false;

	const int numBoards = 0;

	const int receiveTimeoutInMilliseconds = 2000;
	const int busyTimeDelayInMilliseconds = 500;
	const int checkQueueTimerDelayInMilliseconds = 10;
	TimerType timerType;

	bool receivedAnswer = false; // Debug helper flag

	int verbose = 0;
};

#endif
