/*
  ==============================================================================

    lumatone_state.h
    Created: 4 Jun 2023 4:57:50pm
    Author:  Vincenzo

  ==============================================================================
*/

#pragma once

#include "state_base.h"
#include "./lumatone_midi_driver/firmware_definitions.h"
#include "LumatoneDataStructures.h"

enum class ConnectionState
{
    DISCONNECTED = 0,
    SEARCHING,
    OFFLINE,
    ONLINE,
    BUSY,
    ERROR
};

namespace LumatoneEditorProperty
{
    static const juce::Identifier StateTree = juce::Identifier("LumatoneEditorStateTree");

    // Device Management
    static const juce::Identifier DetectDeviceIfDisconnected = juce::Identifier("DetectDeviceIfDisconnected");
    static const juce::Identifier CheckConnectionIfInactive = juce::Identifier("CheckConnectionIfInactive");
    static const juce::Identifier DetectDevicesTimeout = juce::Identifier("DetectDevicesTimeout");
    static const juce::Identifier LastInputDeviceId = juce::Identifier("LastInputDeviceId");
    static const juce::Identifier LastOutputDeviceId = juce::Identifier("LastOutputDeviceId");

    // UI States
    static const juce::Identifier ConnectionStateId = juce::Identifier("ConnectionState");

    // Lumatone Data
    static const juce::Identifier LastConnectedSerialNumber = juce::Identifier("LastConnectedSerialNumber");
    static const juce::Identifier LastConnectedFirmwareVersion = juce::Identifier("LastConnectedFirmwareVersion");
    static const juce::Identifier LastConnectedNumBoards = juce::Identifier("LastConnectedNumBoards");

    static const juce::Identifier MappingData = juce::Identifier("MappingData");

};

class LumatoneState : public LumatoneStateBase, private juce::ValueTree::Listener
{
public:

    LumatoneState(juce::ValueTree stateIn, juce::UndoManager* undoManager=nullptr);
    LumatoneState(const LumatoneState& stateToCopy, juce::UndoManager* undoManagerIn=nullptr);

    virtual ~LumatoneState();

    ConnectionState getConnectionState() const;

    LumatoneFirmwareVersion getLumatoneVersion() const;
    FirmwareVersion getFirmwareVersion() const;

    juce::String getSerialNumber() const;

    int getOctaveBoardSize() const;
    int getNumBoards() const { return BOARD_OCT_5; } // TODO: Set variable based on serial/

    const LumatoneLayout* getMappingData() const;
    const LumatoneBoard* getBoard(int boardIndex) const;
    const LumatoneKey* getKey(int boardIndex, int keyIndex) const;

    const FirmwareSupport& getFirmwareSupport() const;

    virtual bool loadLayoutFromFile(const juce::File& layoutFile);

public:
    static juce::Array<juce::Identifier> getAllProperties();

protected:

    void setConnectedSerialNumber(juce::String serialNumberIn);

    void setFirmwareVersion(FirmwareVersion& versionIn, bool writeToState=false);

private:

    juce::ValueTree loadStateProperties(juce::ValueTree stateIn);

    void convertStateMemberValue(juce::ValueTree stateIn, const juce::Identifier& property);

private:

    virtual void valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property);


protected:
    
    FirmwareSupport firmwareSupport;

    //LumatoneLayout mappingData;
    std::shared_ptr<LumatoneLayout> mappingData;

    LumatoneBoard* getEditBoard(int boardIndex);
    LumatoneKey* getEditKey(int boardIndex, int keyIndex);

private:

    juce::UndoManager* undoManager;

    ConnectionState connectionState = ConnectionState::DISCONNECTED;

    bool detectDeviceIfDisconnected     = true;
    bool monitorConnectionStatus        = true;

    juce::String                connectedSerialNumber = juce::String();
    LumatoneFirmwareVersion     determinedVersion = LumatoneFirmwareVersion::NO_VERSION;
    FirmwareVersion             firmwareVersion = { 0, 0, 0 };
    FirmwareVersion             incomingVersion = { 0, 0, 0 };
    int                         octaveBoardSize = 56;
};
