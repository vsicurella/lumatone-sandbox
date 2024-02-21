#pragma once

#include "./hexagon_automata_game_state.h"

#include "../game_base.h"
#include "../../debug/LumatoneSandboxLogger.h"

namespace HexagonAutomata
{
class Renderer;
struct Rules;

class Game : public LumatoneSandboxGameBase
           , public HexagonAutomata::State
{
public:

    Game(LumatoneGameEngineState& gameEngineStateIn);
    ~Game() override;
    
    bool reset(bool clearQueue) override;

    bool nextTick() override;
    bool pauseTick() override;

    LumatoneEditor::LayoutAction renderFrame() const override;

public:
    void addSeed(int cellNum, float healthIn=1.0f, bool triggerMidi=true);
    void addSeed(Hex::Point coord, float healthIn=1.0f, bool triggerMidi=true);
    void addSeeds(juce::Array<Hex::Point> seedCoords, bool triggerMidi=true);
    void addSeeds(int numSeeds, bool triggerMidi=true);

    void clearCell(const MappedHexState& cell, bool triggerMidi=true);
    void clearAllCells(bool triggerMidi=true);

public:
    void setGameMode(GameMode modeIn) override;
    void setGenerationMode(GenerationMode newMode) override;
    void setRulesMode(RulesMode modeIn) override;

    void setAliveColour(juce::Colour newColour) override;
    void setDeadColour(juce::Colour newColour) override;

    void setBornSurviveRules(juce::String bornInput, juce::String surviveInput) override;
    void setSpiralRule();
    void setReactionRule(int numStates, int k1, int k2, int speed);

private:
    void addToPopulation(MappedHexState& cell, MappedCellStates& population);
    bool removeFromPopulation(MappedHexState& cell, MappedCellStates& population);

    void updateUserInputCells();
    void updateCellStates();

    void addFramesToQueue(); // Turn into game base method?

private:
    // MIDI listener
    void handleNoteOn(int midiChannel, int midiNote, juce::uint8 velocity) override;
    void handleNoteOff(int midiChannel, int midiNote) override;

    void handleController(int channel, int ccNum, juce::uint8 value) override;
    void handleMidiClock(int quarterNoteInterval) override;
    void handleSustain(bool toggled) override;

    // Virtual Keyboard Listener
    void handleKeyDown(int keyNum) override;
    void handleKeyUp(int keyNum) override;

    // Editor Listener
    void completeMappingLoaded(const LumatoneLayout& layout) override;

private:
    void handleCellNoteOn(int cellNum, juce::uint8 velocity);
    void handleCellNoteOff(int cellNum);

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

private:

    void logSkippedFrame(juce::String method) const;
    void logCellState(juce::String method, juce::String message, const MappedCellStates& states) const;

protected:
    virtual void handleStatePropertyChange(juce::ValueTree stateIn, const juce::Identifier& property) override;

public:
    virtual LumatoneSandboxGameComponent* createController() override;

private:

    juce::CriticalSection frameLock;
    juce::CriticalSection inputLock;
    juce::CriticalSection stateLock;

    // Thread safety - only manipulate in game engine calls or use above locks
    MappedCellStates populatedCells;
    MappedCellStates nextPopulation;

    std::unique_ptr<HexagonAutomata::Rules> rules;
    std::unique_ptr<HexagonAutomata::Renderer> render;

    MappedCellStates currentFrameCells;
    int numUpdatesInFrame = 0;
    
    MappedCellStates newCells;
    MappedCellStates clearedCells;

    juce::Random random;

    bool gradientRender = false;

    // bool overrideClock = true;
    bool clockFlag = false;

    bool sustainIsOn = false;
};
}

