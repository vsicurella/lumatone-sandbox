/*
  ==============================================================================

    SandboxState.h
    Created: 4 Jun 2023 4:56:00pm
    Author:  Vincenzo

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class LumatoneSandboxState
{

private:
    juce::ValueTree state;
    juce::UndoManager* undoManager;

public:

    LumatoneSandboxState(juce::ValueTree stateIn, juce::UndoManager* undoManager = nullptr);
    LumatoneSandboxState(const LumatoneSandboxState& stateIn, juce::UndoManager* undoManager = nullptr);

    bool getBoolProperty(const juce::Identifier key, bool fallback) const;
    int getIntProperty(const juce::Identifier key, int fallback) const;
    juce::String getStringProperty(const juce::Identifier key, juce::String fallback = juce::String()) const;

    bool writeToPropertiesFile();

protected:

    void writeBoolProperty(const juce::Identifier key, bool value);
    void writeIntProperty(const juce::Identifier key, int value);
    void writeStringProperty(const juce::Identifier key, juce::String value);
};

namespace LumatoneSandboxProperty
{
    static const juce::Identifier StateTree = juce::Identifier("LumatoneSandboxStateTree");

    // Device Management
    static const juce::Identifier DetectDeviceIfDisconnected = juce::Identifier("DetectDeviceIfDisconnected");
    static const juce::Identifier CheckConnectionIfInactive = juce::Identifier("CheckConnectionIfInactive");
    static const juce::Identifier DetectDevicesTimeout = juce::Identifier("DetectDevicesTimeout");
    static const juce::Identifier LastInputDeviceId = juce::Identifier("LastInputDeviceId");
    static const juce::Identifier LastOutputDeviceId = juce::Identifier("LastOutputDeviceId");


    // UI States

    static const juce::Identifier ConnectionStateId = juce::Identifier("ConnectionState");

    enum class ConnectionState
    {
        DISCONNECTED = 0,
        SEARCHING,
        OFFLINE,
        ONLINE,
        BUSY,
        ERROR
    };
};

