/*
  ==============================================================================

    DeviceActivityMonitor.cpp
    Created: 11 Feb 2021 9:15:43pm
    Author:  Vincenzo

  ==============================================================================
*/

#include "DeviceActivityMonitor.h"

#include "./lumatone_midi_driver/lumatone_midi_driver.h"
#include "./lumatone_midi_driver/firmware_sysex.h"

DeviceActivityMonitor::DeviceActivityMonitor(LumatoneFirmwareDriver* midiDriverIn, LumatoneApplicationState stateIn)
    :   LumatoneApplicationState("DeviceActivityMonitor", stateIn)
    ,   midiDriver(midiDriverIn)
{
    detectDevicesIfDisconnected = getBoolProperty(LumatoneApplicationProperty::DetectDeviceIfDisconnected, true);
    checkConnectionOnInactivity = getBoolProperty(LumatoneApplicationProperty::CheckConnectionIfInactive, true);
    responseTimeoutMs = getIntProperty(LumatoneApplicationProperty::DetectDevicesTimeout, detectRoutineTimeoutMs);

    midiDriver->addDriverListener(this);

    // avoid resizing during communication
    ensureStorageAllocated(2000);
}

DeviceActivityMonitor::~DeviceActivityMonitor()
{
    midiDriver->removeDriverListener(this);
}

void DeviceActivityMonitor::setDetectDeviceIfDisconnected(bool doDetection)
{
    detectDevicesIfDisconnected = doDetection;
    writeBoolProperty(LumatoneApplicationProperty::DetectDeviceIfDisconnected, detectDevicesIfDisconnected);

    if (!detectDevicesIfDisconnected)
    {
        deviceConnectionMode = DetectConnectionMode::idle;
        midiDriver->closeTestingDevices();
    }
    else
    {
        startTimer(detectRoutineTimeoutMs);
    }
}

void DeviceActivityMonitor::setCheckForInactivity(bool monitorActivity)
{
    checkConnectionOnInactivity = monitorActivity;
    writeBoolProperty(LumatoneApplicationProperty::CheckConnectionIfInactive, checkConnectionOnInactivity);
    
    if (checkConnectionOnInactivity && isConnectionEstablished())
    {
        startTimer(inactivityTimeoutMs);
    }
}

void DeviceActivityMonitor::pingAllDevices()
{
    DBG("Ping all devices!");

    deviceDetectInProgress = true;

    outputDevices = midiDriver->getMidiOutputList();

    outputPingIds.clear();

    waitingForResponse = true;

    int maxDevices = juce::jmin(outputDevices.size(), 128);
    if (midiDriver->getHostMode() == LumatoneFirmwareDriver::HostMode::Plugin)
        maxDevices = 1;

    for (int i = 0; i < maxDevices; i++)
    {
        unsigned int id = (unsigned int)(i + 1);
        midiDriver->ping(id, i);
        outputPingIds.add(id);
    }

    startTimer(responseTimeoutMs);
}

// bool DeviceActivityMonitor::testLastConnectedDevice()
// {
//     juce::String inputId = getStringProperty(LumatoneApplicationProperty::LastInputDeviceId);
//     if (inputId.length() <= 0)
//         return false;

//     int inputIndex = midiDriver->findIndexOfInputDevice(inputId);
//     if (inputIndex >= 0)
//     {
//         juce::String outputId = getStringProperty(LumatoneApplicationProperty::LastOutputDeviceId);
//         if (outputId.length() <= 0)
//             return false;
        
//         int outputIndex = midiDriver->findIndexOfOutputDevice(outputId);
//         if (outputIndex >= 0)
//         {
//             deviceDetectInProgress = true;
//             waitingForResponse = true;

//             DBG("Testing last connected device");
//             midiDriver->setMidiInput(inputIndex);
//             midiDriver->setMidiOutput(outputIndex);
//             midiDriver->sendGetSerialIdentityRequest();
//             return true;
//         }
//     }

//     return false;
// }

void DeviceActivityMonitor::startIndividualDetection()
{
    jassert(outputPingIds.size() == 0);
    
    testOutputIndex = -1;
    outputDevices = midiDriver->getMidiOutputList();
    inputDevices = midiDriver->getMidiInputList();

    if (outputDevices.size() > 0 && inputDevices.size() > 0)
    {
        deviceDetectInProgress = true;
        testNextOutput();
    }
    else
    {
        DBG("No input and output MIDI device combination could be made.");
        deviceDetectInProgress = false;
        startTimer(detectRoutineTimeoutMs);
    }
}

