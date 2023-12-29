/*
  ==============================================================================

    activity_monitor.h
    Created: 11 Feb 2021 9:15:43pm
    Author:  Vincenzo

    Finds a pair of devices connected to Lumatone, and monitors activity 
    to detect if the connection to the device is lost. When a change is detected
    a change signal will be broadcasted (for LumatoneController).
 
    When looking for devices, a response timeout is required, but once a device
    has been confirmed, the timing and callbacks are dependent on LumatoneFirmwareDriver.

  ==============================================================================
*/

#ifndef LUMATONE_ACTIVITY_MONITOR_H
#define LUMATONE_ACTIVITY_MONITOR_H

#include "../lumatone_midi_driver/firmware_driver_listener.h"

#include "../data/application_state.h"
#include "../listeners/status_listener.h"

class LumatoneFirmwareDriver;

class DeviceActivityMonitor : protected LumatoneApplicationState,
                              public juce::Timer, 
                              public LumatoneEditor::StatusEmitter,
                              protected LumatoneFirmwareDriverListener
{
    
public:
    enum class DetectConnectionMode
    {
        idle = -1,
        lookingForDevice,
        noDeviceMonitoring,
        waitingForInactivity
    };
    
public:

    DeviceActivityMonitor(LumatoneFirmwareDriver* midiDriverIn, LumatoneApplicationState stateIn);
    ~DeviceActivityMonitor() override;

    DetectConnectionMode getMode() const { return deviceConnectionMode; }
    bool isConnectionEstablished() const { return confirmedInputIndex >= 0 && confirmedOutputIndex >= 0; }
    
    bool willDetectDeviceIfDisconnected() const { return detectDevicesIfDisconnected; }
    void setDetectDeviceIfDisconnected(bool doDetection);
    
    bool willCheckForInactivity() const { return checkConnectionOnInactivity; }
    void setCheckForInactivity(bool monitorActivity);

    int getConfirmedOutputIndex() const { return confirmedOutputIndex; }
    int getConfirmedInputIndex() const { return confirmedInputIndex; }

    int getResponseTimeoutMs() { return responseTimeoutMs; }
    void setResponseTimeoutMs(int timeoutMs) { responseTimeoutMs = timeoutMs; }

    //=========================================================================

    // Start monitoring available MIDI devices and wait for an expected response
    // First uses the "Ping" command to send to all available devices,
    // then goes through a legacy-supported routine with individual output devices.
    void startDeviceDetection();

    // Begin polling selected device until it stops responding. Other messages
    // will reset the inactivity timer.
    void startActivityMonitoring();

    // Turn off device monitoring and idle
    void stopMonitoringDevice();
    
private:

    //=========================================================================
    // juce::Timer Implementation

    void timerCallback() override;


    //=========================================================================

    /// <summary>
    /// Sends a ping to all devices to see if we get a response
    /// </summary>
    void pingAllDevices();

    /// <summary>
    /// If a MIDI device identifier is defined in properties, send a GetSerialIdentity to it.
    /// Returns whether or not a test response was sent
    /// </summary>
    // bool testLastConnectedDevice();

    /// <summary>
    /// Resets device list and starts routine for sending tests to each device individually
    /// </summary>
    void startIndividualDetection();

    /// <summary>
    /// Checks detection routines, will move on from Ping routine if no answers, then will drive the legacy detection routine
    /// </summary>
    void checkDetectionStatus();

    /// <summary>
    /// Increments the output index and sends a Get Serial Identity message to this next output to listen for a response 
    /// </summary>
    void testNextOutput();

    // Turn off device detection and idle
    void stopDeviceDetection();

    /// <summary>
    /// Test device connectivity after a connection has previously been made.
    /// </summary>
    /// <returns>Returns false if devices are not valid, and true if it an attempt to connect was made</returns>
    bool initializeConnectionTest();

    /// <summary>
    /// Handle a response from a test or confirmed juce::MidiInput device
    /// </summary>
    /// <param name="testInputIndex"></param>
    /// <param name="midiMessage"></param>
    void handleResponse(int inputDeviceIndex, const juce::MidiMessage& midiMessage);


private:
    //=========================================================================
    // Callback functions

    void removeFailedPingDevice(const juce::MidiMessage& msg);

    void establishConnection(int inputIndex, int outputIndex);
    void onDisconnection();

    static int getPingIdFromResponse(const juce::MidiMessage& msg);
    
protected:

    //=========================================================================
    // LumatoneFirmwareDriver::Listener Implementation

    void midiMessageReceived(juce::MidiInput* source, const juce::MidiMessage& midiMessage) override;
    void noAnswerToMessage(juce::MidiDeviceInfo expectedDevice, const juce::MidiMessage& midiMessage) override;
    void midiSendQueueSize(int queueSizeIn) override { sentQueueSize = queueSizeIn; }
    
    void midiMessageSent(juce::MidiOutput*, const juce::MidiMessage&) override {}

private:

    LumatoneFirmwareDriver*     midiDriver;

    DetectConnectionMode    deviceConnectionMode   = DetectConnectionMode::idle;
    bool                    deviceDetectInProgress = false;
    bool                    waitingForResponse = false;

    int                     threadDelayMs = 50;
    int                     responseTimeoutMs = 600;
    int                     detectRoutineTimeoutMs = 1000;
    int                     inactivityTimeoutMs  = 3000;

    int                     sentQueueSize = 0;

    int                                 testOutputIndex = -1;
    juce::Array<juce::MidiDeviceInfo>   outputDevices;
    juce::Array<juce::MidiDeviceInfo>   inputDevices;
    juce::Array<unsigned int>           outputPingIds;

    int                     confirmedInputIndex = -1;
    int                     confirmedOutputIndex = -1;

    bool                    detectDevicesIfDisconnected = true;
    bool                    checkConnectionOnInactivity = true;
    
    bool                    sendCalibratePitchModOff = false;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DeviceActivityMonitor)
};

#endif LUMATONE_ACTIVITY_MONITOR_H
