#pragma once

#include "lumatone_state.h"

enum class ConnectionState
{
    DISCONNECTED = 0,
    SEARCHING,
    OFFLINE,
    ONLINE,
    BUSY,
    ERROR
};

namespace LumatoneApplicationProperty
{
    // Device Management
    static const juce::Identifier DetectDeviceIfDisconnected = juce::Identifier("DetectDeviceIfDisconnected");
    static const juce::Identifier CheckConnectionIfInactive = juce::Identifier("CheckConnectionIfInactive");
    static const juce::Identifier DetectDevicesTimeout = juce::Identifier("DetectDevicesTimeout");
    static const juce::Identifier LastInputDeviceId = juce::Identifier("LastInputDeviceId");
    static const juce::Identifier LastOutputDeviceId = juce::Identifier("LastOutputDeviceId");

    // UI States
    static const juce::Identifier ConnectionStateId = juce::Identifier("ConnectionState");

    // Settings
    static const juce::Identifier DefaultMappingsDirectory = juce::Identifier("DefaultMappingsDirectory");
    static const juce::Identifier LastMappingsDirectory = juce::Identifier("LastMappingsDirectory");
}

class LumatoneColourModel;

class LumatoneApplicationState : public LumatoneState
{
public:

    LumatoneApplicationState(juce::ValueTree state=juce::ValueTree(), juce::UndoManager* undoManager=nullptr);
    LumatoneApplicationState(const LumatoneState& stateIn, juce::UndoManager* undoManager=nullptr);
    LumatoneApplicationState(const LumatoneApplicationState& stateIn, juce::UndoManager* undoManager=nullptr);

    virtual ~LumatoneApplicationState();

    ConnectionState getConnectionState() const;

    juce::File getDefaultMappingsDirectory();
    juce::File getLastMappingsDirectory();

    LumatoneColourModel* getColourModel() const;

protected:

    virtual juce::ValueTree loadStateProperties(juce::ValueTree stateIn);

    virtual void handleStatePropertyChange(juce::ValueTree stateIn, const juce::Identifier& property) override;

public:
    static juce::Array<juce::Identifier> getLumatoneApplicationProperties();

protected:
    ConnectionState connectionState = ConnectionState::DISCONNECTED;

    bool detectDeviceIfDisconnected     = true;
    bool monitorConnectionStatus        = true;

private:

    std::shared_ptr<LumatoneColourModel> colourModel;
};
