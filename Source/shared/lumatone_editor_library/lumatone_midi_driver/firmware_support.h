/*
  ==============================================================================

    firmware_support.h
    Created: 30 Sep 2023
    Author:  Vincenzo

  ==============================================================================
*/

#ifndef LUMATONE_FIRMWARE_SUPPORT_H
#define LUMATONE_FIRMWARE_SUPPORT_H

#include "./firmware_types.h"

struct FirmwareSupport
{
	enum class Error
	{
		noError = 0,
		noMidiOutputSet,
		deviceIsBusy,
		messageTooShort,
		messageTooLong,
        messageIsAnEcho,
		messageHasIncorrectManufacturerId,
		messageHasInvalidBoardIndex,
		messageHasInvalidStatusByte,
		messageIsNotResponseToCommand,
		messageIsNotSysEx,
		unknownCommand,
		externalError,
        commandNotImplemented
	};

	static juce::String errorToString(Error err)
	{
		switch (err)
		{
		case Error::noError:
			return "No error";
		case Error::noMidiOutputSet:
			return "No Midi output set";
		case Error::deviceIsBusy:
			return "Device is busy";
		case Error::messageTooShort:
            return "Message too short";
		case Error::messageTooLong:
            return "Message too long";
        case Error::messageIsAnEcho:
            return "Message is an echo";
		case Error::messageHasIncorrectManufacturerId:
			return "Incorrect manufacturer ID";
		case Error::messageHasInvalidBoardIndex:
			return "Invalid board index";
		case Error::messageHasInvalidStatusByte:
			return "Message has invalid status byte";
		case Error::messageIsNotResponseToCommand:
			return "Message is not a response to the command";
		case Error::messageIsNotSysEx:
			return "Message is not a valid SysEx message.";
		case Error::unknownCommand:
			return "Unknown command / Not Acknowledged";
		case Error::externalError:
			return "Error from device";
        case Error::commandNotImplemented:
            return "Command handling not implemented";
		default:
			return "Undefined error..";
		}
	}

	static LumatoneFirmware::ReleaseVersion getReleaseVersion(LumatoneFirmware::Version versionIn)
	{
		if (!(versionIn.major | versionIn.minor | versionIn.revision))
			return LumatoneFirmware::ReleaseVersion::VERSION_55_KEYS;

		else if ((versionIn.major < 0) | (versionIn.minor < 0) | (versionIn.revision < 0))
			return LumatoneFirmware::ReleaseVersion::UNKNOWN_VERSION;
		
        // MAJOR: 1
		else if (versionIn.major == 1)
		{
            // MINOR: 0
			if (versionIn.minor == 0)
			{
				if (versionIn.revision < 3)
					return LumatoneFirmware::ReleaseVersion::VERSION_55_KEYS;

                // Computing is probably not the best thing to do but edge cases are extremely unlikely here
				else if (versionIn.revision - 3 > (int)LumatoneFirmware::ReleaseVersion::LAST_VERSION - (int)LumatoneFirmware::ReleaseVersion::VERSION_1_0_3)
					return LumatoneFirmware::ReleaseVersion::FUTURE_VERSION;

				else if (versionIn.revision >= 3)
					return (LumatoneFirmware::ReleaseVersion)((int)LumatoneFirmware::ReleaseVersion::VERSION_1_0_3 + (versionIn.revision - 3));
			}
            
            // MINOR: 1
            else if (versionIn.minor == 1)
            {
                if (versionIn.revision == 0)
                    return LumatoneFirmware::ReleaseVersion::VERSION_1_1_0;
            }
            
            else if (versionIn.minor == 2)
            {
                if (versionIn.revision == 0)
                    return LumatoneFirmware::ReleaseVersion::VERSION_1_2_0;
            }
            
            return LumatoneFirmware::ReleaseVersion::FUTURE_VERSION;
		}

        // Unsure if this is needed, or if returning FUTURE_VERSION without a condition is better
		else if (versionIn.major < 9 && versionIn.minor < 9 && versionIn.revision < 999)
			return LumatoneFirmware::ReleaseVersion::FUTURE_VERSION;

		return LumatoneFirmware::ReleaseVersion::UNKNOWN_VERSION;
	}

