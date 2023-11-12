/*

    File: hexagon_automata_game_state.h
    Author: Vito
    Date: 2023/11/01

*/

#ifndef LUMATONE_HEX_AUTOMATA_GAME_STATE_H
#define LUMATONE_HEX_AUTOMATA_GAME_STATE_H

#include "./hexagon_automata_board_state.h"

#include "../game_base_state.h"
#include "../games_index.h"

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
    BornMidFarSurvive
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
    
    return HexagonAutomata::RulesMode::None;
}

struct ID
{
    inline static const juce::Identifier GameId = juce::Identifier("HexagonAutomataGame");

    inline static const juce::Identifier CellStates = juce::Identifier("BoardState");

    inline static const juce::Identifier GameMode = juce::Identifier("GameMode");                   // Classic, Sequencer
    inline static const juce::Identifier GenerationMode = juce::Identifier("GenerationMode");       // Sync, Async
    inline static const juce::Identifier RulesMode = juce::Identifier("RulesMode");            // BornSurvive, LifeArray

    inline static const juce::Identifier SyncGenTime = juce::Identifier("SyncGenTime");             // Ms per sync. generation
    inline static const juce::Identifier AsyncGenTime = juce::Identifier("AsyncGenTime");           // Ms per async. generation
    
    inline static const juce::Identifier BornRule = juce::Identifier("BornRule");                   // number string
    inline static const juce::Identifier SurviveRule = juce::Identifier("SurviveRule");             // number string
    inline static const juce::Identifier NeighborShape = juce::Identifier("NeighborShape");         // number string
    
    inline static const juce::Identifier AliveColour = juce::Identifier("AliveColour");             // Classic mode cell colour
    inline static const juce::Identifier DeadColour = juce::Identifier("DeadColour");               // Classic mode background colour
    inline static const juce::Identifier BlendMode = juce::Identifier("BlendMode");                 // Classic mode color blend mode
    
    inline static const juce::Identifier AliveBrightness = juce::Identifier("AliveBrightness");     // Sequencer mode alive cell brightness amount
    inline static const juce::Identifier DeadBrightness = juce::Identifier("DeadBrightness");       // Sequencer mode dead cell brightness amount
};

static juce::Array<juce::Identifier> GetStateProperties()
{
    juce::Array<juce::Identifier> properties;
    properties.add(HexagonAutomata::ID::GameMode);
    properties.add(HexagonAutomata::ID::GenerationMode);
    properties.add(HexagonAutomata::ID::RulesMode);
    properties.add(HexagonAutomata::ID::SyncGenTime);
    properties.add(HexagonAutomata::ID::AsyncGenTime);
    properties.add(HexagonAutomata::ID::BornRule);
    properties.add(HexagonAutomata::ID::SurviveRule);
    properties.add(HexagonAutomata::ID::NeighborShape);
    properties.add(HexagonAutomata::ID::AliveColour);
    properties.add(HexagonAutomata::ID::DeadColour);
    properties.add(HexagonAutomata::ID::BlendMode);
    properties.add(HexagonAutomata::ID::AliveBrightness);
    properties.add(HexagonAutomata::ID::DeadBrightness);
    return properties;
}

struct GameOptions
{
    GameMode        gameMode        = GameMode::Classic;
    GenerationMode  generationMode  = GenerationMode::Synchronous;
    RulesMode       rulesMode       = RulesMode::BornSurvive;

    int syncGenerationMs    = 1000;
    // int asyncGenerationMs   = 1200;

    float bpm = 100;

    juce::String bornRules = juce::String("2");
    juce::String surviveRules = juce::String("3,4");
    juce::String neighborsShape = juce::String("1");

    juce::Colour aliveColour;
    juce::Colour deadColour;
    int blendMode; // todo

    float aliveBrightness = 1.0f;
    float deadBrightness = 0.125f;
};

static juce::Array<juce::Identifier> GetStateProperties();

struct State : public LumatoneGameBaseState 
             , public HexagonAutomata::BoardState
             , public HexagonAutomata::GameOptions
{
    State(std::shared_ptr<LumatoneLayout> layoutIn, juce::ValueTree engineStateIn);
    // State(const State& copy, juce::ValueTree engineStateIn);

    GameMode getGameMode() const { return gameMode; }
    virtual void setGameMode(GameMode modeIn);

    GenerationMode getGenerationMode() const { return generationMode; }
    virtual void setGenerationMode(GenerationMode newMode);

    RulesMode getRulesMode() const { return rulesMode; }
    virtual void setRulesMode(RulesMode newMode);

    juce::Colour getAliveColour() const { return aliveColour; }
    virtual void setAliveColour(juce::Colour newColour);

    juce::Colour getDeadColour() const { return deadColour; }
    virtual void setDeadColour(juce::Colour newColour);

    juce::String getBornRules() const { return bornRules; }
    juce::String getSurviveRules() const { return surviveRules; }
    // void setBornSurviveRules(juce::Array<int> bornNums, juce::Array<int> surviveNums);
    virtual void setBornSurviveRules(juce::String bornInput, juce::String surviveInput); 

    juce::String getNeighborShape() const { return neighborsShape; }
    virtual void setNeighborDistance(int distance);

    float getGenerationMs() const { return syncGenerationMs; }
    float getGenerationBpm() const { return engineState.msecToBpm(syncGenerationMs); }
    virtual void setGenerationMs(float msecValue);
    virtual void setGenerationBpm(float bpmValue);

    // int getSyncGenTicks() const { return ticksPerSyncGeneration; }
    // virtual void setTicksPerSyncGeneration(int ticks);
    
    // int getAsyncGenTicks() const { return ticksPerAsyncGeneration; }
    // virtual void setTicksPerAsyncGeneration(int ticks);

    void setOptions(GameOptions options);
    GameOptions getOptions() const { return static_cast<GameOptions>(*this); }

protected:
    void updateGenerationTickRate();

protected:
    // TODO - this should be the one that's used
    // virtual juce::ValueTree loadStateProperties(juce::ValueTree stateIn) override;
    virtual void handleStatePropertyChange(juce::ValueTree stateIn, const juce::Identifier& property) override;

protected:

    juce::Array<MappedHexState, juce::CriticalSection> populatedCells;

    int ticksPerSyncGeneration = 10;
    int ticksToNextSyncCellUpdate = 0;

    // int ticksPerAsyncGeneration = 15;

    int ticksPerCellUpdate = 1;
    int ticksToNextCellUpdate = 0;
};
    
}

#endif // LUMATONE_HEX_AUTOMATA_GAME_STATE_H
