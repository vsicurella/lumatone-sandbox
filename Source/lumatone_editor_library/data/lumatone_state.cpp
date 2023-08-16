/*
  ==============================================================================

    lumatone_state.cpp
    Created: 4 Jun 2023 4:57:50pm
    Author:  Vincenzo

  ==============================================================================
*/

#include "lumatone_state.h"
#include "../color/colour_model.h"

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

    mappingData = std::make_shared<LumatoneLayout>();
    colourModel = std::make_shared<LumatoneColourModel>();
    midiKeyMap = std::make_shared<LumatoneOutputMap>(mappingData.get());
}

LumatoneState::LumatoneState(const LumatoneState& stateToCopy, juce::UndoManager* undoManagerIn)
    : LumatoneState(stateToCopy.state, undoManagerIn)
{
    mappingData = stateToCopy.mappingData;
    colourModel = stateToCopy.colourModel;
    midiKeyMap = stateToCopy.midiKeyMap;
}

LumatoneState::~LumatoneState()
{
    state.removeListener(this);

    midiKeyMap = nullptr;
    colourModel = nullptr;
    mappingData = nullptr;
}

juce::ValueTree LumatoneState::loadStateProperties(juce::ValueTree stateIn)
{
    juce::ValueTree newState = (stateIn.hasType(LumatoneEditorProperty::StateTree))
                             ? stateIn
                             : juce::ValueTree(LumatoneEditorProperty::StateTree);

    DBG("LumatoneState::loadStateProperties:\n" + newState.toXmlString());
    for (auto property : getAllProperties())
    {
        if (newState.hasProperty(property))
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
        setLumatoneVersion(
            LumatoneFirmwareVersion((int)stateIn.getProperty(property, (int)LumatoneFirmwareVersion::FUTURE_VERSION))
            );
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
        if (mappingString.isEmpty())
            return;

        auto stringArray = juce::StringArray::fromLines(mappingString);
        LumatoneLayout loadedLayout(getNumBoards(), getOctaveBoardSize());
        loadedLayout.fromStringArray(stringArray);

        if (!loadedLayout.isEmpty())
        {
            mappingData.reset(new LumatoneLayout(loadedLayout));
        }
    }
    else if (property == LumatoneEditorProperty::InvertExpression)
    {
        invertExpression = (bool)stateIn.getProperty(property, false);
    }    
    else if (property == LumatoneEditorProperty::InvertSustain)
    {
        invertSustain = (bool)stateIn.getProperty(property, false);
    }
    else if (property == LumatoneEditorProperty::ExpressionSensitivity)
    {
        expressionSensitivity = juce::uint8((int)stateIn.getProperty(property, 127));
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

    numBoards = 5;

    if (connectedSerialNumber == SERIAL_55_KEYS)
    {
        setLumatoneVersion(LumatoneFirmwareVersion::VERSION_55_KEYS);
    }
}

void LumatoneState::setFirmwareVersion(FirmwareVersion& versionIn, bool writeToState)
{
    firmwareVersion = FirmwareVersion(versionIn);
    setLumatoneVersion(firmwareSupport.getLumatoneFirmwareVersion(firmwareVersion), writeToState);
}

void LumatoneState::setLumatoneVersion(LumatoneFirmwareVersion versionIn, bool writeToState)
{
    determinedVersion = versionIn;

    numBoards = 5;

    switch (determinedVersion)
    {
    case LumatoneFirmwareVersion::VERSION_55_KEYS:
        octaveBoardSize = 55;
        break;
    default:
        octaveBoardSize = 56;
        break;
    }

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

int LumatoneState::getNumBoards() const
{
    return numBoards;
}

int LumatoneState::getOctaveBoardSize() const
{
    return octaveBoardSize;
}

const LumatoneLayout* LumatoneState::getMappingData() const
{
    return mappingData.get();
}
const LumatoneBoard* LumatoneState::getBoard(int boardIndex) const
{
    return mappingData->readBoard(boardIndex);
}
const LumatoneKey* LumatoneState::getKey(int boardIndex, int keyIndex) const
{
    return &mappingData->readBoard(boardIndex)->theKeys[keyIndex];
}

const LumatoneKey* LumatoneState::getKey(LumatoneKeyCoord coord) const
{
    return getKey(coord.boardIndex, coord.keyIndex);
}

const LumatoneOutputMap* LumatoneState::getMidiKeyMap() const
{
    return midiKeyMap.get();
}

LumatoneColourModel* LumatoneState::getColourModel() const
{
    return colourModel.get();
}

bool LumatoneState::isKeyCoordValid(const LumatoneKeyCoord& coord) const
{
    return coord.isInitialized()
        && coord.boardIndex < getNumBoards()
        && coord.keyIndex < getOctaveBoardSize();
}

LumatoneBoard* LumatoneState::getEditBoard(int boardIndex)
{
    return mappingData->getBoard(boardIndex);
}

LumatoneKey* LumatoneState::getEditKey(int boardIndex, int keyIndex)
{
    return &mappingData->getBoard(boardIndex)->theKeys[keyIndex];
}

const FirmwareSupport& LumatoneState::getFirmwareSupport() const
{
    return firmwareSupport;
}

void LumatoneState::setInvertExpression(bool invert)
{
    state.setPropertyExcludingListener(this, LumatoneEditorProperty::InvertExpression, invert, undoManager);
    invertExpression = invert;
}

void LumatoneState::setInvertSustain(bool invert)
{
    state.setPropertyExcludingListener(this, LumatoneEditorProperty::InvertSustain, invert, undoManager);
    invertSustain = invert;
}

void LumatoneState::setExpressionSensitivity(juce::uint8 sensitivity)
{
    state.setPropertyExcludingListener(this, LumatoneEditorProperty::ExpressionSensitivity, (int)sensitivity, undoManager);
    expressionSensitivity = sensitivity;
}


bool LumatoneState::loadLayoutFromFile(const juce::File& layoutFile)
{
    bool fileOpened = false;
    bool fileParsed = false;

    if (layoutFile.existsAsFile())
    {
        fileOpened = true;

        juce::StringArray stringArray;
        layoutFile.readLines(stringArray);

        LumatoneLayout newLayout(getNumBoards(), getOctaveBoardSize(), true);
        newLayout.fromStringArray(stringArray);

        // TODO: something if boards/size don't match?
        fileParsed = true;


        if (fileParsed)
        {
            mappingData.reset(new LumatoneLayout(newLayout));

            // Mark file as unchanged
            //setHasChangesToSave(false);

            // Clear undo history
            //undoManager.clearUndoHistory();

            // Add file to recent files list
            //recentFiles.addFile(currentFile);

            return true;
        }
    }

    if (fileOpened)
    {
        // Show error message
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::AlertIconType::WarningIcon, "Open File Error", "The file " + layoutFile.getFullPathName() + " could not be opened.");

        // XXX Update Window title in any case? Make file name empty/make data empty in case of error?
    }

    return false;
}