void DeviceActivityMonitor::testNextOutput()
{

    if (midiDriver->getHostMode() == LumatoneFirmwareDriver::HostMode::Driver)
    {
        testOutputIndex++;
        if (testOutputIndex < 0 || testOutputIndex >= outputDevices.size())
        {
            // Start over
            startTimer(10);
        }
    }

    #if JUCE_DEBUG
        juce::String deviceName = midiDriver->getHostMode() == LumatoneFirmwareDriver::HostMode::Driver ? outputDevices[testOutputIndex].name : "Host";
        juce::String command = sendCalibratePitchModOff ? "Calibrate" : "GetSerial";
        DBG("Test via " + command + ": " + deviceName);
    #endif

    if (sendCalibratePitchModOff)
    {
        midiDriver->sendCalibratePitchModWheel(false, testOutputIndex);
    }
    else
    {
        midiDriver->sendGetSerialIdentityRequest(testOutputIndex);
    }
    
    waitingForResponse = true;
    startTimer(responseTimeoutMs);
}

void DeviceActivityMonitor::startDeviceDetection()
{
    // if (midiDriver->getHostMode() != LumatoneFirmwareDriver::HostMode::Driver)
    //     return; // Only allow in Driver mode
        
    // Belongs somewhere else?
    if (!midiDriver->hasDevicesDefined())
    {
        confirmedInputIndex = -1;
        confirmedOutputIndex = -1;
    }

    if (detectDevicesIfDisconnected)
    {
        deviceConnectionMode = DetectConnectionMode::lookingForDevice;
        deviceDetectInProgress = false; // Don't start until first test response is sent
        startTimer(10);
    }
}

void DeviceActivityMonitor::startActivityMonitoring()
{
    if (midiDriver->getHostMode() != LumatoneFirmwareDriver::HostMode::Driver)
        return; // Only allow in Driver mode
        
    deviceConnectionMode = DetectConnectionMode::waitingForInactivity;
    startTimer(inactivityTimeoutMs);
}

void DeviceActivityMonitor::stopDeviceDetection()
{
    deviceConnectionMode = DetectConnectionMode::idle;
    deviceDetectInProgress = false;
}

void DeviceActivityMonitor::stopMonitoringDevice()
{
    stopTimer();
    waitingForResponse = false;
    deviceConnectionMode = DetectConnectionMode::noDeviceMonitoring;
}

bool DeviceActivityMonitor::initializeConnectionTest()
{    
    bool isIdle = !midiDriver->isWaitingForResponse();
    if (isIdle)
    {
        if (getSerialNumber().isNotEmpty() && getLumatoneVersion() >= LumatoneFirmware::ReleaseVersion::VERSION_1_0_9)
        {
            midiDriver->ping(0xf);
        }

        else
        {
            midiDriver->sendGetSerialIdentityRequest();
        }

        waitingForResponse = true;
    }

    startTimer(inactivityTimeoutMs);
    return isIdle;
}

int DeviceActivityMonitor::getPingIdFromResponse(const juce::MidiMessage &msg)
{
    unsigned int pingId = 0;
    
    auto errorCode = LumatoneSysEx::unpackPingResponse(msg, pingId);

    if (errorCode != FirmwareSupport::Error::noError)
    {
        if (errorCode == FirmwareSupport::Error::messageIsAnEcho)
            return -1;
        
        DBG("WARNING: Ping response error in auto connection routine detected");
        jassertfalse;
    }

    return (int)pingId - 1;
}

void DeviceActivityMonitor::removeFailedPingDevice(const juce::MidiMessage &msg)
{
    unsigned int pingId = 0;
    LumatoneSysEx::unpackPingResponse(msg, pingId);
    outputDevices.remove(outputPingIds.indexOf(pingId));
}

