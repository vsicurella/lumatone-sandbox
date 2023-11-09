#pragma once

#include "./hexagon_automata_game_state.h"

#include "../game_base.h"

namespace HexagonAutomata
{
class Renderer;
struct NeighborFunction;

class Game : public LumatoneSandboxGameBase
           , public HexagonAutomata::State
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
    void redoCensus();

    void initializeLayoutContext();

protected:
    virtual juce::ValueTree loadStateProperties(juce::ValueTree stateIn) override;
    virtual void handleStatePropertyChange(juce::ValueTree stateIn, const juce::Identifier& property) override;

public:
    virtual LumatoneSandboxGameComponent* createController() override;

private:

    std::unique_ptr<HexagonAutomata::NeighborFunction> rules;
    
    std::unique_ptr<HexagonAutomata::Renderer> render;

    juce::Array<MappedHexState> currentFrameCells;
    juce::Array<MappedHexState> newCells;

    juce::Random random;
};
}

