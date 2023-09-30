/*
  ==============================================================================

    firmware_types.h
    Created: 30 Sep 2023
    Author:  Vincenzo

  ==============================================================================
*/

#ifndef LUMATONE_FIRMWARE_TYPES_H
#define LUMATONE_FIRMWARE_TYPES_H

#include <JuceHeader.h>
#include "firmware_definitions.h"

namespace LumatoneFirmware
{

enum class ReleaseVersion
{
	NO_VERSION      = 0,  // Used for instantiation
	UNKNOWN_VERSION = 0,  // Used when no other version applies
	VERSION_55_KEYS = 1,  // Used when GetSerialIdentity returns 00000000
	VERSION_1_0_3 = 0x10,
	VERSION_1_0_4,
	VERSION_1_0_5,
	VERSION_1_0_6,
	VERSION_1_0_7,
	VERSION_1_0_8,
	VERSION_1_0_9,
	VERSION_1_0_10,
	VERSION_1_0_11,
	VERSION_1_0_12,
    VERSION_1_1_0 = VERSION_1_0_12,
    VERSION_1_2_0,
	LAST_VERSION = VERSION_1_2_0,
	FUTURE_VERSION = 0xFFFF  // Used when version is nonnegative and below 9.9.999
};


struct PeripheralChannelSettings
{
	int pitchWheel = 1;
	int modWheel = 1;
	int expressionPedal = 1;
	int sustainPedal = 1;

	void setChannel(PeripheralChannel controlId, int channelIn)
	{
		if (channelIn < 1)
			channelIn = 1;

		if (channelIn > 16)
			channelIn = 16;

		switch (controlId)
		{
		case PeripheralChannel::PitchWheel:
			pitchWheel = channelIn;
			break;

		case PeripheralChannel::ModWheel:
			modWheel = channelIn;
			break;

		case PeripheralChannel::Expression:
			expressionPedal = channelIn;
			break;

		case PeripheralChannel::Sustain:
			sustainPedal = channelIn;
			break;
		}
	}

	int getChannel(PeripheralChannel controlId)
	{
		switch (controlId)
		{
		case PeripheralChannel::PitchWheel:
			return pitchWheel;

		case PeripheralChannel::ModWheel:
			return modWheel;

		case PeripheralChannel::Expression:
			return expressionPedal;

		case PeripheralChannel::Sustain:
			return sustainPedal;
		}

		return 0;
	}
};


struct WheelsCalibrationData
{
	int centerPitch = 0;
	int minPitch = 0;
	int maxPitch = 0;

	int minMod = 0;
	int maxMod = 0;

	float getCentrePitchNorm() const { return (float)centerPitch * ADCSCALAR; }
	float getMinPitchNorm() const { return (float)minPitch * ADCSCALAR; }
	float getMaxPitchNorm() const { return (float)maxPitch * ADCSCALAR; }
	float getMinModNorm() const { return (float)minMod * ADCSCALAR; }
	float getMaxModNorm() const { return (float)maxMod * ADCSCALAR; }

	juce::String toString() const
	{
		juce::String str;
		str += ("Center Pitch: " + juce::String(centerPitch) + juce::newLine);
		str += ("   Min Pitch: " + juce::String(minPitch) + juce::newLine);
		str += ("   Max Pitch: " + juce::String(maxPitch) + juce::newLine);
		str += ("     Min Mod: " + juce::String(minMod) + juce::newLine);
		str += ("     Max Mod: " + juce::String(maxMod) + juce::newLine);
		return str;
	}
};


struct Version
{
	int major = 0;
	int minor = 0;
	int revision = 0;

	Version(int majorVersion=0, int minorVersion=0, int revisionNumber=0)
		: major(majorVersion), minor(minorVersion), revision(revisionNumber) {}

	bool isValid() { return major > 0 || minor > 0 || revision > 0; }

	juce::String toString() const { return juce::String(major) + "." + juce::String(minor) + "." + juce::String(revision); }

	juce::String toDisplayString() const 
	{ 
		juce::String str = juce::String(major) + "." + juce::String(minor);
		if (revision > 0)
			str += ("." + juce::String(revision));

		return str;
	}

	//============================================================================

	static Version fromString(juce::String firmwareVersion)
	{
		Version version(0, 0, 0);

		juce::String afterFirstDecimal = firmwareVersion.fromFirstOccurrenceOf(".", false, true);

		// Just check if it contains at least two decimals
		if (firmwareVersion.contains(".") && afterFirstDecimal.contains("."))
		{
			juce::String majorNum = firmwareVersion.upToFirstOccurrenceOf(".", false, true);

			juce::String minorNum = afterFirstDecimal.upToFirstOccurrenceOf(".", false, true);

			if (minorNum == afterFirstDecimal)
			{
				// This means there was only one decimal, don't try to parse
				return version;
			}

			juce::String revisionNum = firmwareVersion.fromLastOccurrenceOf(".", false, true);
			if (revisionNum != revisionNum.upToFirstOccurrenceOf(".", false, true))
			{
				// This means there's an additional decimal, don't try to parse
				return version;
			}

			version.major = majorNum.getIntValue();
			version.minor = majorNum.getIntValue();
			version.revision = majorNum.getIntValue();
		}

		return version;
	}

	static Version fromReleaseVersion(ReleaseVersion versionIn)
	{
		int versionIndex = (int)versionIn;
		if (versionIn >= ReleaseVersion::VERSION_1_0_3)
		{
			// Return special definition for future version
			if (versionIn > ReleaseVersion::LAST_VERSION)
				return Version(0, 0, (int)ReleaseVersion::FUTURE_VERSION);
			else
				return Version(1, 0, versionIndex - (int)ReleaseVersion::VERSION_1_0_3);
		}
		// Return special definition for 55-keys version
		else if (versionIn == ReleaseVersion::VERSION_55_KEYS)
			return Version(0, 0, 55);
		
		return Version(0, 0, 0);
	}
};

}
#endif
