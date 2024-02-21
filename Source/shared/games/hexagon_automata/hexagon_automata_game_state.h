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
    properties.add(HexagonAutomata::ID::GenerationMs);
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

    ClockMode       clockMode       = ClockMode::Engine;
    bool noSustainPassThrough       = false;

    int generationMs    = 1000;

    float bpm = 100;
    float qnRatio = 1.0f;

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
    State(LumatoneGameEngineState& gameEngineStateIn);
    // State(std::shared_ptr<LumatoneLayout> layoutIn, LumatoneGameEngineState& gameEngineStateIn);
    // State(const State& copy, LumatoneGameEngineState& gameEngineStateIn);

    GameMode getGameMode() const { return gameMode; }
    virtual void setGameMode(GameMode modeIn);

    GenerationMode getGenerationMode() const { return generationMode; }
    virtual void setGenerationMode(GenerationMode newMode);

    RulesMode getRulesMode() const { return rulesMode; }
    virtual void setRulesMode(RulesMode newMode);

    ClockMode getClockMode() const { return clockMode; }
    virtual void setClockMode(ClockMode newMode);

    bool isNoSustainPassthroughOn() const { return noSustainPassThrough; }
    virtual void setNoSustainPassthrough(bool passThroughWithNoSustain);

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

    float getGenerationMs() const { return generationMs; }
    float getGenerationBpm() const { return msecToBpm(generationMs); }
    virtual void setGenerationMs(float msecValue);
    virtual void setGenerationBpm(float bpmValue);
    
    virtual void setGenerationQN(float qnRatio); // Quarter Note ratio - for MIDI clock mode

    int getTicksPerGeneration() const { return ticksPerGeneration; }

    void setOptions(GameOptions options);
    GameOptions getOptions() const { return static_cast<GameOptions>(*this); }

protected:
    void updateGenerationMsTicks();
    void updateGenerationClockTime();

protected:
    virtual juce::ValueTree loadStateProperties(juce::ValueTree stateIn) override;
    virtual void handleStatePropertyChange(juce::ValueTree stateIn, const juce::Identifier& property) override;

protected:

    int ticksPerGeneration = 10;
    int ticksPerQuarterNote = 0;
};
    
}

#endif // LUMATONE_HEX_AUTOMATA_GAME_STATE_H
