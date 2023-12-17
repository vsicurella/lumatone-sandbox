#ifndef LUMATONE_HEXAGON_AUTOMATA_TYPES
#define LUMATONE_HEXAGON_AUTOMATA_TYPES

#include <JuceHeader.h>

namespace HexagonAutomata
{

enum class GameMode
{
    None = 0,
    Classic,
    Sequencer
};

static juce::String GameModeToString(HexagonAutomata::GameMode mode)
{
    switch (mode)
    {
    default:
    case HexagonAutomata::GameMode::None:
        return juce::String("None");
    case HexagonAutomata::GameMode::Classic:
        return juce::String("Classic");
    case HexagonAutomata::GameMode::Sequencer:
        return juce::String("Sequencer");
    }
}

static HexagonAutomata::GameMode GameModeFromString(juce::String mode)
{
    if (mode == "Classic")
        return HexagonAutomata::GameMode::Classic;
    if (mode == "Sequencer")
        return HexagonAutomata::GameMode::Sequencer;
    
    return GameMode::None;
}

enum class GenerationMode
{
    None = 0,
    Synchronous,
    Asynchronous
};

static juce::String GenerationModeToString(HexagonAutomata::GenerationMode mode)
{
    switch (mode)
    {
    default:
    case HexagonAutomata::GenerationMode::None:
        return juce::String("None");
    case HexagonAutomata::GenerationMode::Synchronous:
        return juce::String("Synchronous");
    case HexagonAutomata::GenerationMode::Asynchronous:
        return juce::String("Asynchronous");
    }
}

static HexagonAutomata::GenerationMode GenerationModeFromString(juce::String mode)
{
    if (mode == "Synchronous")
        return HexagonAutomata::GenerationMode::Synchronous;
    if (mode == "Asynchronous")
        return HexagonAutomata::GenerationMode::Asynchronous;
    
    return HexagonAutomata::GenerationMode::None;
}

enum class RulesMode
{
    None = 0,
    BornSurvive,
    BornMidNearSurvive,
    BornMidFarSurvive,
    SpiralRule,
    BzReactionRule
};

static juce::String RulesModeToString(HexagonAutomata::RulesMode mode)
{
    switch (mode)
    {
    default:
    case HexagonAutomata::RulesMode::None:
        return juce::String("None");
    case HexagonAutomata::RulesMode::BornSurvive:
        return juce::String("BornSurvive");
    case HexagonAutomata::RulesMode::BornMidNearSurvive:
        return juce::String("BornMidNearSurvive");
    case HexagonAutomata::RulesMode::BornMidFarSurvive:
        return juce::String("BornMidFarSurvive");
    case HexagonAutomata::RulesMode::SpiralRule:
        return juce::String("SpiralRule");
    case HexagonAutomata::RulesMode::BzReactionRule:
        return juce::String("BzReactionRule");
    }
}

static HexagonAutomata::RulesMode RulesModeFromString(juce::String mode)
{
    if (mode == "BornSurvive")
        return HexagonAutomata::RulesMode::BornSurvive;
    if (mode == "BornMidNearSurvive")
        return HexagonAutomata::RulesMode::BornMidNearSurvive;
    if (mode == "BornMidFarSurvive")
        return HexagonAutomata::RulesMode::BornMidNearSurvive;
    if (mode == "SpiralRule")
        return HexagonAutomata::RulesMode::SpiralRule;
    if (mode == "BzReactionRule")
        return HexagonAutomata::RulesMode::BzReactionRule;
    
    return HexagonAutomata::RulesMode::None;
}

enum class ClockMode
{
    None = 0,
    Engine,
    MidiClockClient
};

static juce::String ClockModeToString(HexagonAutomata::ClockMode mode)
{
    switch (mode)
    {
    default:
    case HexagonAutomata::ClockMode::None:
        return juce::String("None");
    case HexagonAutomata::ClockMode::Engine:
        return juce::String("Engine");
    case HexagonAutomata::ClockMode::MidiClockClient:
        return juce::String("MidiClockClient");
    }
}

static HexagonAutomata::ClockMode ClockModeFromString(juce::String mode)
{
    if (mode == "Engine")
        return HexagonAutomata::ClockMode::Engine;
    if (mode == "MidiClockClient")
        return HexagonAutomata::ClockMode::MidiClockClient;
    
    return ClockMode::None;
}

struct ID
{
    inline static const juce::Identifier GameId = juce::Identifier("HexagonAutomataGame");

    inline static const juce::Identifier CellStates = juce::Identifier("BoardState");

    inline static const juce::Identifier GameMode = juce::Identifier("GameMode");                   // Classic, Sequencer
    inline static const juce::Identifier GenerationMode = juce::Identifier("GenerationMode");       // Sync, Async
    inline static const juce::Identifier RulesMode = juce::Identifier("RulesMode");                 // BornSurvive, SpiralRule
    inline static const juce::Identifier ClockMode = juce::Identifier("ClockMode");                 // Engine, Midi Clock
    inline static const juce::Identifier NoSustainPassThrough = juce::Identifier("NoSustainPassThrough");

    inline static const juce::Identifier GenerationMs = juce::Identifier("GenerationMs");           // Ms per generation
    inline static const juce::Identifier GenerationBpm = juce::Identifier("GenerationBpm");          // Run generation timer at this rate
    inline static const juce::Identifier GenerationQnRatio = juce::Identifier("GenerationQnRatio");      // Multiply generation timer by this rate
    
    inline static const juce::Identifier BornRule = juce::Identifier("BornRule");                   // number string
    inline static const juce::Identifier SurviveRule = juce::Identifier("SurviveRule");             // number string
    inline static const juce::Identifier NeighborShape = juce::Identifier("NeighborShape");         // number string
    
    inline static const juce::Identifier AliveColour = juce::Identifier("AliveColour");             // Classic mode cell colour
    inline static const juce::Identifier DeadColour = juce::Identifier("DeadColour");               // Classic mode background colour
    inline static const juce::Identifier BlendMode = juce::Identifier("BlendMode");                 // Classic mode color blend mode
    
    inline static const juce::Identifier AliveBrightness = juce::Identifier("AliveBrightness");     // Sequencer mode alive cell brightness amount
    inline static const juce::Identifier DeadBrightness = juce::Identifier("DeadBrightness");       // Sequencer mode dead cell brightness amount
};
}

#endif