void DeviceActivityMonitor::checkDetectionStatus()
{
    // Successful
    if (isConnectionEstablished())
    {
        deviceDetectInProgress = false;
        statusListeners.call(&LumatoneEditor::StatusListener::connectionStateChanged, ConnectionState::ONLINE);

        outputPingIds.clear();

        if (checkConnectionOnInactivity)
            startActivityMonitoring();
        else
            deviceConnectionMode = DetectConnectionMode::noDeviceMonitoring;
        
        return;
    }

    if (midiDriver->getHostMode() == LumatoneFirmwareDriver::HostMode::Plugin)
    {
        if (deviceDetectInProgress)
        {
            DBG("Detect device timeout.");
            deviceDetectInProgress = false;
            waitingForResponse = false;
            startTimer(detectRoutineTimeoutMs);
            
            statusListeners.call(&LumatoneEditor::StatusListener::connectionFailed);
        }
        else
        {
            deviceDetectInProgress = true;
            if (sendCalibratePitchModOff)
            {
                midiDriver->sendCalibratePitchModWheel(false, testOutputIndex);
            }
            else
            {
                midiDriver->sendGetSerialIdentityRequest(testOutputIndex);
            }

            waitingForResponse = true;
            startTimer(responseTimeoutMs);
        }
    }
    else
    {
        if (deviceDetectInProgress)
        {
            // Failed last-device test
            if (midiDriver->hasDevicesDefined())
            {
                DBG("Unable to connect to last device.");
                waitingForResponse = false;
                midiDriver->closeMidiInput();
                midiDriver->closeMidiOutput();
                pingAllDevices();
            }

            // Failed ping routine
            else if (outputPingIds.size() > 0)
            {
                waitingForResponse = false;
                outputPingIds.clear();
                startIndividualDetection();
            }

            // Ongoing GetSerial routine
            else if (testOutputIndex >= 0 && testOutputIndex < outputDevices.size())
            {
                testNextOutput();
            }
            
            // Set timeout for next attempt
            else
            {
                DBG("Detect device timeout.");
                deviceDetectInProgress = false;
                waitingForResponse = false;
                startTimer(detectRoutineTimeoutMs);

                statusListeners.call(&LumatoneEditor::StatusListener::connectionFailed);
            }
        }

        // Start detection
        else
        {
            midiDriver->refreshDeviceLists();
            midiDriver->openAvailableDevicesForTesting();
            deviceConnectionMode = DetectConnectionMode::lookingForDevice;

            // If there's no last-connected-device, skip to pinging
            //if (!testLastConnectedDevice())
            //{
                pingAllDevices();
            //}
        }
    }
}

void DeviceActivityMonitor::timerCallback()
{
    stopTimer();

    switch (deviceConnectionMode)
    {
    case DetectConnectionMode::idle:
    case DetectConnectionMode::lookingForDevice:
        if (detectDevicesIfDisconnected)
        {
            checkDetectionStatus();
            break;
        }

        stopDeviceDetection();
        break;

    case DetectConnectionMode::noDeviceMonitoring:
    case DetectConnectionMode::waitingForInactivity:
        if (!checkConnectionOnInactivity)
        {
            stopMonitoringDevice();
            break;
        }

        if (!waitingForResponse)
        {
            if (sentQueueSize > 0)
            {
                // Skip test because there's already a message being sent
                startTimer(inactivityTimeoutMs);
                break;
            }

            if (deviceConnectionMode == DetectConnectionMode::noDeviceMonitoring)
                deviceConnectionMode = DetectConnectionMode::waitingForInactivity;

            initializeConnectionTest();
        }
        else
        {
            DBG("DAM: testing connection...");
        }

        break;

    default:
        jassertfalse;
    }
}

