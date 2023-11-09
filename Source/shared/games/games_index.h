/*

    File: games_index.h
    Author: Vito
    Date: 2023/11/07

*/

#ifndef LUMATONE_GAMES_INDEX
#define LUMATONE_GAMES_INDEX

#include <JuceHeader.h>

namespace LumatoneSandbox
{
enum class GameName
{
    NoGame,
    RandomColors,
    HexRings,
    HexagonAutomata
};

static juce::String GameNameToString(LumatoneSandbox::GameName name)
{
    switch (name)
    {
    case LumatoneSandbox::GameName::RandomColors:
        return juce::String("RandomColors");
    case LumatoneSandbox::GameName::HexRings:
        return juce::String("HexRings");
    case LumatoneSandbox::GameName::HexagonAutomata:
        return juce::String("HexagonAutomata");

    default:
        return juce::String("NoGame");
    }
}

static GameName GameNameFromString(juce::String gameName)
{
    if (gameName == "RandomColors")
        return GameName::RandomColors;
    if (gameName == "HexRings")
        return GameName::HexRings;
    if (gameName == "HexagonAutomata")
        return GameName::HexagonAutomata;

    return GameName::NoGame;
}
}

#endif // LUMATONE_GAMES_INDEX
