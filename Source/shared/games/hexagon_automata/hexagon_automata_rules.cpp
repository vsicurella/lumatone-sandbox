/*
  ==============================================================================

    hexagon_automata_rules.cpp
    Created: 2023/11/01
    Author:  Vincenzo

  ==============================================================================
*/

#include "hexagon_automata_rules.h"

#include "./hexagon_automata_cell_state.h"

static juce::Array<int> ParseListArgument(juce::String numberList)
{
    juce::Array<int> list;
    juce::String dbgStr;

    auto tks = juce::StringArray::fromTokens(numberList, juce::String(","), juce::String());
    for (auto tk : tks)
    { 
        if (tk.trim().isNotEmpty())
        {
            auto num = tk.getIntValue();
            if (num > 0 && num < 280)
            {
                list.addIfNotAlreadyThere(num);
                dbgStr += juce::String(num) + ",";
            }
        }
    }

    DBG("BornSurviveRule::ParseListArgument: " + dbgStr);

    return list;
}


juce::Array<Hex::Point, juce::CriticalSection> HexagonAutomata::NeighborFunction::getNeighborsVector(int distance) const 
{ 
    juce::ScopedLock l(lock);

    auto neighbors = Hex::Point().neighbors(distance);
    juce::Array<Hex::Point, juce::CriticalSection> neighborsOut;
    neighborsOut.addArray(neighbors);
    return neighborsOut;
}

float HexagonAutomata::DefaultNeighborFunction::getLifeFactor(const HexagonAutomata::MappedHexState& origin, const HexagonAutomata::MappedHexState* neighbors, int numNeighbors) const
{
    juce::ScopedLock l(lock);

    if (numNeighbors < 2 || numNeighbors > 3)
        return 0.0f;
    return 1.0f;
}

bool HexagonAutomata::DefaultNeighborFunction::generateNewLife(const HexagonAutomata::MappedHexState& origin, const HexagonAutomata::MappedHexState* neighbors, int numNeighbors) const
{
    juce::ScopedLock l(lock);

    if (numNeighbors == 3)
        return true;
    return false;
}

HexagonAutomata::BornSurviveRule::BornSurviveRule(int numBorn, int surviveLower, int surviveUpper)
{
    numsBorn.add(numBorn);

    for (int i = surviveLower; i <= surviveUpper; i++)
    {
        numsSurvive.add(i);    
    }
}

HexagonAutomata::BornSurviveRule::BornSurviveRule(juce::Array<int> bornNums, juce::Array<int> surviveNums)
    : numsBorn(bornNums)
    , numsSurvive(surviveNums) 
{
}

HexagonAutomata::BornSurviveRule::BornSurviveRule(juce::String bornString, juce::String surviveString)
{
    numsBorn = ParseListArgument(bornString);
    numsSurvive = ParseListArgument(surviveString);
}

float HexagonAutomata::BornSurviveRule::getLifeFactor(const HexagonAutomata::MappedHexState& origin, const HexagonAutomata::MappedHexState* neighbors, int numNeighbors) const
{
    juce::ScopedLock l(lock);

    if (numsSurvive.contains(numNeighbors))
        return 1.0f;
    return 0.0f;
}

bool HexagonAutomata::BornSurviveRule::generateNewLife(const HexagonAutomata::MappedHexState& origin, const HexagonAutomata::MappedHexState* neighbors, int numNeighbors) const
{
    juce::ScopedLock l(lock);

    if (numsBorn.contains(numNeighbors))
        return true;
    return false;
}

// HexagonAutomata::BornMiddleSurviveRule::BornMiddleSurviveRule(int numForBorn, int maxBorn, int surviveLower, int surviveUpper)
// {
//     numsForBorn.add(numForBorn);
//     maxNumsBorn.add(maxBorn);
//     for (int i = surviveLower; i <= surviveUpper; i++)
//         numsSurvive.add(i);
// }

// HexagonAutomata::BornMiddleSurviveRule::BornMiddleSurviveRule(juce::Array<int> numForBornIn, juce::Array<int> bornMaxes, juce::Array<int> surviveNums)
//     : numsForBorn(numsForBornIn)
//     , maxNumsForBorn(bornMaxes)
//     , numsSurvive(surviveNums)
// {
// }

// HexagonAutomata::BornMiddleSurviveRule::BornMiddleSurviveRule(juce::String numForBorn, juce::String bornMaxes, juce::String survieNums)
// {
//     numsForBorn = ParseListArgument(numForBorn);
//     maxNumsBorn = ParseListArgument(bornMaxes);
//     numsSurvive = ParseListArgument(survieNums);
// }

// juce::Array<Hex::Point> HexagonAutomata::BornMidNearSurviveRule::getNewCells(const MappedHexState &parentCell, const MappedHexState *neighbors, int numNeighbors)
// {
//     juce::Array<Hex::Point> newCells;

//     juce::Array<Hex::Point> aliveNeighbors;
//     for (int i = 0; i < numNeighbors; i++)
//     {

//     }

//     return newCells;
// }

// juce::Array<Hex::Point> HexagonAutomata::BornMidFarSurviveRule::getNewCells(const MappedHexState &parentCell, const MappedHexState *neighbors, int numNeighbors)
// {
//     return juce::Array<Hex::Point>();
// }
