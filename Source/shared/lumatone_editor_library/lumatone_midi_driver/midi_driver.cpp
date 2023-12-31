/*
  ==============================================================================

    HajuMidiDriver.cpp
    Created: 23 Feb 2018 11:30:58pm
    Author:  hsstraub

  ==============================================================================
*/

#include "./midi_driver.h"


HajuMidiDriver::HajuMidiDriver()
{
	midiInputs = juce::MidiInput::getAvailableDevices();
	midiOutputs = juce::MidiOutput::getAvailableDevices();
    midiInput = nullptr;
    midiOutput = nullptr;
}

HajuMidiDriver::~HajuMidiDriver()
{
    closeMidiInput();
    closeMidiOutput();
}

const juce::Array<juce::MidiDeviceInfo>& HajuMidiDriver::getMidiInputList() const
{
	return midiInputs;
}

const juce::Array<juce::MidiDeviceInfo>& HajuMidiDriver::getMidiOutputList() const
{ 
	return midiOutputs;
}

int HajuMidiDriver::findIndexOfInputDevice(juce::String inputDeviceId) const
{
    for (int i = 0; i < midiInputs.size(); i++)
        if (midiInputs[i].identifier == inputDeviceId)
            return i;

    return -1;
}


int HajuMidiDriver::findIndexOfOutputDevice(juce::String outputDeviceId) const
{
    for (int i = 0; i < midiOutputs.size(); i++)
        if (midiOutputs[i].identifier == outputDeviceId)
            return i;

    return -1;
}

bool HajuMidiDriver::refreshDeviceLists()
{
    bool inputsChanged = false;
    auto newInputs = juce::MidiInput::getAvailableDevices();
    if (newInputs.size() == midiInputs.size())
    {
        for (int i = 0; i < newInputs.size(); i++)
            if (newInputs[i] != midiInputs[i])
                inputsChanged = true;
    }
    else
        inputsChanged = true;

    // Reset last opened device to be the same index
    // so we close the correct one if that's requested
    // order change is probably rare but can't be guaranteed

    if (inputsChanged)
    {
        if (lastInputIndex >= 0)
        {
            juce::String lastInputId = midiInputs[lastInputIndex].identifier;

            for (int i = 0; i < newInputs.size(); i++)
                if (newInputs[i].identifier == lastInputId)
                    lastInputIndex = i;
        }
        
        midiInputs = newInputs;
    }

    bool outputsChanged = false;
    auto newOutputs = juce::MidiOutput::getAvailableDevices();
    if (newOutputs.size() == midiOutputs.size())
    {
        for (int i = 0; i < newOutputs.size(); i++)
            if (newOutputs[i] != midiOutputs[i])
                outputsChanged = true;
    }
    else
        outputsChanged = true;

    if (outputsChanged)
    {
        if (lastOutputIndex >= 0)
        {
            juce::String lastOutputId = midiOutputs[lastOutputIndex].identifier;

            for (int i = 0; i < newOutputs.size(); i++)
                if (newOutputs[i].identifier == lastOutputId)
                    lastOutputIndex = i;
        }
        
        midiOutputs = newOutputs;
    }

    return inputsChanged || outputsChanged;
}

void HajuMidiDriver::setMidiInput(int deviceIndex)
{
    jassert(deviceIndex < midiInputs.size());

    testInputs.clear();
    closeMidiInput();

    if (deviceIndex < 0)
    {
        return;
    }
    
    DBG("Trying to open input device: " + midiInputs[deviceIndex].name);
    selectedInput = juce::MidiInput::openDevice(midiInputs[deviceIndex].identifier, this);

    if (selectedInput != nullptr)
    {
        midiInput = selectedInput.get();
        midiInput->start();

        lastInputIndex = deviceIndex;
        lastInputDevice = midiInputs[deviceIndex];
    }
    else
    {
        DBG("WARNING: Could not open up input device " + juce::String(deviceIndex));
    }
}

void HajuMidiDriver::setMidiOutput(int deviceIndex)
{
    jassert(deviceIndex < midiOutputs.size());

    testOutputs.clear();
    closeMidiOutput();

    if (deviceIndex < 0)
    {
        return;
    }

    DBG("Trying to open output device: " + midiOutputs[deviceIndex].name);
    selectedOutput = juce::MidiOutput::openDevice(midiOutputs[deviceIndex].identifier);

    if (selectedOutput != nullptr)
    {
        midiOutput = selectedOutput.get();
        lastOutputIndex = deviceIndex;
        lastOutputDevice = midiOutputs[deviceIndex];
    }
    else
    {
        DBG("WARNING: Could not open up output device " + juce::String(deviceIndex));
    }
}

void HajuMidiDriver::sendMessageNow(const juce::MidiMessage& message)
{
	// Send only if output device is there
	if (midiOutput != nullptr)
    {
		midiOutput->sendMessageNow(message);
        return;
    }
    // jassertfalse;
    DBG("MidiOutput is null!");
}

void HajuMidiDriver::closeMidiInput()
{
    if (midiInput != nullptr)
    {
        midiInput->stop();
        midiInput = nullptr;
        lastInputIndex = -1;
    }
    
    selectedInput = nullptr;
}

void HajuMidiDriver::closeMidiOutput()
{
    if (midiOutput != nullptr)
    {
        midiOutput = nullptr;
        lastOutputIndex = -1;
    }

    if (selectedOutput != nullptr)
    {
        selectedOutput = nullptr;
    }
}

//============================================================================
// Device detection support

void HajuMidiDriver::openAvailableDevicesForTesting()
{
    // May be preferable to only close and open input devices when necessary
    closeTestingDevices();

    for (auto device : midiOutputs)
    {
        auto newOutput = juce::MidiOutput::openDevice(device.identifier);
        if (newOutput == nullptr)
            continue;

        testOutputs.add(newOutput.release());
    }

    for (auto device : midiInputs)
    {
        auto newInput = juce::MidiInput::openDevice(device.identifier, this);
        if (newInput == nullptr)
            continue;

        auto openedInput = testInputs.add(newInput.release());
        openedInput->start();
    }
}

void HajuMidiDriver::sendTestMessageNow(int outputDeviceIndex, const juce::MidiMessage& message)
{
    // Return some error code?
    if (outputDeviceIndex >= 0 && outputDeviceIndex < testOutputs.size())
    {
        //DBG("Sending this message to " + testOutputs[outputDeviceIndex]->getName() + ":");
        DBG("TEST: " + message.getDescription());
        testOutputs.getUnchecked(outputDeviceIndex)->sendMessageNow(message);
    }
}

// Closes all open testing devices; either setMidiInput and setMidiOutput will call this if it's not empty
void HajuMidiDriver::closeTestingDevices()
{
    if (testOutputs.size() > 0)
        testOutputs.clear();

    int errors = 0;
    for (int i = 0; i < testInputs.size(); i++)
    {
        auto input = testInputs[i];
        if (input != nullptr)
        {
            input->stop();
            input = nullptr;
        }
    }

    testInputs.clear();
}
