#ifndef LUMATONE_EDITOR_FIRMWARE_LISTENER_H
#define LUMATONE_EDITOR_FIRMWARE_LISTENER_H

#include <JuceHeader.h>
#include "../lumatone_midi_driver/firmware_definitions.h"

namespace LumatoneEditor
{
//============================================================================
// Public interface for Lumatone firmware communication
class FirmwareListener
{
public:
    
    virtual ~FirmwareListener() {}

    // rgbFlag uses 0 for red, 1 for green, 2 for blue
    virtual void octaveColourConfigReceived(int octaveIndex, juce::uint8 rgbFlag, const int* colourData) {};

    virtual void octaveChannelConfigReceived(int octaveIndex, const int* channelData) {};

    virtual void octaveNoteConfigReceived(int octaveIndex, const int* noteData) {};

    virtual void keyTypeConfigReceived(int octaveIndex, const int* keyTypeData) {};

    virtual void velocityConfigReceived(const int* velocityData) {};

    virtual void aftertouchConfigReceived(const int* aftertouch) {};

    virtual void velocityIntervalConfigReceived(const int* velocityData) {};

    virtual void faderConfigReceived(const int* faderData) {};

    virtual void faderTypeConfigReceived(int octaveIndex, const int* faderTypeData) {};

    virtual void serialIdentityReceived(const int* serialBytes) {};
    
    virtual void calibratePitchModWheelAnswer(LumatoneFirmware::ReturnCode code) {};

    virtual void lumatouchConfigReceived(const int* lumatouchData) {};

    virtual void firmwareRevisionReceived(LumatoneFirmware::Version version) {};

    virtual void pingResponseReceived(unsigned int pingValue) {};

    virtual void peripheralMidiChannelsReceived(LumatoneFirmware::PeripheralChannelSettings channelSettings) {};

    virtual void pedalCalibrationDataReceived(int minBound, int maxBound, bool pedalIsActive) {};

    virtual void wheelsCalibrationDataReceived(LumatoneFirmware::WheelsCalibrationData calibrationData) {};

    virtual void presetFlagsReceived(LumatoneFirmware::PresetFlags presetFlags) {};

    virtual void expressionPedalSensitivityReceived(int sensitivity) {};

    virtual void noAnswerToCommand(int cmd) {};
};

}

#endif
