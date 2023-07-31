/*
  ==============================================================================

    EditActions.cpp
    Created: 9 May 2021 9:49:31pm
    Author:  hsstraub

  ==============================================================================
*/
#include "edit_actions.h"

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
    : LumatoneAction(controller)
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
    juce::Colour newColour ) 
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
            // Send to device
            controller->sendKeyParam(
                boardId,
                keyIndex,
                newData);

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

SectionEditAction::SectionEditAction(LumatoneController* controller, int boardIndexIn, const LumatoneBoard& newSectionValue)
    : LumatoneAction(controller)
    , boardId(boardIndexIn + 1)
    , newData(newSectionValue)
{
    previousData = *controller->getBoard(boardId);
}

bool SectionEditAction::isValid() const
{
    return boardId > 0 && boardId <= numOctaveBoards;
}

bool SectionEditAction::perform()
{
    if (boardId >= 0 && boardId < NUMBEROFBOARDS)
    {
        // Send to device
        controller->sendAllParamsOfBoard(boardId, &newData, true);

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
        controller->sendAllParamsOfBoard(boardId, &previousData, true);

        // Notify that there are changes: in calling function
        return true;
    }
    else
    {
        jassertfalse;
        return false;
    }

}

// ==============================================================================
// Implementation of InvertFootControllerEditAction

InvertFootControllerEditAction::InvertFootControllerEditAction(LumatoneController* controller, bool newValue)
    : LumatoneAction(controller), newData(newValue)
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
    : LumatoneAction(controller), newData(newValue)
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
    : LumatoneAction(controller), newData(newValue)
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

