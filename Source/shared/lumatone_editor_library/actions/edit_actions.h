/*
  ==============================================================================

    EditActions.h
    Created: 9 May 2021 9:49:13pm
    Author:  hsstraub

  ==============================================================================
*/

#pragma once
#include "./lumatone_action.h"
#include "../data/lumatone_layout.h"

namespace LumatoneEditAction {

    class SingleNoteAssignAction : public LumatoneAction
    {
    public:
        SingleNoteAssignAction(
            LumatoneApplicationState* state,
            int boardIndexIn,
            int keyIndexIn,
            bool setKeyType,
            bool setChannel, 
            bool setNote,
            bool setColour,
            bool ccFaderDefault,
            LumatoneKeyType newKeyType = LumatoneKeyType::noteOnNoteOff,
            int newChannelNumber = 0, 
            int newNoteNumber = 0,
            juce::Colour newColour = juce::Colour(),
            bool newCCFaderDefault = true);

        SingleNoteAssignAction(const SingleNoteAssignAction& second)
            : LumatoneAction(second.state, "SingleNoteAssign")
            , boardId(second.boardId)
            , keyIndex(second.keyIndex)
            , setKeyType(second.setKeyType)
            , setChannel(second.setChannel)
            , setNote(second.setNote)
            , setColour(second.setColour)
            , setCCFaderPolarity(second.setCCFaderPolarity)
            , previousData(second.previousData)
            , newData(second.newData)
        {}

        SingleNoteAssignAction(
            LumatoneApplicationState* state,
            int boardId,
            int keyIndex,
            juce::Colour newColour
        );

        bool isValid() const;

        bool perform() override;
        bool undo() override;
        
        int getSizeInUnits() override { return sizeof(SingleNoteAssignAction); }

    private:
        int boardId = - 1;
        int keyIndex = -1;

        bool setKeyType = false;
        bool setChannel = false;
        bool setNote = false;
        bool setColour = false;
        bool setCCFaderPolarity = false;

        LumatoneKey previousData;
        LumatoneKey newData;
    };

    class SectionEditAction : public LumatoneAction
    {
    public:
        SectionEditAction(LumatoneApplicationState* state, int boardIndexIn, const LumatoneBoard& newSectionValue, bool bufferKeyUpdates=false);

        SectionEditAction(const SectionEditAction& second)
            : LumatoneAction(second.state, "SectionEditAction")
            , boardId(second.boardId)
            , previousData(second.previousData)
            , newData(second.newData)
            , useKeyBuffer(second.useKeyBuffer)
        {}

        bool isValid() const;

        bool perform() override;
        bool undo() override;
        int getSizeInUnits() override { return sizeof(SectionEditAction); }

    private:
        int boardId = -1;

        LumatoneBoard previousData;
        LumatoneBoard newData;

        bool useKeyBuffer;
    };

    class MultiKeyAssignAction : public LumatoneAction
    {
    public:
        MultiKeyAssignAction(LumatoneApplicationState* state, const juce::Array<MappedLumatoneKey>& updatedKeys, bool setConfig=true, bool setColour=true, bool bufferKeyUpdates=false);
        MultiKeyAssignAction(const MultiKeyAssignAction& copy)
            : LumatoneAction(copy.state, "MultiKeyAssign")
			, previousKeys(copy.previousKeys)
            , newData(copy.newData)
            , setConfig(copy.setConfig)
            , setColours(copy.setColours)
            , useKeyBuffer(copy.useKeyBuffer)
        {}

        bool isValid() const;

        bool perform() override;
		bool undo() override;

		int getSizeInUnits() override { return sizeof(MultiKeyAssignAction); }

	private:
		void applyMappedKeyData(const juce::Array<MappedLumatoneKey>& newKeys, const juce::Array<MappedLumatoneKey>& oldKeys);

	private:
        bool setConfig = true;
        bool setColours = true;
        bool useKeyBuffer = false;

		juce::Array<MappedLumatoneKey> previousKeys;
		juce::Array<MappedLumatoneKey> newData;
    };

    class InvertFootControllerEditAction : public LumatoneAction
    {
    public:
        InvertFootControllerEditAction(LumatoneApplicationState* state, bool newValue);

        InvertFootControllerEditAction(const InvertFootControllerEditAction& second)
            : LumatoneAction(second.state, "InvertFootControllerEdit")
            , previousData(second.previousData)
            , newData(second.newData)
        {}

        bool perform() override;
        bool undo() override;
        int getSizeInUnits() override { return sizeof(InvertFootControllerEditAction); }

    private:
        bool previousData;
        bool newData;
    };

    class ExprPedalSensivityEditAction : public LumatoneAction
    {
    public:
        ExprPedalSensivityEditAction(LumatoneApplicationState* state, int newValue);

        ExprPedalSensivityEditAction(const ExprPedalSensivityEditAction& second)
            : LumatoneAction(second.state, "ExprPedalSensitivityEdit")
            , previousData(second.previousData)
            , newData(second.newData)
        {}

        bool perform() override;
        bool undo() override;
        int getSizeInUnits() override { return sizeof(ExprPedalSensivityEditAction); }

    private:
        int previousData;
        int newData;
    };

    class InvertSustainEditAction : public LumatoneAction
    {
    public:
        InvertSustainEditAction(LumatoneApplicationState* state, bool newValue);
        
        InvertSustainEditAction(const InvertSustainEditAction& second)
            : LumatoneAction(second.state, "InvertSustainEdit")
            , previousData(second.previousData)
            , newData(second.newData)
        {}
        
        bool perform() override;
        bool undo() override;
        int getSizeInUnits() override { return sizeof(InvertSustainEditAction); }

    private:
        int previousData;
        int newData;
    };

}
