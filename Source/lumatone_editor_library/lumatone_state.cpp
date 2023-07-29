/*
  ==============================================================================

    lumatone_state.cpp
    Created: 4 Jun 2023 4:57:50pm
    Author:  Vincenzo

  ==============================================================================
*/

#include "lumatone_state.h"

juce::Array<juce::Identifier> LumatoneState::getAllProperties()
{
    juce::Array<juce::Identifier> properties;
    properties.add(LumatoneEditorProperty::ConnectionStateId);
    properties.add(LumatoneEditorProperty::LastConnectedSerialNumber);
    properties.add(LumatoneEditorProperty::LastConnectedFirmwareVersion);
    properties.add(LumatoneEditorProperty::LastConnectedNumBoards);

    properties.add(LumatoneEditorProperty::DetectDeviceIfDisconnected);
    properties.add(LumatoneEditorProperty::CheckConnectionIfInactive);

    properties.add(LumatoneEditorProperty::MappingData);

    return properties;
}

LumatoneState::LumatoneState(juce::ValueTree stateIn, juce::UndoManager* undoManagerIn)
    : LumatoneStateBase(), undoManager(undoManagerIn) 
{ 
    state = loadStateProperties(stateIn);
    state.addListener(this);
}

LumatoneState::LumatoneState(const LumatoneState& stateToCopy, juce::UndoManager* undoManagerIn)
    : LumatoneState(stateToCopy.state, undoManagerIn)
{
}

juce::ValueTree LumatoneState::loadStateProperties(juce::ValueTree stateIn)
{
    juce::ValueTree newState = (stateIn.hasType(LumatoneEditorProperty::StateTree))
                             ? stateIn
                             : juce::ValueTree(LumatoneEditorProperty::StateTree);

    for (auto property : getAllProperties())
    {
        convertStateMemberValue(newState, property);
    }

    return newState;
}

void LumatoneState::convertStateMemberValue(juce::ValueTree stateIn, const juce::Identifier& property)
{
    if (property == LumatoneEditorProperty::ConnectionStateId)
    {
        connectionState = ConnectionState((int)stateIn.getProperty(property, (int)ConnectionState::DISCONNECTED));
    }
    else if (property == LumatoneEditorProperty::LastConnectedSerialNumber)
    {
        connectedSerialNumber = stateIn.getProperty(property).toString();
    }
    else if (property == LumatoneEditorProperty::LastConnectedFirmwareVersion)
    {
        determinedVersion = LumatoneFirmwareVersion((int)stateIn.getProperty(property, (int)LumatoneFirmwareVersion::FUTURE_VERSION));
        firmwareVersion = FirmwareVersion::fromDeterminedVersion(determinedVersion);
    }
    else if (property == LumatoneEditorProperty::DetectDeviceIfDisconnected)
    {
        detectDeviceIfDisconnected = (bool)stateIn.getProperty(property, true);
    }
    else if (property == LumatoneEditorProperty::CheckConnectionIfInactive)
    {
        monitorConnectionStatus = (bool)stateIn.getProperty(property, true);
    }
    else if (property == LumatoneEditorProperty::MappingData)
    {
        juce::String mappingString = stateIn.getProperty(property).toString();
        auto stringArray = juce::StringArray::fromLines(mappingString);
        LumatoneLayout loadedLayout;
        loadedLayout.fromStringArray(stringArray);

        if (!loadedLayout.isEmpty())
        {
            mappingData = loadedLayout;
        }
    }
}

void LumatoneState::setConnectedSerialNumber(juce::String serialNumberIn)
{
    connectedSerialNumber = serialNumberIn;
    state.setPropertyExcludingListener(
        this, 
        LumatoneEditorProperty::LastConnectedSerialNumber, 
        connectedSerialNumber, 
        undoManager);

    if (connectedSerialNumber == SERIAL_55_KEYS)
    {
        determinedVersion = LumatoneFirmwareVersion::VERSION_55_KEYS;
        state.setPropertyExcludingListener(
            this, 
            LumatoneEditorProperty::LastConnectedFirmwareVersion, 
            (int)determinedVersion, 
            undoManager);
    }
}

void LumatoneState::setFirmwareVersion(FirmwareVersion& versionIn, bool writeToState)
{
    firmwareVersion = FirmwareVersion(versionIn);
    determinedVersion = firmwareSupport.getLumatoneFirmwareVersion(firmwareVersion);

    if (writeToState)
    {
        state.setPropertyExcludingListener(
            this,
            LumatoneEditorProperty::LastConnectedFirmwareVersion,
            (int)determinedVersion,
            undoManager);
    }
}

void LumatoneState::valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property)
{
    if (treeWhosePropertyHasChanged == state)
    {
        DBG("LumatoneStsate::valueTreePropertyChanged(" 
            + treeWhosePropertyHasChanged.getType().toString() + ", " 
            + property.toString() + ")");

        convertStateMemberValue(state, property);
    }
}

ConnectionState LumatoneState::getConnectionState() const
{
    return connectionState;
}

LumatoneFirmwareVersion LumatoneState::getLumatoneVersion() const
{
    return determinedVersion;
}

FirmwareVersion LumatoneState::getFirmwareVersion() const
{
    return firmwareVersion;
}

juce::String LumatoneState::getSerialNumber() const
{
    return connectedSerialNumber;
}

int LumatoneState::getOctaveBoardSize() const
{
    return octaveBoardSize;
}

const LumatoneLayout* LumatoneState::getMappingData() const
{
    return &mappingData;
}
const LumatoneBoard* LumatoneState::getBoard(int boardIndex) const
{
    return mappingData.readBoard(boardIndex);
}
const LumatoneKey* LumatoneState::getKey(int boardIndex, int keyIndex) const
{
    return &mappingData.readBoard(boardIndex)->theKeys[keyIndex];
}

LumatoneBoard* LumatoneState::getEditBoard(int boardIndex)
{
    return mappingData.getBoard(boardIndex);
}

LumatoneKey* LumatoneState::getEditKey(int boardIndex, int keyIndex)
{
    return &mappingData.getBoard(boardIndex)->theKeys[keyIndex];
}

const FirmwareSupport& LumatoneState::getFirmwareSupport() const
{
    return firmwareSupport;
}

