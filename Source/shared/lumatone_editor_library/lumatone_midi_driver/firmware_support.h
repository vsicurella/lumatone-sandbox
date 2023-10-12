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

};

#endif
