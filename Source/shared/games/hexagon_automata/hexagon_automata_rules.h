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

struct NeighborFunction 
{
    virtual juce::Array<Hex::Point> getNeighborsVector(int distance=1) const;

    virtual float getLifeFactor(const MappedHexState& origin, const MappedHexState* neighbors, int numNeighbors) const = 0;
    virtual bool generateNewLife(const MappedHexState& origin, const MappedHexState* neighbors, int numNeighbors) const = 0;
};

struct DefaultNeighborFunction : public NeighborFunction
{
    virtual float getLifeFactor(const MappedHexState&, const MappedHexState*, int) const override;
    virtual bool generateNewLife(const MappedHexState&, const MappedHexState*, int) const override;
};

struct BornSurviveRule : public NeighborFunction
{
    juce::Array<int> numsBorn;
    juce::Array<int> numsSurvive;
    
    BornSurviveRule(int numBorn, int surviveLower, int surviveUpper);
    BornSurviveRule(juce::Array<int> bornNums, juce::Array<int> surviveNums);
    BornSurviveRule(juce::String bornString, juce::String surviveString);

    virtual float getLifeFactor(const MappedHexState& origin, const MappedHexState* neighbors, int numNeighbors) const override;
    virtual bool generateNewLife(const MappedHexState& origin, const MappedHexState* neighbors, int numNeighbors) const override;
};

}


#endif // LUMATONE_HEXAGON_AUTOMATA_RULES_H
