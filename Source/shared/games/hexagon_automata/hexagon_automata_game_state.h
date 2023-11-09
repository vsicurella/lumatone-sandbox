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
    Classic = 0,
    Sequencer
};

enum class GenerationMode
{
    Synchronous,
    Asynchronous
};

struct ID
{
    inline static const juce::Identifier GameId = juce::Identifier("HexagonAutomataGame");

    inline static const juce::Identifier CellStates = juce::Identifier("BoardState");

    inline static const juce::Identifier GameMode = juce::Identifier("GameMode");                   // Classic, Sequencer
    inline static const juce::Identifier GenerationMode = juce::Identifier("GenerationMode");       // Sync, Async

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

struct GameOptions
{
    GameMode        mode            = GameMode::Classic;
    GenerationMode  generationMode  = GenerationMode::Synchronous;

    int syncGenerationMs    = 1000;
    int asyncGenerationMs   = 1200;

    juce::String bornRules;
    juce::String surviveRules;
    juce::String neighborsShape;

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

    // virtual void resetState();
    // MappedHexState getMappedCell(int cellNum);

    void setAliveColour(juce::Colour newColour);
    juce::Colour getAliveColour() const { return aliveColour; }

    void setDeadColour(juce::Colour newColour);
    juce::Colour getDeadColour() const { return deadColour; }

    // void setBornSurviveRules(juce::Array<int> bornNums, juce::Array<int> surviveNums);
    void setBornSurviveRules(juce::String bornInput, juce::String surviveInput);

    void setNeighborDistance(int distance);

    void setTicksPerSyncGeneration(int ticks);
    void setTicksPerAsyncGeneration(int ticks);

    void setOptions(GameOptions options);
    GameOptions getOptions() const { return static_cast<GameOptions>(*this); }

protected:
    virtual juce::ValueTree loadStateProperties(juce::ValueTree stateIn) override;
    virtual void handleStatePropertyChange(juce::ValueTree stateIn, const juce::Identifier& property) override;

protected:

    juce::CriticalSection lock;

    juce::Array<MappedHexState> populatedCells;

    juce::Array<Hex::Point> neighborsVector;

    int ticksPerSyncGeneration = 10;
    int ticksToNextSyncCellUpdate = 0;

    int ticksPerAsyncGeneration = 15;

    int ticksPerCellUpdate = 1;
    int ticksToNextCellUpdate = 0;
};
    
}

#endif // LUMATONE_HEX_AUTOMATA_GAME_STATE_H