	// TODO use map instead
	// Returns the lowest version that will return ACK for a given command
	static LumatoneFirmware::ReleaseVersion getLowestVersionAcknowledged(unsigned int CMD)
	{
		if (CMD < CHANGE_KEY_NOTE) // 0x00
			return LumatoneFirmware::ReleaseVersion::UNKNOWN_VERSION;

		else if (CMD <= GET_SERIAL_IDENTITY) // 0x23
			return LumatoneFirmware::ReleaseVersion::NO_VERSION;

		else if (CMD <= DEMO_MODE) // 0x25
			return LumatoneFirmware::ReleaseVersion::VERSION_1_0_5;

		else if (CMD <= CALIBRATE_PITCH_MOD_WHEEL) // 0x26
			return LumatoneFirmware::ReleaseVersion::VERSION_1_0_6;

		else if (CMD <= SET_KEY_MAX_THRESHOLD) // 0x29
			return LumatoneFirmware::ReleaseVersion::VERSION_1_0_7;

		else if (CMD <= GET_FIRMWARE_REVISION) // 0x31
			return LumatoneFirmware::ReleaseVersion::VERSION_1_0_8;

		else if (CMD <= LUMA_PING) // 0x33
			return LumatoneFirmware::ReleaseVersion::VERSION_1_0_9;

		else if (CMD <= SET_KEY_SAMPLING) // 0x35
			return LumatoneFirmware::ReleaseVersion::VERSION_1_0_10;

		else if (CMD <= RESET_EXPRESSION_PEDAL_BOUNDS) // 0x39
			return LumatoneFirmware::ReleaseVersion::VERSION_1_0_11;

		else if (CMD <= GET_EXPRESSION_PEDAL_BOUNDS) // 0x4F
			return LumatoneFirmware::ReleaseVersion::VERSION_1_1_0;

		else
			return LumatoneFirmware::ReleaseVersion::FUTURE_VERSION;
	}

	static bool versionAcknowledgesCommand(LumatoneFirmware::ReleaseVersion VERSION, unsigned int CMD)
	{
		return getLowestVersionAcknowledged(CMD) <= VERSION;
	}

	static bool versionAcknowledgesCommand(LumatoneFirmware::Version versionIn, unsigned int CMD)
	{
		auto VERSION = getReleaseVersion(versionIn);
		return versionAcknowledgesCommand(VERSION, CMD);
	}

	// Should always be 56 in production
	static int getOctaveSize(LumatoneFirmware::ReleaseVersion VERSION)
	{
		if (VERSION >= LumatoneFirmware::ReleaseVersion::VERSION_1_0_3)
			return 56;
		else
			return 55;
	}

	static int getCommandNumber(const juce::MidiMessage& msg)
	{
		return msg.getSysExData()[CMD_ID];
	}
    
    static juce::String serialIdentityToString(const int* serialBytes)
    {
        return juce::String::toHexString(serialBytes, 6);
    }

