/*

    File: hexagon_automata_game_state.h
    Author: Vito
    Date: 2023/11/01

*/

#ifndef LUMATONE_HEX_AUTOMATA_GAME_STATE_H
#define LUMATONE_HEX_AUTOMATA_GAME_STATE_H

#include "./hexagon_automata_types.h"
#include "./hexagon_automata_board_state.h"

#include "../game_base_state.h"
#include "../games_index.h"

namespace HexagonAutomata
{
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

    int getTicksPerGeneration() const { return ticksPerGeneration; }

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

    int ticksPerGeneration = 10;
};
    
}

#endif // LUMATONE_HEX_AUTOMATA_GAME_STATE_H
