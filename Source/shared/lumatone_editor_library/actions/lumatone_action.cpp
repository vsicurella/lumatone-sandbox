#include "lumatone_action.h"
#include "../data/application_state.h"


LumatoneAction::LumatoneAction(LumatoneApplicationState* stateIn, juce::String nameIn)
    : state(stateIn)
    , name(nameIn)
{
    if (state)
    {
        octaveBoardSize = state->getOctaveBoardSize();
        numOctaveBoards = state->getNumBoards();
    }
}
