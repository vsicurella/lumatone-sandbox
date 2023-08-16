/*
  ==============================================================================

    ApplicationListeners.h
    Created: 22 Mar 2022 10:11:10pm
    Author:  Vincenzo Sicurella
 
    Various listener and enums that should be shared throughout the app

  ==============================================================================
*/

#pragma once
#include  <JuceHeader.h>

#include "./data/lumatone_layout.h"
#include "./data/lumatone_state.h"

namespace LumatoneEditor
{
    //============================================================================
    // Public interface for Lumatone connection status

    class StatusListener
    {
    public:

        virtual ~StatusListener() {}

        virtual void connectionFailed() {}
        virtual void connectionEstablished(juce::String inputDeviceId, juce::String outputDeviceId) {}
        virtual void connectionLost() {}
    };

    //============================================================================
    // Interface for components that declare Lumatone connection changes

    class StatusEmitter
    {
    protected:
        juce::ListenerList<StatusListener> statusListeners;

    public:

        virtual ~StatusEmitter() {}

        void addStatusListener(StatusListener* listener) { statusListeners.add(listener); }

        void removeStatusListener(StatusListener* listener) { statusListeners.remove(listener); }
    };

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
        
        virtual void calibratePitchModWheelAnswer(TerpstraMIDIAnswerReturnCode code) {};

        virtual void lumatouchConfigReceived(const int* lumatouchData) {};

        virtual void firmwareRevisionReceived(FirmwareVersion version) {};

        virtual void pingResponseReceived(unsigned int pingValue) {};

        virtual void peripheralMidiChannelsReceived(PeripheralChannelSettings channelSettings) {};

        virtual void pedalCalibrationDataReceived(int minBound, int maxBound, bool pedalIsActive) {};

        virtual void wheelsCalibrationDataReceived(WheelsCalibrationData calibrationData) {};

        virtual void presetFlagsReceived(PresetFlags presetFlags) {};

        virtual void expressionPedalSensitivityReceived(int sensitivity) {};

        virtual void noAnswerToCommand(int cmd) {};
    };


    class EditorListener
    {
    public:

        virtual ~EditorListener() {}

        // TODO - change this to "status" and use "disconnected", "offline", "live", "firmware"
        // Editor Actions
        //virtual void editorModeChanged(sysExSendingMode newEditorMode) {}
        virtual void connectionStatusChanged(ConnectionState status) {}
        
        // App Actions
        virtual void completeMappingLoaded(LumatoneLayout mappingData) {};
        virtual void boardChanged(LumatoneBoard boardData) {};
        virtual void keyChanged(int boardIndex, int keyIndex, LumatoneKey lumatoneKey) {};

        virtual void tableChanged(LumatoneConfigTable::TableType type, const juce::uint8* table, int tableSize) {};

        // Firmware Actions
        virtual void keyConfigChanged(int boardIndex, int keyIndex, LumatoneKey keyData) {};
        virtual void keyColourChanged(int octaveNumber, int keyNumber, juce::Colour keyColour) {};
        virtual void expressionPedalSensitivityChanged(unsigned char value) {};
        virtual void invertFootControllerChanged(bool inverted) {};
        virtual void macroButtonActiveColourChagned(juce::Colour colour) {};
        virtual void macroButtonInactiveColourChanged(juce::Colour colour) {};
        virtual void lightOnKeyStrokesChanged(bool lightOn) {};


        virtual void velocityConfigSaved() {};
        virtual void velocityConfigReset() {};

        virtual void aftertouchToggled(bool enabled) {};
        virtual void calibrateAftertouchToggled(bool active) {};

        virtual void aftertouchConfigReset() {};

        virtual void serialIdentityRequested() {};
        virtual void calibrateKeysRequested() {};
        virtual void calibratePitchModWheelToggled(bool active) {};

        virtual void lumatouchConfigReset() {};

        virtual void firmwareVersionRequested() {};
        virtual void pingSent(juce::uint8 pingId) {};

        virtual void peripheralChannelsChanged(int pitchWheelChannel, int modWheelChannel, int expressionChannel, int sustainChannel) {};
        virtual void invertSustainToggled(bool inverted) {};

     };


    class MidiListener
    {
    public:
    
        virtual ~MidiListener() {}
    
        virtual void handleMidiMessage(const juce::MidiMessage& msg) = 0;
    };
}
