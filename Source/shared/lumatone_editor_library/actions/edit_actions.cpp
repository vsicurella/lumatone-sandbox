/*
  ==============================================================================

    EditActions.cpp
    Created: 9 May 2021 9:49:31pm
    Author:  hsstraub

  ==============================================================================
*/
#include "edit_actions.h"
#include "../data/application_state.h"

using namespace LumatoneEditor;

static bool performSingleNoteAssign(LumatoneApplicationState* state
                                   , int boardIndex
                                   , int keyIndex
                                   , const LumatoneKey& newData
                                   , KeyUpdateFlag updateFlag
    )
{
    if (state->getMappingData()->isKeyCoordValid(keyIndex, boardIndex))
    {
        if (updateFlag == KeyUpdateFlagName::Colr)
        {
            state->setKeyColour(newData.getColour(), boardIndex + 1, keyIndex);
        }
        else if (updateFlag > 0)
        {
            LumatoneKey resultKey = state->getKey(boardIndex, keyIndex);

            if (updateFlag & KeyUpdateFlagName::Note)
            {
                resultKey.setNoteOrCC(newData.getMidiNumber());
            }
            if (updateFlag & KeyUpdateFlagName::Chnl)
            {
                resultKey.setChannelNumber(newData.getMidiChannel());
            }
            if (updateFlag & KeyUpdateFlagName::Type)
            {
                resultKey.setKeyType(newData.getType());
            }
            if (updateFlag & KeyUpdateFlagName::CCIn)
            {
                resultKey.setDefaultCCFader(newData.isCCFaderDefault());
            }
            if (updateFlag & KeyUpdateFlagName::Colr)
            {
                resultKey.setColour(newData.getColour());
            }

            state->setKey(resultKey, boardIndex + 1, keyIndex);
        }
        // Notfy that there are changes: in calling function

        return true;
    }
    else
    {
        jassertfalse;
        return false;
    }
}

static bool performSectionAssign(LumatoneApplicationState* state
    , int boardIndex
    , const LumatoneBoard& newData
    , KeyUpdateFlag flag=KeyUpdateFlagName::All
    )
{
    if (flag == 0)
        return false;

    if (boardIndex >= 0 && boardIndex < state->getNumBoards())
    {
        if (flag == KeyUpdateFlagName::All)
            state->setBoard(newData, boardIndex + 1);

        else
        {
            const LumatoneBoard& board = state->getBoard(boardIndex);

            if (flag == KeyUpdateFlagName::Colr)
            {
                for (int keyIndex = 0; keyIndex < board.getNumKeys(); keyIndex++)
                {
                    state->setKeyColour(newData.getKey(keyIndex).getColour(), board.getBoardIndex() + 1, keyIndex);
                }
            }
            else
            {
                // TODO set according to flag 
                for (int keyIndex = 0; keyIndex < board.getNumKeys(); keyIndex++)
                {
                    state->setKeyConfig(newData.getKey(keyIndex), board.getBoardIndex() + 1, keyIndex);
                }
            }
        }
    }
}

static bool performKeySelectionAssign(LumatoneApplicationState* state
    , const juce::Array<MappedLumatoneKey> keyUpdates
    , KeyUpdateFlag flag = KeyUpdateFlagName::All
    )
{
    if (flag == 0)
        return false;

    if (flag == KeyUpdateFlagName::Colr)
    {
        for (const MappedLumatoneKey& update : keyUpdates)
        {
            state->setKeyColour(update.getColour(), update.boardIndex + 1, update.keyIndex);
        }
    }
    else if (flag == KeyUpdateFlagName::All)
    {
        for (const MappedLumatoneKey& update : keyUpdates)
        {
            state->setKey(update, update.boardIndex + 1, update.keyIndex);
        }
    }
    else
    {
        for (const MappedLumatoneKey& update : keyUpdates)
        {
            state->setKeyConfig(update, update.boardIndex + 1, update.keyIndex);
        }
    }
}

