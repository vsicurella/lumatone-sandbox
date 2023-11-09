/*
  ==============================================================================

    lumatone_state.cpp
    Created: 4 Jun 2023 4:57:50pm
    Author:  Vincenzo

  ==============================================================================
*/

#include "lumatone_state.h"

#include "../lumatone_output_map.h"

juce::Array<juce::Identifier> LumatoneState::getLumatoneStateProperties()
{
    juce::Array<juce::Identifier> properties;
    properties.add(LumatoneStateProperty::LastConnectedSerialNumber);
    properties.add(LumatoneStateProperty::LastConnectedFirmwareVersion);
    properties.add(LumatoneStateProperty::LastConnectedNumBoards);

    properties.add(LumatoneStateProperty::MappingData);

    return properties;
}

LumatoneState::LumatoneState(juce::String nameIn, juce::ValueTree stateIn, juce::UndoManager* undoManagerIn)
    : LumatoneStateBase(nameIn)
    , undoManager(undoManagerIn)
{ 
    state = loadStateProperties(stateIn);
    state.addListener(this);

    mappingData = std::make_shared<LumatoneLayout>();
    midiKeyMap = std::make_shared<LumatoneOutputMap>(mappingData.get());
}

LumatoneState::LumatoneState(juce::String nameIn, const LumatoneState& stateToCopy, juce::UndoManager* undoManagerIn)
    : LumatoneState(nameIn, stateToCopy.state, undoManagerIn)
{
    // mappingData = stateToCopy.mappingData;
    // midiKeyMap = stateToCopy.midiKeyMap;
}

LumatoneState::~LumatoneState()
{
    state.removeListener(this);

    midiKeyMap = nullptr;
    mappingData = nullptr;
}

juce::ValueTree LumatoneState::loadStateProperties(juce::ValueTree stateIn)
{
    juce::ValueTree newState = (stateIn.hasType(LumatoneStateProperty::StateTree))
                             ? stateIn
                             : juce::ValueTree(LumatoneStateProperty::StateTree);

    for (auto property : getLumatoneStateProperties())
    {
        if (newState.hasProperty(property))
            handleStatePropertyChange(newState, property);
    }

    return newState;
}

void LumatoneState::handleStatePropertyChange(juce::ValueTree stateIn, const juce::Identifier& property)
{
    if (property == LumatoneStateProperty::LastConnectedSerialNumber)
    {
        connectedSerialNumber = stateIn.getProperty(property).toString();
    }
    else if (property == LumatoneStateProperty::LastConnectedFirmwareVersion)
    {
        setLumatoneVersion(
            LumatoneFirmware::ReleaseVersion((int)stateIn.getProperty(property, (int)LumatoneFirmware::ReleaseVersion::FUTURE_VERSION))
            );
        firmwareVersion = LumatoneFirmware::Version::fromReleaseVersion(determinedVersion);
    }
    else if (property == LumatoneStateProperty::MappingData)
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
    else if (property == LumatoneStateProperty::InvertExpression)
    {
        invertExpression = (bool)stateIn.getProperty(property, false);
    }    
    else if (property == LumatoneStateProperty::InvertSustain)
    {
        invertSustain = (bool)stateIn.getProperty(property, false);
    }
    else if (property == LumatoneStateProperty::ExpressionSensitivity)
    {
        expressionSensitivity = juce::uint8((int)stateIn.getProperty(property, 127));
    }
}

void LumatoneState::setConnectedSerialNumber(juce::String serialNumberIn)
{
    connectedSerialNumber = serialNumberIn;
    state.setPropertyExcludingListener(
        this, 
        LumatoneStateProperty::LastConnectedSerialNumber, 
        connectedSerialNumber, 
        undoManager);

    numBoards = 5;

    if (connectedSerialNumber == SERIAL_55_KEYS)
    {
        setLumatoneVersion(LumatoneFirmware::ReleaseVersion::VERSION_55_KEYS);
    }
}

void LumatoneState::setFirmwareVersion(LumatoneFirmware::Version& versionIn, bool writeToState)
{
    firmwareVersion = LumatoneFirmware::Version(versionIn);
    setLumatoneVersion(firmwareSupport.getReleaseVersion(firmwareVersion), writeToState);
}

void LumatoneState::setLumatoneVersion(LumatoneFirmware::ReleaseVersion versionIn, bool writeToState)
{
    determinedVersion = versionIn;

    numBoards = 5;

    switch (determinedVersion)
    {
    case LumatoneFirmware::ReleaseVersion::VERSION_55_KEYS:
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
            LumatoneStateProperty::LastConnectedFirmwareVersion,
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

        handleStatePropertyChange(state, property);
    }
}

LumatoneFirmware::ReleaseVersion LumatoneState::getLumatoneVersion() const
{
    return determinedVersion;
}

LumatoneFirmware::Version LumatoneState::getFirmwareVersion() const
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
    state.setPropertyExcludingListener(this, LumatoneStateProperty::InvertExpression, invert, undoManager);
    invertExpression = invert;
}

void LumatoneState::setInvertSustain(bool invert)
{
    state.setPropertyExcludingListener(this, LumatoneStateProperty::InvertSustain, invert, undoManager);
    invertSustain = invert;
}

void LumatoneState::setExpressionSensitivity(juce::uint8 sensitivity)
{
    state.setPropertyExcludingListener(this, LumatoneStateProperty::ExpressionSensitivity, (int)sensitivity, undoManager);
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
            *mappingData = LumatoneLayout(newLayout);

            auto layoutString = mappingData->toStringArray().joinIntoString(juce::newLine);
            DBG("Loaded: " + layoutString);

            writeStringProperty(LumatoneStateProperty::MappingData, layoutString, undoManager);

            invertSustain = mappingData->invertSustain;
            writeBoolProperty(LumatoneStateProperty::InvertSustain, invertSustain, undoManager);

            invertExpression = mappingData->invertExpression;
            writeBoolProperty(LumatoneStateProperty::InvertExpression, invertExpression, undoManager);
            
            expressionSensitivity = mappingData->expressionControllerSensivity;
            writeIntProperty(LumatoneStateProperty::ExpressionSensitivity, expressionSensitivity, undoManager);

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
