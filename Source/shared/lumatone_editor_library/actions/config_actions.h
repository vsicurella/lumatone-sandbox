/*
  ==============================================================================

    game_action.h
    Created: 30 Jan 2024 9:42pm
    Author:  Vincenzo

    Wrapper class for LumatoneAction data

  ==============================================================================
*/

#ifndef LUMATONE_EDITOR_CONFIG_ACTION_H
#define LUMATONE_EDITOR_CONFIG_ACTION_H

#include "./lumatone_action.h"

namespace LumatoneEditor
{
class InvertFootControllerEditAction : public LumatoneEditor::UndoableAction
{
public:
    InvertFootControllerEditAction(LumatoneApplicationState* state, bool newValue);

    InvertFootControllerEditAction(const InvertFootControllerEditAction& second)
        : LumatoneEditor::UndoableAction(second.state, "InvertFootControllerEdit")
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

class ExprPedalSensivityEditAction : public LumatoneEditor::UndoableAction
{
public:
    ExprPedalSensivityEditAction(LumatoneApplicationState* state, int newValue);

    ExprPedalSensivityEditAction(const ExprPedalSensivityEditAction& second)
        : LumatoneEditor::UndoableAction(second.state, "ExprPedalSensitivityEdit")
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

class InvertSustainEditAction : public LumatoneEditor::UndoableAction
{
public:
    InvertSustainEditAction(LumatoneApplicationState* state, bool newValue);
    
    InvertSustainEditAction(const InvertSustainEditAction& second)
        : LumatoneEditor::UndoableAction(second.state, "InvertSustainEdit")
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

#endif // LUMATONE_EDITOR_CONFIG_ACTION_H
