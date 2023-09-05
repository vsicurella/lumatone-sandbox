/*
  ==============================================================================

    DeviceActivityMonitor.cpp
    Created: 11 Feb 2021 9:15:43pm
    Author:  Vincenzo

  ==============================================================================
*/

#include "DeviceActivityMonitor.h"
//#include "Main.h"


DeviceActivityMonitor::DeviceActivityMonitor(LumatoneFirmwareDriver* midiDriverIn, LumatoneApplicationState stateIn)
    :   LumatoneApplicationState(stateIn)
    ,   midiDriver(midiDriverIn) 
    ,   readQueueSize(0)
{
    detectDevicesIfDisconnected = getBoolProperty(LumatoneApplicationProperty::DetectDeviceIfDisconnected, true);
    checkConnectionOnInactivity = getBoolProperty(LumatoneApplicationProperty::CheckConnectionIfInactive, true);
    responseTimeoutMs = getIntProperty(LumatoneApplicationProperty::DetectDevicesTimeout, detectRoutineTimeoutMs);

//    midiDriver->addListener(this);
    reset(readBlockSize);
    midiDriver->addMessageCollector(this);

    // avoid resizing during communication
    ensureStorageAllocated(2000);
    testResponseDeviceIndices.resize(2000);
}

DeviceActivityMonitor::~DeviceActivityMonitor()
{
    midiDriver->removeMessageCollector(this);
}

void DeviceActivityMonitor::setDetectDeviceIfDisconnected(bool doDetection)
{
    detectDevicesIfDisconnected = doDetection;
    writeBoolProperty(LumatoneApplicationProperty::DetectDeviceIfDisconnected, detectDevicesIfDisconnected);

    if (!detectDevicesIfDisconnected)
    {
        deviceConnectionMode = DetectConnectionMode::noDeviceActivity;
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
    deviceDetectInProgress = true;

    outputDevices = midiDriver->getMidiOutputList();

    outputPingIds.clear();

    waitingForResponse = true;

    int maxDevices = juce::jmin(outputDevices.size(), 128);
    for (int i = 0; i < maxDevices; i++)
    {
        unsigned int id = (unsigned int)i + 1;
        midiDriver->ping(id, i);
        outputPingIds.add(id);
    }

    startTimer(responseTimeoutMs);
}

bool DeviceActivityMonitor::testLastConnectedDevice()
{
    juce::String inputId = getStringProperty(LumatoneApplicationProperty::LastInputDeviceId);
    if (inputId.length() <= 0)
        return false;

    int inputIndex = midiDriver->findIndexOfInputDevice(inputId);
    if (inputIndex >= 0)
    {
        juce::String outputId = getStringProperty(LumatoneApplicationProperty::LastOutputDeviceId);
        if (outputId.length() <= 0)
            return false;
        
        int outputIndex = midiDriver->findIndexOfOutputDevice(outputId);
        if (outputIndex >= 0)
        {
            deviceDetectInProgress = true;
            waitingForResponse = true;

            DBG("Testing last connected device");
            midiDriver->setMidiInput(inputIndex);
            midiDriver->setMidiOutput(outputIndex);
            midiDriver->sendGetSerialIdentityRequest();
            return true;
        }
    }

    return false;
}

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
    testOutputIndex++;

    if (testOutputIndex >= 0 && testOutputIndex < outputDevices.size()) 
    {
        DBG("Testing " + outputDevices[testOutputIndex].name);
        if (sendCalibratePitchModOff)
            midiDriver->sendCalibratePitchModWheel(false, testOutputIndex);
        else
            midiDriver->sendGetSerialIdentityRequest(testOutputIndex);
        
        waitingForResponse = true;
        startTimer(responseTimeoutMs);
    }
    // failed
    else
    {
        startTimer(10);
    }
}