static bool performKeySelectionAssign(LumatoneApplicationState* state
    , const LumatoneLayout& keyUpdates
    , KeyUpdateFlag* flags
    )
{
    int keyLayoutIndex = 0;
    KeyUpdateFlag* flag;

    for (int boardIndex = 0; boardIndex < keyUpdates.getNumBoards(); boardIndex++)
    {
        const LumatoneBoard& board = keyUpdates.getBoard(boardIndex);
        for (int keyIndex = 0; keyIndex < board.getNumKeys(); keyIndex++)
        {
            flag = &flags[keyLayoutIndex];
            if (*flag == 0)
                {
                keyLayoutIndex++;
                continue;
                }

            const LumatoneKey& newData = board.getKey(keyIndex);
            if (*flag == KeyUpdateFlagName::Colr)
            {
                state->setKeyColour(newData.getColour(), boardIndex + 1, keyIndex);
            }
            else if (*flag == KeyUpdateFlagName::All)
            {
                state->setKey(newData, boardIndex + 1, keyIndex);
            }
            else
            {
                state->setKeyConfig(newData, boardIndex + 1, keyIndex);
            }

            keyLayoutIndex++;
        }
    }
}

bool LayoutAction::perform(LumatoneApplicationState *state)
{
    switch (type)
    {
    case LayoutAction::Type::KeyConfigUpdate:
    case LayoutAction::Type::KeyParamUpdate:
    case LayoutAction::Type::KeyColourUpdate:
    {
        return performSingleNoteAssign(
            state,
            boardIndex,
            keyIndex,
            layout.getKey(boardIndex, keyIndex),
            *flags
        );
    }
    case LayoutAction::Type::MultiKeyUpdate:
    {
        return performKeySelectionAssign(
            state,
            layout,
            flags
        );
    }
    case LayoutAction::Type::BoardUpdate:
    {
        return performSectionAssign(
            state,
            boardIndex,
            layout.getBoard(boardIndex)
        );
    }

    case LayoutAction::Type::None:
    default:
        return false;
    }
}

bool LayoutAction::isValid(LumatoneApplicationState* state) const
{
    switch (type)
    {
    case Type::KeyParamUpdate:
    case Type::KeyColourUpdate:
    case Type::KeyConfigUpdate:
        return state->getMappingData()->isKeyCoordValid(keyIndex, boardIndex);

    case Type::BoardUpdate:
        return boardIndex >= 0 && boardIndex < state->getNumBoards();

    case Type::MultiKeyUpdate:
        return true; // checked on setData

    default:
        return false;
    }
}

void LayoutAction::setFlags(const KeyUpdateFlag *newFlags, int numNewFlags, int flagStartIndex)
{
    int flagOverflow = numNewFlags + flagStartIndex - MAX_KEY_FLAGS;
    int flagsLimit = numNewFlags - flagOverflow;
    for (int i = 0; i < flagsLimit; i++)
    {
        flags[i + flagStartIndex] = *(newFlags + i);
    }
}

void LayoutAction::operator=(const LayoutAction &other)
{
    switch (other.type)
    {
    case Type::KeyParamUpdate:
    case Type::KeyConfigUpdate:
        setData(other.layout.getKey(other.boardIndex, other.keyIndex), other.type, other.boardIndex, other.keyIndex);
        break;
    case Type::KeyColourUpdate:
        setData(other.layout.getKey(other.boardIndex, other.keyIndex).getColour(), other.boardIndex, other.keyIndex);
        break;
    case Type::BoardUpdate:
        setData(other.layout.getBoard(other.boardIndex), other.boardIndex);
        break;
    case Type::MultiKeyUpdate:
        type = Type::MultiKeyUpdate;
        layout = other.layout;
        break;
    }

    for (int i = 0; i < MAX_KEY_FLAGS; i++)
    {
        flags[i] = other.flags[i];
    }
}

LumatoneEditor::LayoutAction::LayoutAction(Type typeIn, int boardIndexIn, int keyIndexIn)
    : type(typeIn) 
    , boardIndex(boardIndexIn) 
    , keyIndex(keyIndexIn)
{
    for (int i = 0; i < MAX_KEY_FLAGS; i++)
    {
        flags[i] = 0;
    }
}

LayoutAction::LayoutAction(const LayoutAction &other)
{
    operator=(other);
}

void LayoutAction::setData(const LumatoneKey &keyData, Type typeIn, int boardIndexIn, int keyIndexIn)
{
    if (typeIn == Type::KeyParamUpdate)
    {
        type = Type::KeyParamUpdate;
        *flags = KeyUpdateFlagName::Prm;
    }
    else
    {
        type = Type::KeyConfigUpdate;
        *flags = KeyUpdateFlagName::All;
    }

    boardIndex = boardIndexIn;
    keyIndex = keyIndexIn;
    layout.setKey(keyData, boardIndex, keyIndex);
}

