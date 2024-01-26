/*
  ==============================================================================

    EditActions.cpp
    Created: 9 May 2021 9:49:31pm
    Author:  hsstraub

  ==============================================================================
*/
#include "edit_actions.h"

#include "../data/application_state.h"

using namespace LumatoneEditAction;

// ==============================================================================
// Implementation of SingleNoteAssignAction
SingleNoteAssignAction::SingleNoteAssignAction(
    LumatoneApplicationState* stateIn,
    int boardIndexIn,
    int keyIndexIn,
    bool setKeyType,
    bool setChannel,
    bool setNote,
    bool setColour,
    bool setCCPolarity,
    LumatoneKeyType newKeyType,
    int newChannelNumber,
    int newNoteNumber,
    juce::Colour newColour,
    bool newCCFaderIsDefault)
    : LumatoneAction(stateIn, "SingleNoteAssign")
    , boardId(boardIndexIn + 1), keyIndex(keyIndexIn)
    , setKeyType(setKeyType), setChannel(setChannel), setNote(setNote), setColour(setColour), setCCFaderPolarity(setCCPolarity)
    , newData(newKeyType, newChannelNumber, newNoteNumber, newColour, newCCFaderIsDefault)
{
    previousData = state->getKey(boardIndexIn, keyIndexIn);
}

SingleNoteAssignAction::SingleNoteAssignAction(
    LumatoneApplicationState* stateIn,
    int boardIndex,
    int keyIndex,
    juce::Colour newColour)
    : SingleNoteAssignAction(state,
                            boardIndex, keyIndex,
                            false, false, false, true, false,
                            LumatoneKeyType::disabledDefault, 0, 0, newColour, false) {}

bool SingleNoteAssignAction::isValid() const
{
    return boardId > 0 && boardId <= numOctaveBoards && keyIndex >= 0 && keyIndex < octaveBoardSize;
}

bool SingleNoteAssignAction::perform()
{
    if (boardId > 0 && boardId <= numOctaveBoards && keyIndex >= 0 && keyIndex < octaveBoardSize)
    {
        LumatoneKey resultKey = previousData;
        if (setKeyType || setChannel || setNote || setColour || setCCFaderPolarity)
        {

            if (setKeyType)
            {
                resultKey.setKeyType(newData.getType());
            }
            if (setChannel)
            {
                resultKey.setChannelNumber(newData.getMidiChannel());
            }
            if (setNote)
            {
                resultKey.setNoteOrCC(newData.getMidiNumber());
            }
            if (setColour)
            {
                resultKey.setColour(newData.getColour());
            }
            if (setCCFaderPolarity)
            {
                resultKey.setDefaultCCFader(newData.isCCFaderDefault());
            }

            newData = resultKey;

            bool onlyColor = setColour && !(setKeyType || setChannel || setNote || setCCFaderPolarity);

            if (onlyColor)
            {
                state->setKeyColour(newData.getColour(), boardId, keyIndex);
            }
            else
            {
                state->setKey(newData, boardId, keyIndex);
            }

            // Notfy that there are changes: in calling function
        }
        else
        {
            // Nothing to do
            jassertfalse;
        }

        return true;
    }
    else
    {
        jassertfalse;
        return false;
    }
}

bool SingleNoteAssignAction::undo()
{
    if (boardId > 0 && boardId <= numOctaveBoards && keyIndex >= 0 && keyIndex < octaveBoardSize)
    {
        if (setKeyType || setChannel || setNote || setColour || setCCFaderPolarity)
        {
            LumatoneKey resultKey = newData;

            if (setKeyType)
            {
                resultKey.setKeyType(previousData.getType());
            }
            if (setChannel)
            {
                resultKey.setChannelNumber(previousData.getMidiChannel());
            }
            if (setNote)
            {
                resultKey.setNoteOrCC(previousData.getMidiNumber());
            }
            if (setColour)
            {
                resultKey.setColour(previousData.getColour());
            }
            if (setCCFaderPolarity)
            {
                resultKey.setDefaultCCFader(previousData.isCCFaderDefault());
            }

            previousData = resultKey;

            // Send to device
            state->setKey(
                previousData,
                boardId,
                keyIndex
                );

            // Notify that there are changes: in calling function
        }
        else
        {
            // Nothing to do
            jassertfalse;
        }

        return true;
    }
    else
    {
        jassertfalse;
        return false;
    }
}

// ==============================================================================
// Implementation of SectionEditAction

