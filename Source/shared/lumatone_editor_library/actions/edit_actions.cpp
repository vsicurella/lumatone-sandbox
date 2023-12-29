/*
  ==============================================================================

    EditActions.cpp
    Created: 9 May 2021 9:49:31pm
    Author:  hsstraub

  ==============================================================================
*/
#include "edit_actions.h"

#include "../device/lumatone_controller.h"

using namespace LumatoneEditAction;

// ==============================================================================
// Implementation of SingleNoteAssignAction
SingleNoteAssignAction::SingleNoteAssignAction(
    LumatoneController* controller,
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
    : LumatoneAction(controller, "SingleNoteAssign")
    , boardId(boardIndexIn + 1), keyIndex(keyIndexIn)
    , setKeyType(setKeyType), setChannel(setChannel), setNote(setNote), setColour(setColour), setCCFaderPolarity(setCCPolarity)
    , newData(newKeyType, newChannelNumber, newNoteNumber, newColour, newCCFaderIsDefault)
{
    previousData = *controller->getKey(boardIndexIn, keyIndexIn);
}

SingleNoteAssignAction::SingleNoteAssignAction(
    LumatoneController* controller,
    int boardIndex,
    int keyIndex,
    juce::Colour newColour) 
    : SingleNoteAssignAction(controller, 
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
                resultKey.keyType = newData.keyType;
            }
            if (setChannel)
            {
                resultKey.channelNumber = newData.channelNumber;
            }
            if (setNote)
            {
                resultKey.noteNumber = newData.noteNumber;
            }
            if (setColour)
            {
                resultKey.colour = newData.colour;
            }
            if (setCCFaderPolarity)
            {
                resultKey.ccFaderDefault = newData.ccFaderDefault;
            }

            newData = resultKey;

            bool onlyColor = setColour && !(setKeyType || setChannel || setNote || setCCFaderPolarity);
            
            if (onlyColor)
            {
                controller->sendKeyColourConfig(boardId, keyIndex, newData);
            }
            else
            {
                // Send to device
                controller->sendKeyParam(
                    boardId,
                    keyIndex,
                    newData);
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
                resultKey.keyType = previousData.keyType;
            }
            if (setChannel)
            {
                resultKey.channelNumber = previousData.channelNumber;
            }
            if (setNote)
            {
                resultKey.noteNumber = previousData.noteNumber;
            }
            if (setColour)
            {
                resultKey.colour = previousData.colour;
            }
            if (setCCFaderPolarity)
            {
                resultKey.ccFaderDefault = previousData.ccFaderDefault;
            }

            previousData = resultKey;

            // Send to device
            controller->sendKeyParam(
                boardId,
                keyIndex,
                previousData);

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

SectionEditAction::SectionEditAction(LumatoneController* controller, int boardIndexIn, const LumatoneBoard& newSectionValue, bool bufferKeyUpdates)
    : LumatoneAction(controller, "SectionEdit")
    , boardId(boardIndexIn + 1)
    , newData(newSectionValue)
    , useKeyBuffer(bufferKeyUpdates)
{
    previousData = *controller->getBoard(boardIndexIn);
}

bool SectionEditAction::isValid() const
{
    return boardId > 0 && boardId <= numOctaveBoards;
}

bool SectionEditAction::perform()
{
    if (boardId > 0 && boardId <= controller->getNumBoards())
    {
        // Send to device
        controller->sendAllParamsOfBoard(boardId, &newData, true, useKeyBuffer);

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
        controller->sendAllParamsOfBoard(boardId, &previousData, true, useKeyBuffer);

        // Notify that there are changes: in calling function
        return true;
    }
    else
    {
        jassertfalse;
        return false;
    }

}


MultiKeyAssignAction::MultiKeyAssignAction(LumatoneController* controller, const juce::Array<MappedLumatoneKey>& updatedKeys, bool setConfigIn, bool setColourIn, bool bufferKeyUpdates)
    : LumatoneAction(controller, "MultiKeyAssign")
    , useKeyBuffer(bufferKeyUpdates)
    , setConfig(setConfigIn)
    , setColours(setColourIn)
{
    for (auto updatedKey : updatedKeys)
    {
        auto coord = updatedKey.getKeyCoord();
        if (controller->getMappingData()->isKeyCoordValid(coord))
        {
            newData.add(updatedKey);

            auto key = controller->getKey(coord.boardIndex, coord.keyIndex);
            previousKeys.add(MappedLumatoneKey(*key, coord));
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
        controller->sendSelectionParam(newKeys, false, useKeyBuffer);
        
    if (setColours)
        controller->sendSelectionColours(newKeys, true, useKeyBuffer);
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

InvertFootControllerEditAction::InvertFootControllerEditAction(LumatoneController* controller, bool newValue)
    : LumatoneAction(controller, "InvertFootControllerEdit"), newData(newValue)
{
    previousData = controller->getInvertExpression();
}

bool InvertFootControllerEditAction::perform()
{
    controller->invertSustainPedal(newData);
    return true;
}

bool InvertFootControllerEditAction::undo()
{
    controller->invertSustainPedal(previousData);
    return true;
}

// ==============================================================================
// Implementation of ExprPedalSensivityEditAction

ExprPedalSensivityEditAction::ExprPedalSensivityEditAction(LumatoneController* controller, int newValue)
    : LumatoneAction(controller, "ExprPedlSensitivity"), newData(newValue)
{
    previousData = controller->getExpressionSensitivity();
}

bool ExprPedalSensivityEditAction::perform()
{
    controller->sendExpressionPedalSensivity(newData);
    return true;
}

bool ExprPedalSensivityEditAction::undo()
{
    controller->sendExpressionPedalSensivity(previousData);
    return true;
}

// ==============================================================================
// Implementation of InvertSustainEditAction

InvertSustainEditAction::InvertSustainEditAction(LumatoneController* controller, bool newValue)
    : LumatoneAction(controller, "InvertSustainEdit"), newData(newValue)
{
    previousData = controller->getInvertSustain();
}

bool InvertSustainEditAction::perform()
{
    controller->invertSustainPedal(newData);
    return true;
}

bool InvertSustainEditAction::undo()
{
    controller->invertSustainPedal(previousData);
    return true;
}

