#pragma once

#include "./hexagon_automata_game_state.h"

#include "../game_base.h"

namespace HexagonAutomata
{
class Renderer;
struct NeighborFunction;

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

class Game : public LumatoneSandboxGameBase
            , private HexagonAutomata::GameState
{
public:

    Game(LumatoneController* controller);
    Game(LumatoneController* controller, const HexagonAutomata::GameState& stateIn);
    
    void reset(bool clearQueue) override;

    void nextTick() override;
    void pauseTick() override;

    void setAliveColour(juce::Colour newColour);
    juce::Colour getAliveColour() const;

    void setDeadColour(juce::Colour newColour);
    juce::Colour getDeadColour() const;

    void setBornSurviveRules(juce::Array<int> bornNums, juce::Array<int> surviveNums);
    void setBornSurviveRules(juce::String bornInput, juce::String surviveInput);
    
    void setNeighborDistance(int distance);

    void updateNewCells();

    void setTicksPerSyncGeneration(int ticks);
    void setTicksPerAsyncGeneration(int ticks);

    LumatoneAction* renderFrame() const override;

    double getLockedFps() const { return 0; }

public:
    void addSeed(Hex::Point coord, bool triggerMidi=true);
    void addSeeds(juce::Array<Hex::Point> seedCoords, bool triggerMidi=true);
    void addSeeds(int numSeeds, bool triggerMidi=true);

    void clearCell(Hex::Point coord, bool triggerMidi=true);
    void clearCell(MappedHexState& cell, bool triggerMidi=true);
    void clearAllCells(bool triggerMidi=true);
        
private:
    void updateCellStates();

private:

    void handleAnyNoteOn(int midiChannel, int midiNote, juce::uint8 velocity) override;

    void completeMappingLoaded(LumatoneLayout layout);

private:

    // Apply cell updates to game state
    // Returns whether or not cell is still populated
    bool applyUpdatedCell(const MappedHexState& cellUpdate);

    // Produce midi note from cell and send immediately
    // Returns whether or not cell can be triggered
    bool triggerCellMidi(const MappedHexState& cell);

private:

    void resetState() override;

    void initialize();
    void redoCensus();

    void initializeLayoutContext();

private:

    juce::CriticalSection lock;
    juce::Array<MappedHexState> currentFrameCells;

    std::unique_ptr<HexagonAutomata::NeighborFunction> rules;
    std::unique_ptr<HexagonAutomata::Renderer> render;

    juce::Array<Hex::Point> neighborsVector;

    GameMode mode;
    GenerationMode generationMode = GenerationMode::Asynchronous;

    int ticks = 0;

    int ticksPerSyncGeneration = 10;
    int ticksToNextSyncCellUpdate = 0;

    int ticksPerAsyncGeneration = 15;

    int ticksPerCellUpdate = 1;
    int ticksToNextCellUpdate = 0;

    int maxUpdatesPerFrame = 10;

    int verbose = 0;

    juce::Array<MappedHexState> populatedCells;

    juce::Array<MappedHexState> newCells;
    // juce::Array<MappedHexState> diedCells;
    // juce::Array<MappedHexState> bornCells;
    // juce::Array<MappedHexState> agingCells;

    juce::Random random;
};
}

