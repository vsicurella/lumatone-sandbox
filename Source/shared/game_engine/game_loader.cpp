/*

    File: game_loader.h
    Author: Vito
    Date: 2023/11/08

*/

#include "./game_loader.h"

#include "../games/games_index.h"

#include "../lumatone_editor_library/device/lumatone_controller.h"

#include "../games/random_colors/random_colors.h"
#include "../games/hex_rings/hex_rings.h"
#include "../games/hexagon_automata/hexagon_automata.h"

LumatoneSandboxGameBase* LumatoneSandboxGameLoader::CreateGameInstance(juce::String gameId, LumatoneGameEngineState& gameEngineState)
{
    LumatoneSandbox::GameName gameName = LumatoneSandbox::GameNameFromString(gameId);

    switch (gameName)
    {
    default:
    case LumatoneSandbox::GameName::NoGame:
        return nullptr;

    // Tried to use static cast but I think the forward declare messes it up
    case LumatoneSandbox::GameName::RandomColors:
        return dynamic_cast<LumatoneSandboxGameBase*>(new RandomColors(gameEngineState));

    case LumatoneSandbox::GameName::HexRings:
        return dynamic_cast<LumatoneSandboxGameBase*>(new HexRings(gameEngineState));

    case LumatoneSandbox::GameName::HexagonAutomata:
        return dynamic_cast<LumatoneSandboxGameBase*>(new HexagonAutomata::Game(gameEngineState));
    }
}
