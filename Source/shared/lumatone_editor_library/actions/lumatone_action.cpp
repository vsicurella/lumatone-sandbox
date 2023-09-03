#include "lumatone_action.h"
#include "../LumatoneController.h"


LumatoneAction::LumatoneAction(LumatoneController* controllerIn, juce::String nameIn)
    : controller(controllerIn)
    , name(nameIn)
{
    if (controller)
    {
        octaveBoardSize = controller->getOctaveBoardSize();
        numOctaveBoards = controller->getNumBoards();
    }
}
