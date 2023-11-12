#pragma once

#include "./hexagon_automata_game_state.h"

#include "../game_base.h"
#include "../../debug/LumatoneSandboxLogger.h"

namespace HexagonAutomata
{
class Renderer;
struct NeighborFunction;

class Game : public LumatoneSandboxGameBase
           , public HexagonAutomata::State
           , private LumatoneSandboxLogger
{
public:

    Game(juce::ValueTree engineStateIn, LumatoneController* controller);
    // Game(LumatoneController* controller, const HexagonAutomata::BoardState& stateIn);
    
    void reset(bool clearQueue) override;

    void nextTick() override;
    void pauseTick() override;

    LumatoneAction* renderFrame() const override;

public:
    void addSeed(Hex::Point coord, bool triggerMidi=true);
    void addSeeds(juce::Array<Hex::Point> seedCoords, bool triggerMidi=true);
    void addSeeds(int numSeeds, bool triggerMidi=true);

    void clearCell(Hex::Point coord, bool triggerMidi=true);
    void clearCell(MappedHexState& cell, bool triggerMidi=true);
    void clearAllCells(bool triggerMidi=true);

public:
    void setGameMode(GameMode modeIn) override;
    void setGenerationMode(GenerationMode newMode) override;
    void setAliveColour(juce::Colour newColour) override;
    void setDeadColour(juce::Colour newColour) override;
    void setBornSurviveRules(juce::String bornInput, juce::String surviveInput) override;
    // void setNeighborDistance(int distance) override;

        
private:
    void updateNewCells();
    void updateCellStates();

private:

    void handleAnyNoteOn(int midiChannel, int midiNote, juce::uint8 velocity) override;

    void completeMappingLoaded(LumatoneLayout layout) override;

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

    void initializeLayoutContext();

protected:
    juce::ValueTree loadStateProperties(juce::ValueTree stateIn) override; // kludge?
    virtual void handleStatePropertyChange(juce::ValueTree stateIn, const juce::Identifier& property) override;

public:
    virtual LumatoneSandboxGameComponent* createController() override;

private:
    // juce::CriticalSection cellsLock;
    // juce::CriticalSection rulesLock;

    juce::Array<Hex::Point, juce::CriticalSection> neighborsVector;

    std::unique_ptr<HexagonAutomata::NeighborFunction> rules;
    std::unique_ptr<HexagonAutomata::Renderer> render;

    juce::Array<MappedHexState, juce::CriticalSection> currentFrameCells;
    juce::Array<MappedHexState, juce::CriticalSection> newCells;

    juce::Random random;
};
}

