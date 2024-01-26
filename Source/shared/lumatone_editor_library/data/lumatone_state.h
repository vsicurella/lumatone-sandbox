/*
  ==============================================================================

    lumatone_state.h
    Created: 4 Jun 2023 4:57:50pm
    Author:  Vincenzo

  ==============================================================================
*/

#ifndef LUMATONE_STATE_H
#define LUMATONE_STATE_H

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

    static const juce::Identifier LightsOnAfterKeystroke = juce::Identifier("LightsOnAfterKeystroke");
    static const juce::Identifier AftertouchEnabled = juce::Identifier("AftertouchEnabled");
    static const juce::Identifier InvertExpression = juce::Identifier("InvertExpression");
    static const juce::Identifier InvertSustain = juce::Identifier("InvertSustain");
    static const juce::Identifier ExpressionSensitivity = juce::Identifier("ExpressionSensitivity");

    static const juce::Identifier InactiveMacroButtonColour = juce::Identifier("InactiveMacroButtonColour");
    static const juce::Identifier ActiveMacroButtonColour = juce::Identifier("ActiveMacroButtonColour");
};

class LumatoneState : public LumatoneStateBase
{
public:

    LumatoneState(juce::String nameIn, juce::ValueTree stateIn=juce::ValueTree(), juce::UndoManager* undoManager=nullptr);
    LumatoneState(juce::String nameIn, const LumatoneState& stateToCopy);
    LumatoneState(const LumatoneState& stateIn);

    virtual ~LumatoneState() override;

    LumatoneFirmware::ReleaseVersion getLumatoneVersion() const;
    LumatoneFirmware::Version getFirmwareVersion() const;

    juce::String getSerialNumber() const;

    LumatoneLayout* getMappingData() const;
    std::shared_ptr<LumatoneLayout> shareMappingData() { return mappingData; }

    const LumatoneBoard& getBoard(int boardIndex) const;

    const LumatoneKey& getKey(int boardIndex, int keyIndex) const;
    const LumatoneKey& getKey(LumatoneKeyCoord coord) const;

    const LumatoneOutputMap* getMidiKeyMap() const;

    const FirmwareSupport& getFirmwareSupport() const;

    // virtual bool loadLayoutFromFile(const juce::File& layoutFile);

    virtual void setCompleteConfig(const LumatoneLayout& layoutIn);
    virtual void setLayout(const LumatoneLayout& layoutIn);
    virtual void setBoard(const LumatoneBoard& boardIn, int boardId);

    virtual void setKey(const LumatoneKey& keyIn, int boardId, int keyIndex);
    virtual void setKeyConfig(const LumatoneKey& keyIn, int boardId, int keyIndex);
    virtual void setKeyColour(juce::Colour colour, int boardId, int keyIndex);

    virtual void sendSelectionParam(const juce::Array<MappedLumatoneKey>& selection, bool signalEditorListeners = true, bool bufferKeyUpdates = false);
    virtual void sendSelectionColours(const juce::Array<MappedLumatoneKey>& selection, bool signalEditorListeners = true, bool bufferKeyUpdates = false);

    virtual void setAftertouchEnabled(bool enabled);
    virtual void setLightOnKeyStrokes(bool enabled);
    virtual void setInvertExpression(bool invert);
    virtual void setInvertSustain(bool invert);
    virtual void setExpressionSensitivity(juce::uint8 sensitivity);

    virtual void setConfigTable(LumatoneConfigTable::TableType type, const LumatoneConfigTable& table);

    virtual void setInactiveMacroButtonColour(juce::Colour buttonColour);
    virtual void setActiveMacroButtonColour(juce::Colour buttonColour);

public:
    // Layout Helpers
    int getNumBoards() const { return mappingData->getNumBoards(); }
    int getOctaveBoardSize() const { return mappingData->getOctaveBoardSize(); }

    bool getAftertouchOn() const { return mappingData->getAftertouchOn(); }
    bool getLightOnKeyStrokes() const { return mappingData->getLightOnKeyStrokes(); }
    bool getInvertExpression() const { return mappingData->getInvertExpression(); }
    bool getInvertSustain() const { return mappingData->getInvertSustain(); }
    int getExpressionSensitivity() const { return mappingData->getExpressionSensitivity(); }

    juce::Colour getInactiveMacroButtonColour() const { return inactiveMacroButtonColour; }
    juce::Colour getActiveMacroButtonColour() const { return activeMacroButtonColour; }

protected:
    void setConnectedSerialNumber(juce::String serialNumberIn);
    void setFirmwareVersion(LumatoneFirmware::Version& versionIn, bool writeToState=false);

    void setLumatoneVersion(LumatoneFirmware::ReleaseVersion versionIn, bool writeToState=false);

protected:

    virtual juce::ValueTree loadStateProperties(juce::ValueTree stateIn) override;

    virtual void valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property) override;

    virtual void handleStatePropertyChange(juce::ValueTree stateIn, const juce::Identifier& property) override;

    virtual void loadPropertiesFile(juce::PropertiesFile* properties) override;

public:
    static juce::Array<juce::Identifier> getLumatoneStateProperties();

protected:

    juce::UndoManager* undoManager;

    FirmwareSupport firmwareSupport;

    std::shared_ptr<LumatoneLayout> mappingData;
    std::shared_ptr<LumatoneOutputMap> midiKeyMap;

private:
    juce::String connectedSerialNumber = juce::String();

    LumatoneFirmware::ReleaseVersion     determinedVersion;
    LumatoneFirmware::Version             firmwareVersion;
    LumatoneFirmware::Version             incomingVersion;

    juce::Colour    inactiveMacroButtonColour;
    juce::Colour    activeMacroButtonColour;
};



#endif // LUMATONE_STATE_H