	static juce::String getCommandDescription(const juce::MidiMessage& msg)
	{
		// TODO unpack values?
		juce::String name;
		juce::String description;

		const juce::uint8* data = msg.getRawData();
		juce::uint8 board = static_cast<juce::uint8>(data[BOARD_IND]);

		switch (getCommandNumber(msg))
		{
		case CHANGE_KEY_NOTE:
			name = "Set Key Params";
			description = "Board " + juce::String(board) + " Key " + juce::String(data[5]);
			break;
		case SET_KEY_COLOUR:
			name = "Set Key Color";
			description = "Board " + juce::String(board) + " Key " + juce::String(data[5]);
			for (int i = 6; i < msg.getRawDataSize(); i++)
				description += juce::String(data[i]) + "|";
			break;
		case SAVE_PROGRAM:
			name = "SAVE_PROGRAM";
			description = "";
			break;
		case SET_FOOT_CONTROLLER_SENSITIVITY:
			name = "SET_FOOT_CONTROLLER_SENSITIVITY";
			description = "";
			break;
		case INVERT_FOOT_CONTROLLER:
			name = "INVERT_FOOT_CONTROLLER";
			description = "";
			break;
		case MACROBUTTON_COLOUR_ON:
			name = "MACROBUTTON_COLOUR_ON";
			description = "";
			break;
		case MACROBUTTON_COLOUR_OFF:
			name = "MACROBUTTON_COLOUR_OFF";
			description = "";
			break;
		case SET_LIGHT_ON_KEYSTROKES:
			name = "SET_LIGHT_ON_KEYSTROKES";
			description = "";
			break;
		case SET_VELOCITY_CONFIG:
			name = "SET_VELOCITY_CONFIG";
			description = "";
			break;
		case SAVE_VELOCITY_CONFIG:
			name = "SAVE_VELOCITY_CONFIG";
			description = "";
			break;
		case RESET_VELOCITY_CONFIG:
			name = "RESET_VELOCITY_CONFIG";
			description = "";
			break;
		case SET_FADER_CONFIG:
			name = "SET_FADER_CONFIG";
			description = "";
			break;
		case SAVE_FADER_CONFIG:
			name = "SAVE_FADER_CONFIG";
			description = "";
			break;
		case RESET_FADER_CONFIG:
			name = "RESET_FADER_CONFIG";
			description = "";
			break;
		case SET_AFTERTOUCH_FLAG:
			name = "SET_AFTERTOUCH_FLAG";
			description = "";
			break;
		case CALIBRATE_AFTERTOUCH:
			name = "CALIBRATE_AFTERTOUCH";
			description = "";
			break;
		case SET_AFTERTOUCH_CONFIG:
			name = "SET_AFTERTOUCH_CONFIG";
			description = "";
			break;
		case SAVE_AFTERTOUCH_CONFIG:
			name = "SAVE_AFTERTOUCH_CONFIG";
			description = "";
			break;
		case RESET_AFTERTOUCH_CONFIG:
			name = "RESET_AFTERTOUCH_CONFIG";
			description = "";
			break;
		case GET_RED_LED_CONFIG:
			name = "GET_RED_LED_CONFIG";
			description = "";
			break;
		case GET_GREEN_LED_CONFIG:
			name = "GET_GREEN_LED_CONFIG";
			description = "";
			break;
		case GET_BLUE_LED_CONFIG:
			name = "GET_BLUE_LED_CONFIG";
			description = "";
			break;
		case GET_CHANNEL_CONFIG:
			name = "GET_CHANNEL_CONFIG";
			description = "";
			break;
		case GET_NOTE_CONFIG:
			name = "GET_NOTE_CONFIG";
			description = "";
			break;
		case GET_KEYTYPE_CONFIG:
			name = "GET_KEYTYPE_CONFIG";
			description = "";
			break;
		case GET_MAX_THRESHOLD:
			name = "GET_MAX_THRESHOLD";
			description = "";
			break;
		case GET_MIN_THRESHOLD:
			name = "GET_MIN_THRESHOLD";
			description = "";
			break;
		case GET_AFTERTOUCH_MAX:
			name = "GET_AFTERTOUCH_MAX";
			description = "";
			break;
		case GET_KEY_VALIDITY:
			name = "GET_KEY_VALIDITY";
			description = "";
			break;
		case GET_VELOCITY_CONFIG:
			name = "GET_VELOCITY_CONFIG";
			description = "";
			break;
		case GET_FADER_CONFIG:
			name = "GET_FADER_CONFIG";
			description = "";
			break;
		case GET_AFTERTOUCH_CONFIG:
			name = "GET_AFTERTOUCH_CONFIG";
			description = "";
			break;
		case SET_VELOCITY_INTERVALS:
			name = "SET_VELOCITY_INTERVALS";
			description = "";
			break;
		case GET_VELOCITY_INTERVALS:
			name = "GET_VELOCITY_INTERVALS";
			description = "";
			break;
		case GET_FADER_TYPE_CONFIGURATION:
			name = "GET_FADER_TYPE_CONFIGURATION";
			description = "";
			break;
		case GET_SERIAL_IDENTITY:
			name = "GET_SERIAL_IDENTITY";
			description = "";
			break;
		case CALIBRATE_KEYS:
			name = "CALIBRATE_KEYS";
			description = "";
			break;
		case DEMO_MODE:
			name = "DEMO_MODE";
			description = "";
			break;
		case CALIBRATE_PITCH_MOD_WHEEL:
			name = "CALIBRATE_PITCH_MOD_WHEEL";
			description = "";
			break;
		case SET_MOD_WHEEL_SENSITIVITY:
			name = "SET_MOD_WHEEL_SENSITIVITY";
			description = "";
			break;
		case SET_PITCH_WHEEL_SENSITIVITY:
			name = "SET_PITCH_WHEEL_SENSITIVITY";
			description = "";
			break;
		case SET_KEY_MAX_THRESHOLD:
			name = "SET_KEY_MAX_THRESHOLD";
			description = "";
			break;
		case SET_KEY_MIN_THRESHOLD:
			name = "SET_KEY_MIN_THRESHOLD";
			description = "";
			break;
		case SET_KEY_FADER_SENSITIVITY:
			name = "SET_KEY_FADER_SENSITIVITY";
			description = "";
			break;
		case SET_KEY_AFTERTOUCH_SENSITIVITY:
			name = "SET_KEY_AFTERTOUCH_SENSITIVITY";
			description = "";
			break;
		case SET_LUMATOUCH_CONFIG:
			name = "SET_LUMATOUCH_CONFIG";
			description = "";
			break;
		case SAVE_LUMATOUCH_CONFIG:
			name = "SAVE_LUMATOUCH_CONFIG";
			description = "";
			break;
		case RESET_LUMATOUCH_CONFIG:
			name = "RESET_LUMATOUCH_CONFIG";
			description = "";
			break;
		case GET_LUMATOUCH_CONFIG:
			name = "GET_LUMATOUCH_CONFIG";
			description = "";
			break;
		case GET_FIRMWARE_REVISION:
			name = "GET_FIRMWARE_REVISION";
			description = "";
			break;
		case SET_CC_ACTIVE_THRESHOLD:
			name = "SET_CC_ACTIVE_THRESHOLD";
			description = "";
			break;
		case LUMA_PING:
			name = "LUMA_PING";
			description = "";
			break;
		case RESET_BOARD_THRESHOLDS:
			name = "RESET_BOARD_THRESHOLDS";
			description = "";
			break;
		case SET_KEY_SAMPLING:
			name = "SET_KEY_SAMPLING";
			description = "";
			break;
		case RESET_WHEELS_THRESHOLD:
			name = "RESET_WHEELS_THRESHOLD";
			description = "";
			break;
		case SET_PITCH_WHEEL_CENTER_THRESHOLD:
			name = "SET_PITCH_WHEEL_CENTER_THRESHOLD";
			description = "";
			break;
		case CALLIBRATE_EXPRESSION_PEDAL:
			name = "CALLIBRATE_EXPRESSION_PEDAL";
			description = "";
			break;
		case RESET_EXPRESSION_PEDAL_BOUNDS:
			name = "RESET_EXPRESSION_PEDAL_BOUNDS";
			description = "";
			break;
		case GET_BOARD_THRESHOLD_VALUES:
			name = "GET_BOARD_THRESHOLD_VALUES";
			description = "";
			break;
		case GET_BOARD_SENSITIVITY_VALUES:
			name = "GET_BOARD_SENSITIVITY_VALUES";
			description = "";
			break;
		case SET_PERIPHERAL_CHANNELS:
			name = "SET_PERIPHERAL_CHANNELS";
			description = "";
			break;
		case GET_PERIPHERAL_CHANNELS:
			name = "GET_PERIPHERAL_CHANNELS";
			description = "";
			break;
		case PERIPHERAL_CALBRATION_DATA:
			name = "PERIPHERAL_CALBRATION_DATA";
			description = "";
			break;
		case SET_AFTERTOUCH_TRIGGER_DELAY:
			name = "SET_AFTERTOUCH_TRIGGER_DELAY";
			description = "";
			break;
		case GET_AFTERTOUCH_TRIGGER_DELAY:
			name = "GET_AFTERTOUCH_TRIGGER_DELAY";
			description = "";
			break;
		case SET_LUMATOUCH_NOTE_OFF_DELAY:
			name = "SET_LUMATOUCH_NOTE_OFF_DELAY";
			description = "";
			break;
		case GET_LUMATOUCH_NOTE_OFF_DELAY:
			name = "GET_LUMATOUCH_NOTE_OFF_DELAY";
			description = "";
			break;
		case SET_EXPRESSION_PEDAL_THRESHOLD:
			name = "SET_EXPRESSION_PEDAL_THRESHOLD";
			description = "";
			break;
		case GET_EXPRESSION_PEDAL_THRESHOLD:
			name = "GET_EXPRESSION_PEDAL_THRESHOLD";
			description = "";
			break;
		case INVERT_SUSTAIN_PEDAL:
			name = "INVERT_SUSTAIN_PEDAL";
			description = "";
			break;
		case RESET_DEFAULT_PRESETS:
			name = "RESET_DEFAULT_PRESETS";
			description = "";
			break;
		case GET_PRESET_FLAGS:
			name = "GET_PRESET_FLAGS";
			description = "";
			break;
		case GET_EXPRESSION_PEDAL_SENSITIVIY:
			name = "GET_EXPRESSION_PEDAL_SENSITIVIY";
			description = "";
			break;
		case GET_MACRO_LIGHT_INTENSITY:
			name = "GET_MACRO_LIGHT_INTENSITY";
			description = "";
			break;
		case RESET_MACRO_LIGHT_INTENSITY:
			name = "RESET_MACRO_LIGHT_INTENSITY";
			description = "";
			break;
		case RESET_BOARD_KEYS:
			name = "RESET_BOARD_KEYS";
			description = "";
			break;
		case RESET_AFTERTOUCH_TRIGGER_DELAY:
			name = "RESET_AFTERTOUCH_TRIGGER_DELAY";
			description = "";
			break;
		case RESET_LUMATOUCH_NOTE_OFF_DELAY:
			name = "RESET_LUMATOUCH_NOTE_OFF_DELAY";
			description = "";
			break;
		case GET_PITCH_AND_MOD_BOUNDS:
			name = "GET_PITCH_AND_MOD_BOUNDS";
			description = "";
			break;
		case GET_EXPRESSION_PEDAL_BOUNDS:
			name = "GET_EXPRESSION_PEDAL_BOUNDS";
			description = "";
			break;
		default:
			name = "ERROR_UNKNOWN_COMMAND";
			jassertfalse;
			break;
		}

		return name + ": " + description;
	}

};

#endif