void DeviceActivityMonitor::startDeviceDetection()
{
    if (midiDriver->getHostMode() != LumatoneFirmwareDriver::HostMode::Driver)
        return; // Only allow in Driver mode
        
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
    deviceConnectionMode = DetectConnectionMode::noDeviceActivity;
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
    if (getSerialNumber().isNotEmpty() && getLumatoneVersion() >= LumatoneFirmwareVersion::VERSION_1_0_9)
    {
        midiDriver->ping(0xf);
    }

    else
    {
        midiDriver->sendGetSerialIdentityRequest();
    }

    waitingForResponse = true;
    startTimer(inactivityTimeoutMs);
    return true;
}


void DeviceActivityMonitor::onSerialIdentityResponse(const juce::MidiMessage& msg, int deviceIndexResponded)
{
    waitingForResponse = false;

    switch (deviceConnectionMode)
    {
    case DetectConnectionMode::lookingForDevice:
        if (midiDriver->hasDevicesDefined())
        {
            confirmedOutputIndex = midiDriver->getMidiOutputIndex();
            confirmedInputIndex = midiDriver->getMidiInputIndex();
        }
        else
        {
            connectionEstablished(deviceIndexResponded, testOutputIndex);
            //confirmedOutputIndex = testOutputIndex;
            //confirmedInputIndex = deviceIndexResponded;
        }

        break;

    case DetectConnectionMode::waitingForInactivity:
        startTimer(inactivityTimeoutMs);
        break;

    default:
        // TODO review
        break;
    }
}

void DeviceActivityMonitor::onFailedPing(const juce::MidiMessage& msg)
{
    unsigned int pingId = -1;
    midiDriver->unpackPingResponse(msg, pingId);
    if (pingId >= 0 && pingId < outputDevices.size())
        outputDevices.remove(outputPingIds.indexOf(pingId));
}

void DeviceActivityMonitor::onPingResponse(const juce::MidiMessage& msg, int deviceIndexResponded)
{
    waitingForResponse = false;

    if (deviceConnectionMode == DetectConnectionMode::lookingForDevice && outputPingIds.size() > 0)
    {
        unsigned int pingId = 0;
        auto errorCode = midiDriver->unpackPingResponse(msg, pingId);

        if (errorCode != FirmwareSupport::Error::noError)
        {
            if (errorCode == FirmwareSupport::Error::messageIsAnEcho)
                return;
            
            DBG("WARNING: Ping response error in auto connection routine detected");
            jassertfalse;
            return;
        }

        if (pingId > 0)
        {
            connectionEstablished(deviceIndexResponded, pingId - 1);
            //confirmedOutputIndex = pingId - 1;
            //confirmedInputIndex = deviceIndexResponded;
            // startTimer(10);
        }
    }

    else if (deviceConnectionMode == DetectConnectionMode::waitingForInactivity)
    {
        activityResponseReceived();
    }
}

void DeviceActivityMonitor::activityResponseReceived()
{
    waitingForResponse = false;
    
    if (sendCalibratePitchModOff)
    {
        sendCalibratePitchModOff = false;
        checkDetectionStatus();
        return;
    }

    else if (deviceConnectionMode == DetectConnectionMode::waitingForInactivity)
    {
        startTimer(inactivityTimeoutMs);
    }
}

