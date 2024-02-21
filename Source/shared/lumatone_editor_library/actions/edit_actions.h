/*
  ==============================================================================

    EditActions.h
    Created: 9 May 2021 9:49:13pm
    Author:  hsstraub

  ==============================================================================
*/

#ifndef LUMATONE_EDITOR_UNDOABLE_ACTIONS_H
#define LUMATONE_EDITOR_UNDOABLE_ACTIONS_H

#include "./lumatone_action.h"
#include "../data/lumatone_layout.h"

#define MAX_KEY_FLAGS 280

class LumatoneApplicationState;

namespace LumatoneEditor 
{

using KeyUpdateFlag = unsigned short;
enum KeyUpdateFlagName : KeyUpdateFlag
{
    None = 0b0,
    Note = 0b1,
    Chnl = 0b10,
    Type = 0b100,
    CCIn = 0b1000,
    Colr = 0b10000,

    Prm  = 0b01111,
    All  = 0b11111
};

//======================================+
class LayoutAction : public Action
{
public:
    enum class Type
    {
        None,
        KeyParamUpdate,
        KeyColourUpdate,
        KeyConfigUpdate,
        MultiKeyUpdate,
        BoardUpdate,
        // LayoutUpdate
    };

public:
    Type type;
    LayoutAction(Type typeIn=Type::None, int boardIndexIn = -1, int keyIndexIn = -1);
    LayoutAction(const LayoutAction& other);
    ~LayoutAction() {}

    void operator=(const LayoutAction& other);

    // void setData(const LumatoneLayout& layout, Type typeIn, int boardIndexIn = -1, int keyIndexIn = -1);
    void setData(const LumatoneKey& keyData, Type typeIn, int boardIndexIn = -1, int keyIndexIn = -1);
    void setData(juce::Colour newColour, int boardIndexIn, int keyIndexIn);
    void setData(const LumatoneBoard& boardData, int boardIndexIn);
    void setData(const juce::Array<MappedLumatoneKey>&);

    void setFlags(const KeyUpdateFlag* newFlags, int numNewFlags, int flagStartIndex=0);
    
    bool perform(LumatoneApplicationState*) override;
    
    bool isValid(LumatoneApplicationState*) const;

protected:
    LumatoneLayout layout;
    int boardIndex = -1;
    int keyIndex = -1;

    LumatoneEditor::KeyUpdateFlag flags[MAX_KEY_FLAGS];
};

class SingleNoteAssignAction : public LumatoneEditor::LayoutAction 
                             , public LumatoneEditor::UndoableAction
{
public:
    SingleNoteAssignAction(
        LumatoneApplicationState* state,
        int boardIndexIn,
        int keyIndexIn,
        const LumatoneKey& newKeyData,
        bool setKeyType,
        bool setChannel, 
        bool setNote,
        bool setColour,
        bool setCCPolarity
    );

    SingleNoteAssignAction(const SingleNoteAssignAction& second)
        : LumatoneEditor::LayoutAction(second)
        // : LumatoneEditor::LayoutAction(static_cast<const LumatoneEditor::LayoutAction&>(second))
        , LumatoneEditor::UndoableAction(second.state, "SingleNoteAssign")
        , previousData(second.previousData)
    {}

    SingleNoteAssignAction(
        LumatoneApplicationState* state,
        int boardId,
        int keyIndex,
        juce::Colour newColour
    );

    bool perform() override;
    bool undo() override;
    
    int getSizeInUnits() override { return sizeof(SingleNoteAssignAction); }

private:
    LumatoneKey previousData;
};

class SectionEditAction : public LumatoneEditor::LayoutAction
                        , public LumatoneEditor::UndoableAction
{
public:
    SectionEditAction(LumatoneApplicationState* state, int boardIndexIn, const LumatoneBoard& newSectionValue, bool bufferKeyUpdates=false);

    SectionEditAction(const SectionEditAction& second)
        : LumatoneEditor::LayoutAction(second)
        , LumatoneEditor::UndoableAction(second.state, "SectionEditAction")
        , previousData(second.previousData)
        , useKeyBuffer(second.useKeyBuffer)
    {}

    bool perform() override;
    bool undo() override;
    int getSizeInUnits() override { return sizeof(SectionEditAction); }

private:
    LumatoneBoard previousData;

    bool useKeyBuffer;
};

class MultiKeyAssignAction : public LumatoneEditor::LayoutAction 
                            , public LumatoneEditor::UndoableAction
{
public:
    MultiKeyAssignAction(LumatoneApplicationState* state, const juce::Array<MappedLumatoneKey>& updatedKeys, bool setConfig=true, bool setColour=true, bool bufferKeyUpdates=false);
    MultiKeyAssignAction(const MultiKeyAssignAction& copy)
        : LumatoneEditor::LayoutAction(copy)
        , LumatoneEditor::UndoableAction(copy.state, "MultiKeyAssign")
        , previousKeys(copy.previousKeys)
        , useKeyBuffer(copy.useKeyBuffer)
    {}

    bool perform() override;
    bool undo() override;

    int getSizeInUnits() override { return sizeof(MultiKeyAssignAction); }

private:
    bool useKeyBuffer = false;

    juce::Array<MappedLumatoneKey> previousKeys;
};

}

#endif // LUMATONE_EDITOR_UNDOABLE_ACTIONS_H