void LayoutAction::setData(juce::Colour newColour, int boardIndexIn, int keyIndexIn)
{
    type = Type::KeyColourUpdate;
    *flags = KeyUpdateFlagName::Colr;

    boardIndex = boardIndexIn;
    keyIndex = keyIndexIn;
    layout.setKeyColour(newColour, boardIndex, keyIndex);
}

void LayoutAction::setData(const LumatoneBoard &boardData, int boardIndexIn)
{
    type = Type::BoardUpdate;
    *flags = KeyUpdateFlagName::All;

    boardIndex = boardIndexIn;
    layout.setBoard(boardData, boardIndex);
}

void LayoutAction::setData(const juce::Array<MappedLumatoneKey> &updatesIn)
{
    type = Type::MultiKeyUpdate;
    for (auto update : updatesIn)
    {
        if (layout.isKeyCoordValid(update.keyIndex, update.boardIndex))
        {
            layout.setKey(update);

            // TODO support other board sizes?
            int layoutIndex = layout.getOctaveBoardSize() * update.boardIndex + update.keyIndex;
            flags[layoutIndex] = KeyUpdateFlagName::All;
        }

    }
}

// ==============================================================================
// Implementation of SingleNoteAssignAction
SingleNoteAssignAction::SingleNoteAssignAction(LumatoneApplicationState* stateIn
    , int boardIndexIn
    , int keyIndexIn
    , const LumatoneKey& newKeyData
    , bool setKeyType
    , bool setChannel
    , bool setNote
    , bool setColour
    , bool setCCPolarity
    )
    : LayoutAction(LayoutAction::Type::KeyConfigUpdate, boardIndexIn, keyIndexIn)
    , UndoableAction(stateIn, "SingleNoteAssign")
{
    KeyUpdateFlag flag = setNote
                                       | (setChannel << 1)
                                       | (setKeyType << 2)
                                       | (setCCPolarity << 3)
                                       | (setColour << 4);
    setFlags(&flag, 1);
    layout.setKey(newKeyData, boardIndex, keyIndex);
    previousData = state->getKey(boardIndex, keyIndex);
}

SingleNoteAssignAction::SingleNoteAssignAction(LumatoneApplicationState* stateIn, int boardIndexIn, int keyIndexIn, juce::Colour newColour)
    : SingleNoteAssignAction(stateIn, boardIndexIn, keyIndexIn,
                            state->getKey(boardIndexIn, keyIndexIn),
                            false, false, false, true, false)
{
    layout.setKeyColour(newColour, boardIndex, keyIndex);
}

bool SingleNoteAssignAction::perform()
{
    return LayoutAction::perform(state);
}

bool SingleNoteAssignAction::undo()
{
    return performSingleNoteAssign(state, boardIndex, keyIndex, previousData, *flags);
}

// ==============================================================================
// Implementation of SectionEditAction

SectionEditAction::SectionEditAction(LumatoneApplicationState* stateIn, int boardIndexIn, const LumatoneBoard& newSectionValue, bool bufferKeyUpdates)
    : UndoableAction(stateIn, "SectionEdit")
    , useKeyBuffer(bufferKeyUpdates)
{
    setData(newSectionValue, boardIndex);
    // TODO flags
    previousData = state->getBoard(boardIndexIn);
}

bool SectionEditAction::perform()
{
    return LayoutAction::perform(state);
}

bool SectionEditAction::undo()
{
    return performSectionAssign(
        state,
        boardIndex,
        layout.getBoard(boardIndex),
        KeyUpdateFlagName::All
    );
}


MultiKeyAssignAction::MultiKeyAssignAction(LumatoneApplicationState* stateIn, const juce::Array<MappedLumatoneKey>& updatedKeys, bool setConfigIn, bool setColourIn, bool bufferKeyUpdates)
    : UndoableAction(stateIn, "MultiKeyAssign")
    , useKeyBuffer(bufferKeyUpdates)
{
    setData(updatedKeys);

    for (auto key : updatedKeys)
    {
        auto coord = key.getKeyCoord();
        if (state->getMappingData()->isKeyCoordValid(coord))
        {
            const LumatoneKey& key = state->getKey(coord.boardIndex, coord.keyIndex);
            previousKeys.add(MappedLumatoneKey(key, coord));
        }
    }
}

bool MultiKeyAssignAction::perform()
{
    return LayoutAction::perform(state);
}

bool MultiKeyAssignAction::undo()
{
    auto undoAction = LayoutAction();
    undoAction.setData(previousKeys);
    return undoAction.perform(state);
}
