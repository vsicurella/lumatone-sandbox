/*
  ==============================================================================

    hexagon_automata_rules.h
    Created: 2023/11/01
    Author:  Vincenzo

  ==============================================================================
*/

#ifndef LUMATONE_HEXAGON_AUTOMATA_RULES_H
#define LUMATONE_HEXAGON_AUTOMATA_RULES_H

#include <JuceHeader.h>

#include "./hexagon_automata_cell_state.h"
#include "../../lumatone_editor_library/hex/hex_field.h"

namespace HexagonAutomata
{

class State;

using NeighborsShape = Hex::Matrix<juce::CriticalSection>;
using NeighborsShapeTemp = Hex::Matrix<juce::DummyCriticalSection>;

// TODO: Redo "newCells" part of algorithm 
// Have rule defining what cells should be born or eligible to be born
class Rules
{
public:
    Rules();
    Rules(juce::String neighborsShapeIn);
    Rules(NeighborsShapeTemp neighborsShapeIn);
    Rules(const HexagonAutomata::State& gameStateIn);

    virtual ~Rules() { }

    void setNeighborsShape(juce::String shapeInputIn);
    void setNeighborsShape(const NeighborsShape& shapeIn);
    virtual NeighborsShapeTemp getNeighborsShape() const;
    virtual NeighborsShapeTemp getDefaultNeighborsShape() const;

    // Return array of cells that are alive
    // virtual MappedCellStates getPopulation(const HexagonAutomata::State& gameState) const; 
    // Return Array of cells to run born rules on
    virtual MappedCellStates getEmptyNeighbors(const HexagonAutomata::State& gameState, const MappedCellStates& population) const;

    virtual MappedCellStates getNewCells(const HexagonAutomata::State& board, const MappedCellStates& population) const = 0;
    virtual MappedCellStates getUpdatedCells(const HexagonAutomata::State& board, const MappedCellStates& population) const = 0;
    
    // virtual juce::Array<MappedHexState> getNeighboringCells(const CellStates& population, const NeighborsShape& neighborShape) const = 0;

    const juce::CriticalSection& getLock() const { return lock; }

protected:
    virtual Hex::Matrix<juce::CriticalSection> createNeighborsShape(int distance=1) const;

    virtual float getLifeFactor(const MappedHexState& origin, const MappedCellStates& neighbors) const = 0;
    virtual bool generateNewLife(const MappedHexState& origin, const MappedCellStates& neighbors) const = 0;

protected:
    juce::CriticalSection lock;

    NeighborsShape neighborsShape;
};

struct BornSurviveRule : public Rules
{
    BornSurviveRule(int numBorn, int surviveLower, int surviveUpper);
    BornSurviveRule(juce::Array<int> bornNums, juce::Array<int> surviveNums);
    BornSurviveRule(juce::String bornString, juce::String surviveString);

    virtual ~BornSurviveRule() override { }

    virtual MappedCellStates getNewCells(const HexagonAutomata::State& board, const MappedCellStates& population) const override;
    virtual MappedCellStates getUpdatedCells(const HexagonAutomata::State& board, const MappedCellStates& population) const override;

    virtual float getLifeFactor(const MappedHexState& origin, const MappedCellStates& neighbors) const override;
    virtual bool generateNewLife(const MappedHexState& origin, const MappedCellStates& neighbors) const override;

    juce::Array<int> numsBorn;
    juce::Array<int> numsSurvive;
};

// Cells are born in the midpoint of the matching neighbor(s)
// struct BornMiddleSurviveRule : public BornArrayFunction
// {
//     BornMiddleSurviveRule(int numForBorn, int maxBorn, int surviveLower, int surviveUpper);
//     BornMiddleSurviveRule(juce::Array<int> numForBorn, juce::Array<int> bornMaxes, juce::Array<int> survieNums);
//     BornMiddleSurviveRule(juce::String numForBorn, juce::String bornMaxes, juce::String survieNums);

//     void setRoundUp(bool roundMidPointUp) { roundUp = roundMidPointUp; }

// protected:
//     bool roundUp = false;
    
//     juce::Array<int> numsForBorn;
//     juce::Array<int> maxNumsBorn;
//     juce::Array<int> numsSurvive;
// };

// struct BornMidNearSurviveRule : public BornArrayFunction
// {
//     BornMidNearSurviveRule(int numForBorn, int maxBorn, int surviveLower, int surviveUpper);
//     BornMidNearSurviveRule(juce::Array<int> numForBorn, juce::Array<int> bornMaxes, juce::Array<int> survieNums);
//     BornMidNearSurviveRule(juce::String numForBorn, juce::String bornMaxes, juce::String survieNums);

//     juce::Array<Hex::Point> getNewCells(const MappedHexState& parentCell, const MappedHexState* neighbors, int numNeighbors) const override;
// };

// struct BornMidFarSurviveRule : public BornArrayFunction
// {
//     BornMidFarSurviveRule(int numForBorn, int maxBorn, int surviveLower, int surviveUpper);
//     BornMidFarSurviveRule(juce::Array<int> numForBorn, juce::Array<int> bornMaxes, juce::Array<int> survieNums);
//     BornMidFarSurviveRule(juce::String numForBorn, juce::String bornMaxes, juce::String survieNums);

//     juce::Array<Hex::Point> getNewCells(const MappedHexState& parentCell, const MappedHexState* neighbors, int numNeighbors)  constoverride;
// };
}

#endif // LUMATONE_HEXAGON_AUTOMATA_RULES_H
