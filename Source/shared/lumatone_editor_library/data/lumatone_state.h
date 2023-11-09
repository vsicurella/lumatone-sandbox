/*
  ==============================================================================

    lumatone_state.h
    Created: 4 Jun 2023 4:57:50pm
    Author:  Vincenzo

  ==============================================================================
*/

#pragma once

#include "state_base.h"
#include "lumatone_layout.h"

#include "../lumatone_midi_driver/firmware_support.h"

class LumatoneOutputMap;


namespace LumatoneStateProperty
{
    static const juce::Identifier StateTree = juce::Identifier("LumatoneEditorStateTree");

    // Lumatone Data
    static const juce::Identifier LastConnectedSerialNumber = juce::Identifier("LastConnectedSerialNumber");
    static const juce::Identifier LastConnectedFirmwareVersion = juce::Identifier("LastConnectedFirmwareVersion");
    static const juce::Identifier LastConnectedNumBoards = juce::Identifier("LastConnectedNumBoards");

    static const juce::Identifier MappingData = juce::Identifier("MappingData");

    static const juce::Identifier InvertExpression = juce::Identifier("InvertExpression");
    static const juce::Identifier InvertSustain = juce::Identifier("InvertSustain");
    static const juce::Identifier ExpressionSensitivity = juce::Identifier("ExpressionSensitivity");
};

class LumatoneState : public LumatoneStateBase
{
public:

    LumatoneState(juce::String nameIn, juce::ValueTree stateIn, juce::UndoManager* undoManager=nullptr);
    LumatoneState(juce::String nameIn, const LumatoneState& stateToCopy, juce::UndoManager* undoManagerIn=nullptr);

    virtual ~LumatoneState();

    LumatoneFirmware::ReleaseVersion getLumatoneVersion() const;
    LumatoneFirmware::Version getFirmwareVersion() const;

    juce::String getSerialNumber() const;

    int getOctaveBoardSize() const;
    int getNumBoards() const;

    const LumatoneLayout* getMappingData() const;
    const LumatoneBoard* getBoard(int boardIndex) const;

    std::shared_ptr<LumatoneLayout> shareMappingData() { return mappingData; }
    
    const LumatoneKey* getKey(int boardIndex, int keyIndex) const;
    const LumatoneKey* getKey(LumatoneKeyCoord coord) const;

    const LumatoneOutputMap* getMidiKeyMap() const;

    const FirmwareSupport& getFirmwareSupport() const;

    bool getInvertExpression() const { return invertExpression; }
    bool getInvertSustain() const { return invertSustain; }
    juce::uint8 getExpressionSensitivity() const { return expressionSensitivity; }

    virtual bool loadLayoutFromFile(const juce::File& layoutFile);

protected:

    void setConnectedSerialNumber(juce::String serialNumberIn);
    void setFirmwareVersion(LumatoneFirmware::Version& versionIn, bool writeToState=false);

    void setLumatoneVersion(LumatoneFirmware::ReleaseVersion versionIn, bool writeToState=false);

    void setInvertExpression(bool invert);
    void setInvertSustain(bool invert);
    void setExpressionSensitivity(juce::uint8 sensitivity);

protected:

    virtual juce::ValueTree loadStateProperties(juce::ValueTree stateIn) override;

    virtual void valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property) override;
    
    virtual void handleStatePropertyChange(juce::ValueTree stateIn, const juce::Identifier& property) override;

public:
    static juce::Array<juce::Identifier> getLumatoneStateProperties();

protected:

    juce::UndoManager* undoManager;
    
    FirmwareSupport firmwareSupport;

    std::shared_ptr<LumatoneLayout> mappingData;
    std::shared_ptr<LumatoneOutputMap> midiKeyMap;

    LumatoneBoard* getEditBoard(int boardIndex);
    LumatoneKey* getEditKey(int boardIndex, int keyIndex);

private:

    bool    invertExpression = false;
    bool    invertSustain = false;
    juce::uint8     expressionSensitivity = 0;

    juce::String                connectedSerialNumber = juce::String();
    LumatoneFirmware::ReleaseVersion     determinedVersion;
    LumatoneFirmware::Version             firmwareVersion;
    LumatoneFirmware::Version             incomingVersion;

    int                         numBoards = 5;
    int                         octaveBoardSize = 56;
};
