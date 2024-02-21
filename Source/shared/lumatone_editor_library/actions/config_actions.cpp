/*
  ==============================================================================

    EditActions.cpp
    Created: 9 May 2021 9:49:31pm
    Author:  hsstraub

  ==============================================================================
*/
#include "config_actions.h"
#include "../data/application_state.h"

using namespace LumatoneEditor;

// ==============================================================================
// Implementation of InvertFootControllerEditAction

InvertFootControllerEditAction::InvertFootControllerEditAction(LumatoneApplicationState* stateIn, bool newValue)
    : UndoableAction(stateIn, "InvertFootControllerEdit")
    , newData(newValue)
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
    : UndoableAction(stateIn, "ExprPedlSensitivity"), newData(newValue)
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
    : UndoableAction(stateIn, "InvertSustainEdit"), newData(newValue)
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