void DeviceActivityMonitor::checkDetectionStatus()
{
    // Successful
    if (isConnectionEstablished())
    {
        activityResponseReceived();
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

    juce::MidiBuffer readBuffer;
    removeNextBlockOfMessages(readBuffer, readBlockSize);
    
    handleMessageQueue(readBuffer, testResponseDeviceIndices);
    auto size = readQueueSize.load();
    readQueueSize.store(juce::jlimit(0, 999999, size - readBlockSize));

    switch (deviceConnectionMode)
    {
    case DetectConnectionMode::noDeviceActivity:
    case DetectConnectionMode::lookingForDevice:
        if (detectDevicesIfDisconnected)
        {
            checkDetectionStatus();
            break;
        }

        stopDeviceDetection();
        break;

    case DetectConnectionMode::confirmingDevice:
        break;

    case DetectConnectionMode::noDeviceMonitoring:
    case DetectConnectionMode::waitingForInactivity:
        if (JUCE_DEBUG) 
        {
            bool estab = isConnectionEstablished();
            bool devdef = midiDriver->hasDevicesDefined();
            jassert(estab && devdef);
        }

        if (!checkConnectionOnInactivity)
        {
            stopMonitoringDevice();
            break;
        }

        if (!waitingForResponse)
        {
            if (sentQueueSize > 0)
            {
                startTimer(inactivityTimeoutMs);
                break;
            }

            if (deviceConnectionMode == DetectConnectionMode::noDeviceMonitoring)
                deviceConnectionMode = DetectConnectionMode::waitingForInactivity;

            initializeConnectionTest();
        }
        else
        {
            DBG("waiting for test response...");
        }

        break;

    default:
        jassertfalse;
    }
}

void DeviceActivityMonitor::handleResponse(int inputDeviceIndex, const juce::MidiMessage& msg)
{
    if (msg.isSysEx())
    {
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
            // Skip echos, or mark as a failed ping
            case TEST_ECHO:
            {
                switch (cmd)
                {
                case LUMA_PING:
                {
                    DBG("Ignoring Ping Echo");
                    onFailedPing(msg);
                    break;
                }
                case GET_SERIAL_IDENTITY:
                    break;

                case GET_FIRMWARE_REVISION:
                    DBG("Ignoring Firmware Echo");
                    // activityResponseReceived();
                    break;

                default:
                    return;
                }
                break;
            }

            case TerpstraMIDIAnswerReturnCode::ACK:
            {
                switch (cmd)
                {
                case GET_SERIAL_IDENTITY:
                    onSerialIdentityResponse(msg, inputDeviceIndex);
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
                    onPingResponse(msg, inputDeviceIndex);
                    break;

                default:
                    break;
                }

                waitingForResponse = false;
                break;
            }

            // Consider a response from a different firmware state as successful
            case TerpstraMIDIAnswerReturnCode::STATE:
            {
                // Find 'off' message if possible
                //activityResponseReceived();
            }
        }

        // Edge case if we're disconnected but get a response
        else if (!isConnectionEstablished())
        {
            connectionEstablished(midiDriver->getMidiInputIndex(), midiDriver->getMidiOutputIndex());
        }
        else
        {
            startTimer(inactivityTimeoutMs);
        }
    }

}

void DeviceActivityMonitor::handleMessageQueue(const juce::MidiBuffer& readBuffer, const juce::Array<int, juce::CriticalSection>& devices)
{
    int smpl = 0;
    for (auto event : readBuffer)
    {
        auto msg = event.getMessage();
        handleResponse(devices[smpl], msg);
        smpl++;
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

    addMessageToQueue(msg);

    auto size = readQueueSize.load();
    testResponseDeviceIndices.set(size, deviceIndex);
    readQueueSize.store(size + 1);
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
            onFailedPing(midiMessage);
        }
        else
        {
            checkDetectionStatus();
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

    if (detectDevicesIfDisconnected)
    {
        startDeviceDetection();
    }
    else
    {
        statusListeners.call(&LumatoneEditor::StatusListener::connectionStateChanged, ConnectionState::DISCONNECTED);
        deviceConnectionMode = DetectConnectionMode::noDeviceActivity;
        stopTimer();
    }
}

//==============================================================================
// LumatoneEditor::StatusListener

void DeviceActivityMonitor::connectionFailed()
{
    if (deviceConnectionMode == DetectConnectionMode::lookingForDevice && outputPingIds.size() > 0)
    {
        startTimer(inactivityTimeoutMs);
    }

    statusListeners.call(&LumatoneEditor::StatusListener::connectionFailed);
}

void DeviceActivityMonitor::connectionEstablished(int inputIndex, int outputIndex)
{
    deviceDetectInProgress = false;
    outputPingIds.clear();
    stopTimer();

    midiDriver->setMidiInput(inputIndex);
    midiDriver->setMidiOutput(outputIndex);

    confirmedInputIndex = inputIndex;
    confirmedOutputIndex = outputIndex;

    statusListeners.call(&LumatoneEditor::StatusListener::connectionStateChanged, ConnectionState::ONLINE);


    if (checkConnectionOnInactivity)
    {
        startActivityMonitoring();
    }
    else
    {
        deviceConnectionMode = DetectConnectionMode::noDeviceMonitoring;
    }
}
