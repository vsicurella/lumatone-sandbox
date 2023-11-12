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

#include "../../lumatone_editor_library/hex/hex_field.h"

namespace HexagonAutomata
{

struct MappedHexState;

// TODO: Redo "newCells" part of algorithm 
// Have rule defining what cells should be born or eligible to be born

struct NeighborFunction
{
    virtual ~NeighborFunction() { }
    virtual juce::Array<Hex::Point, juce::CriticalSection> getNeighborsVector(int distance=1) const;

    virtual float getLifeFactor(const MappedHexState& origin, const MappedHexState* neighbors, int numNeighbors) const = 0;
    virtual bool generateNewLife(const MappedHexState& origin, const MappedHexState* neighbors, int numNeighbors) const = 0;

    const juce::CriticalSection& getLock() const { return lock; }

protected:
    juce::CriticalSection lock;
};

struct BornArrayFunction : public NeighborFunction
{
    virtual ~BornArrayFunction() { }

    // Inefficent - either ignore or reimplement. It just uses getNewCells and returns the array size
    virtual bool generateNewLife(const MappedHexState& origin, const MappedHexState* neighbors, int numNeighbors);

    virtual juce::Array<Hex::Point> getNewCells(const MappedHexState& parentCell, const MappedHexState* neighbors, int numNeighbors) const = 0;
};

struct DefaultNeighborFunction : public NeighborFunction
{
    float getLifeFactor(const MappedHexState&, const MappedHexState*, int) const override;
    bool generateNewLife(const MappedHexState&, const MappedHexState*, int) const override;
};

struct BornSurviveRule : public NeighborFunction
{
    BornSurviveRule(int numBorn, int surviveLower, int surviveUpper);
    BornSurviveRule(juce::Array<int> bornNums, juce::Array<int> surviveNums);
    BornSurviveRule(juce::String bornString, juce::String surviveString);

    virtual ~BornSurviveRule() override { }

    virtual float getLifeFactor(const MappedHexState& origin, const MappedHexState* neighbors, int numNeighbors) const override;
    virtual bool generateNewLife(const MappedHexState& origin, const MappedHexState* neighbors, int numNeighbors) const override;

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
