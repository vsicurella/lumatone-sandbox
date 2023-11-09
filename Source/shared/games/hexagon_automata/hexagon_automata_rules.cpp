/*
  ==============================================================================

    hexagon_automata_rules.cpp
    Created: 2023/11/01
    Author:  Vincenzo

  ==============================================================================
*/

#include "./hexagon_automata_rules.h"

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


juce::Array<Hex::Point> HexagonAutomata::NeighborFunction::getNeighborsVector(int distance) const 
{ 
    return Hex::Point().neighbors(distance); 
}

float HexagonAutomata::DefaultNeighborFunction::getLifeFactor(const HexagonAutomata::MappedHexState& origin, const HexagonAutomata::MappedHexState* neighbors, int numNeighbors) const
{
    if (numNeighbors < 2 || numNeighbors > 3)
        return 0.0f;
    return 1.0f;
}

bool HexagonAutomata::DefaultNeighborFunction::generateNewLife(const HexagonAutomata::MappedHexState& origin, const HexagonAutomata::MappedHexState* neighbors, int numNeighbors) const
{
    if (numNeighbors == 3)
        return true;
    return false;
}

HexagonAutomata::BornSurviveRule::BornSurviveRule(int numBorn, int surviveLower, int surviveUpper)
{
    numsBorn.add(numBorn);
    numsSurvive.add(surviveLower);
    numsSurvive.add(surviveUpper);
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
    if (numsSurvive.contains(numNeighbors))
        return 1.0f;
    return 0.0f;
}

bool HexagonAutomata::BornSurviveRule::generateNewLife(const HexagonAutomata::MappedHexState& origin, const HexagonAutomata::MappedHexState* neighbors, int numNeighbors) const
{
    if (numsBorn.contains(numNeighbors))
        return true;
    return false;
}

