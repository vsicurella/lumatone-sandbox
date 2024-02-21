#include "lumatone_action.h"
#include "../data/application_state.h"


LumatoneEditor::UndoableAction::UndoableAction(LumatoneApplicationState* stateIn, juce::String nameIn)
    : state(stateIn)
    , name(nameIn)
{}