SectionEditAction::SectionEditAction(LumatoneApplicationState* stateIn, int boardIndexIn, const LumatoneBoard& newSectionValue, bool bufferKeyUpdates)
    : LumatoneAction(stateIn, "SectionEdit")
    , boardId(boardIndexIn + 1)
    , newData(newSectionValue)
    , useKeyBuffer(bufferKeyUpdates)
{
    previousData = state->getBoard(boardIndexIn);
}

bool SectionEditAction::isValid() const
{
    return boardId > 0 && boardId <= numOctaveBoards;
}

bool SectionEditAction::perform()
{
    if (boardId > 0 && boardId <= state->getNumBoards())
    {
        // Send to device
        state->setBoard(newData, boardId);
        //state->sendAllParamsOfBoard(boardId, &newData, true, useKeyBuffer);

        // Notify that there are changes: in calling function
        return true;
    }
    else
    {
        jassertfalse;
        return false;
    }
}

bool SectionEditAction::undo()
{
    if (boardId >= 0 && boardId <= numOctaveBoards)
    {
        // Send to device
        state->setBoard(previousData, boardId);
        //state->sendAllParamsOfBoard(boardId, &previousData, true, useKeyBuffer);

        // Notify that there are changes: in calling function
        return true;
    }
    else
    {
        jassertfalse;
        return false;
    }

}


MultiKeyAssignAction::MultiKeyAssignAction(LumatoneApplicationState* stateIn, const juce::Array<MappedLumatoneKey>& updatedKeys, bool setConfigIn, bool setColourIn, bool bufferKeyUpdates)
    : LumatoneAction(stateIn, "MultiKeyAssign")
    , useKeyBuffer(bufferKeyUpdates)
    , setConfig(setConfigIn)
    , setColours(setColourIn)
{
    for (auto updatedKey : updatedKeys)
    {
        auto coord = updatedKey.getKeyCoord();
        if (state->getMappingData()->isKeyCoordValid(coord))
        {
            newData.add(updatedKey);

            auto key = state->getKey(coord.boardIndex, coord.keyIndex);
            previousKeys.add(MappedLumatoneKey(key, coord));
        }
    }
}

bool MultiKeyAssignAction::isValid() const
{
    return newData.size() > 0 && newData.size() == previousKeys.size();
}

void MultiKeyAssignAction::applyMappedKeyData(const juce::Array<MappedLumatoneKey>& newKeys, const juce::Array<MappedLumatoneKey>& oldKeys)
{
    if (setConfig)
        state->sendSelectionParam(newKeys, false, useKeyBuffer);

    if (setColours)
        state->sendSelectionColours(newKeys, true, useKeyBuffer);
}

bool MultiKeyAssignAction::perform()
{
    applyMappedKeyData(newData, previousKeys);
    return true;
}

bool MultiKeyAssignAction::undo()
{
    applyMappedKeyData(previousKeys, newData);
    return true;
}

// ==============================================================================
// Implementation of InvertFootControllerEditAction

InvertFootControllerEditAction::InvertFootControllerEditAction(LumatoneApplicationState* stateIn, bool newValue)
    : LumatoneAction(stateIn, "InvertFootControllerEdit"), newData(newValue)
{
    previousData = state->getInvertExpression();
}

bool InvertFootControllerEditAction::perform()
{
    state->setInvertSustain(newData);
    return true;
}

bool InvertFootControllerEditAction::undo()
{
    state->setInvertSustain(previousData);
    return true;
}

// ==============================================================================
// Implementation of ExprPedalSensivityEditAction

ExprPedalSensivityEditAction::ExprPedalSensivityEditAction(LumatoneApplicationState* stateIn, int newValue)
    : LumatoneAction(stateIn, "ExprPedlSensitivity"), newData(newValue)
{
    previousData = state->getExpressionSensitivity();
}

bool ExprPedalSensivityEditAction::perform()
{
    state->setExpressionSensitivity(newData);
    return true;
}

bool ExprPedalSensivityEditAction::undo()
{
    state->setExpressionSensitivity(previousData);
    return true;
}

// ==============================================================================
// Implementation of InvertSustainEditAction

InvertSustainEditAction::InvertSustainEditAction(LumatoneApplicationState* stateIn, bool newValue)
    : LumatoneAction(stateIn, "InvertSustainEdit"), newData(newValue)
{
    previousData = state->getInvertSustain();
}

bool InvertSustainEditAction::perform()
{
    state->setInvertSustain(newData);
    return true;
}

bool InvertSustainEditAction::undo()
{
    state->setInvertSustain(previousData);
    return true;
}