void DeviceActivityMonitor::handleResponse(int inputDeviceIndex, const juce::MidiMessage& msg)
{
    if (!msg.isSysEx())
        return;

    auto sysExData = msg.getSysExData();
    auto cmd = sysExData[CMD_ID];

    if (cmd == PERIPHERAL_CALBRATION_DATA && !isConnectionEstablished())
    {
        sendCalibratePitchModOff = true;
        // startTimer(100);
        return;
    }

    if (waitingForResponse) switch (sysExData[MSG_STATUS])
    {
        // Ignore or mark as a failed ping
        case TEST_ECHO:
        {
            DBG("DAM ignoring feedback from device " + juce::String(inputDeviceIndex) + " with message: " + msg.getDescription());
            if (cmd == LUMA_PING)
                removeFailedPingDevice(msg);
            return;
        }

        case LumatoneFirmware::ReturnCode::ACK:
        {
            int establishOutIndex = -1;

            switch (cmd)
            {
            case GET_SERIAL_IDENTITY:
                establishOutIndex = testOutputIndex;
                break;

            case CALIBRATE_PITCH_MOD_WHEEL:
                if (sysExData[PAYLOAD_INIT] != TEST_ECHO)
                {
                    if (!isConnectionEstablished())
                    {
                        sendCalibratePitchModOff = true;
                    }
                }
                break;

            case GET_FIRMWARE_REVISION:
                break;

            case LUMA_PING:
                if (outputPingIds.size() == 0)
                    break;

                establishOutIndex = getPingIdFromResponse(msg);
                break;

            default:
                break;
            }

            waitingForResponse = false;

            if (sendCalibratePitchModOff)
            {
                sendCalibratePitchModOff = false;
                juce::MessageManager::callAsync([&]() { checkDetectionStatus(); });
                return;
            }

            switch (deviceConnectionMode)
            {
            case DetectConnectionMode::lookingForDevice:
                if (establishOutIndex >= 0)
                {
                    establishConnection(inputDeviceIndex, establishOutIndex);
                }
                else
                    jassertfalse;
                break;

            case DetectConnectionMode::waitingForInactivity:
                startTimer(inactivityTimeoutMs);
                break;
            
            default:
                break;
            }

            break;
        }

        case LumatoneFirmware::ReturnCode::STATE:
        {
            // TODO set warning flag ?
            break;
        }
    }

    // Edge case if we're disconnected but get a response
    else if (!isConnectionEstablished())
    {
        establishConnection(midiDriver->getMidiInputIndex(), midiDriver->getMidiOutputIndex());
    }
    else if (checkConnectionOnInactivity)
    {
        startTimer(inactivityTimeoutMs);
    }
}

//=========================================================================
// LumatoneFirmwareDriver::Listener Implementation

void DeviceActivityMonitor::midiMessageReceived(juce::MidiInput* source, const juce::MidiMessage& msg)
{
    if (!msg.isSysEx())
        return;

    int deviceIndex = (source == nullptr) ? -1
                                          : midiDriver->getMidiInputList().indexOf(source->getDeviceInfo());

    stopTimer();
    handleResponse(deviceIndex, msg);
}

void DeviceActivityMonitor::noAnswerToMessage(juce::MidiDeviceInfo expectedDevice, const juce::MidiMessage& midiMessage)
{
    stopTimer();

    if (waitingForResponse && deviceConnectionMode < DetectConnectionMode::noDeviceMonitoring)
    {
        waitingForResponse = false;
        auto sysExData = midiMessage.getSysExData();
        if (sysExData[CMD_ID] == LUMA_PING && outputPingIds.size() > 0)
        {
            removeFailedPingDevice(midiMessage);
        }
        else
        {
            startTimer(threadDelayMs);
        }
    }
    
    else if (isConnectionEstablished())
    {
        onDisconnection();
    }
}

void DeviceActivityMonitor::onDisconnection()
{
    DBG("DISCONNECTION DETECTED");

    confirmedInputIndex = -1;
    confirmedOutputIndex = -1;

    waitingForResponse = false;

    juce::MessageManager::callAsync([&]() { statusListeners.call(&LumatoneEditor::StatusListener::connectionStateChanged, ConnectionState::DISCONNECTED); });

    if (detectDevicesIfDisconnected)
    {
        startDeviceDetection();
    }
    else
    {
        deviceConnectionMode = DetectConnectionMode::idle;
        stopTimer();
    }
}

//==============================================================================
// LumatoneEditor::StatusListener

void DeviceActivityMonitor::establishConnection(int inputIndex, int outputIndex)
{
    stopTimer();

    DBG("DAM: Devices confirmed.");

    if (midiDriver->hasDevicesDefined())
    {
        confirmedOutputIndex = midiDriver->getMidiOutputIndex();
        confirmedInputIndex = midiDriver->getMidiInputIndex();

        DBG("\tInput: " + midiDriver->getMidiInputInfo().name);
        DBG("\tOutput: " + midiDriver->getMidiOutputInfo().name);
    }
    else if (midiDriver->getHostMode() == LumatoneFirmwareDriver::HostMode::Driver)
    {
        midiDriver->setMidiInput(inputIndex);
        midiDriver->setMidiOutput(outputIndex);

        DBG("\tInput: " + midiDriver->getMidiInputInfo().name);
        DBG("\tOutput: " + midiDriver->getMidiOutputInfo().name);
    }
    else
    {
        confirmedInputIndex = 0;
        confirmedOutputIndex = 0;

        DBG("\tPlugin host mode.");
    }

    startTimer(threadDelayMs);
}
