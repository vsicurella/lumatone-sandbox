/*
  ==============================================================================

    EditActions.h
    Created: 9 May 2021 9:49:13pm
    Author:  hsstraub

  ==============================================================================
*/

#pragma once
#include "./lumatone_action.h"

namespace LumatoneEditAction {

	class SingleNoteAssignAction : public LumatoneAction
	{
	public:
		SingleNoteAssignAction(
			LumatoneController* controller,
			int setSelection,
			int keySelection,
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
			: LumatoneAction(second.controller)
			, setSelection(second.setSelection)
			, keySelection(second.keySelection)
			, setKeyType(second.setKeyType)
			, setChannel(second.setChannel)
			, setNote(second.setNote)
			, setColour(second.setColour)
            , setCCFaderPolarity(second.setCCFaderPolarity)
			, previousData(second.previousData)
			, newData(second.newData)
		{}

		SingleNoteAssignAction(
			LumatoneController* controller,
			int boardIndex,
			int keyIndex,
			juce::Colour newColour
		);

		bool isValid() const;

		virtual bool perform() override;
		virtual bool undo() override;
		
		int getSizeInUnits() override { return sizeof(SingleNoteAssignAction); }

	private:
		int setSelection = - 1;
		int keySelection = -1;

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
		SectionEditAction(LumatoneController* controller, int setSelection, const LumatoneBoard& newSectionValue);

		SectionEditAction(const SectionEditAction& second)
			: LumatoneAction(second.controller)
			, setSelection(second.setSelection)
			, previousData(second.previousData)
			, newData(second.newData)
		{}

		bool isValid() const;

		virtual bool perform() override;
		virtual bool undo() override;
		int getSizeInUnits() override { return sizeof(SectionEditAction); }

	private:
		int setSelection = -1;

		LumatoneBoard previousData;
		LumatoneBoard newData;
	};

	class InvertFootControllerEditAction : public LumatoneAction
	{
	public:
		InvertFootControllerEditAction(LumatoneController* controller, bool newValue);

		InvertFootControllerEditAction(const InvertFootControllerEditAction& second)
			: LumatoneAction(second.controller)
			, previousData(second.previousData)
			, newData(second.newData)
		{}

		virtual bool perform() override;
		virtual bool undo() override;
		int getSizeInUnits() override { return sizeof(InvertFootControllerEditAction); }

	private:
		bool previousData;
		bool newData;
	};

	class ExprPedalSensivityEditAction : public LumatoneAction
	{
	public:
		ExprPedalSensivityEditAction(LumatoneController* controller, int newValue);

		ExprPedalSensivityEditAction(const ExprPedalSensivityEditAction& second)
			: LumatoneAction(second.controller)
			, previousData(second.previousData)
			, newData(second.newData)
		{}

		virtual bool perform() override;
		virtual bool undo() override;
		int getSizeInUnits() override { return sizeof(ExprPedalSensivityEditAction); }

	private:
		int previousData;
		int newData;
	};

    class InvertSustainEditAction : public LumatoneAction
    {
    public:
        InvertSustainEditAction(LumatoneController* controller, bool newValue);
        
        InvertSustainEditAction(const InvertSustainEditAction& second)
			: LumatoneAction(second.controller)
			, previousData(second.previousData)
			, newData(second.newData)
        {}
        
        virtual bool perform() override;
        virtual bool undo() override;
        int getSizeInUnits() override { return sizeof(InvertSustainEditAction); }

    private:
        int previousData;
        int newData;
    };

}
