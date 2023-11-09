/*

    File: hexagon_automata_renderer.h
    Author: Vito
    Date: 2023/11/01

*/

#ifndef LUMATONE_HEXAGON_AUTOMATA_RENDERER_H
#define LUMATONE_HEXAGON_AUTOMATA_RENDERER_H

#include "hexagon_automata_cell_state.h"

namespace HexagonAutomata
{

class Renderer
{
public:

    Renderer(juce::Colour aliveColourIn=juce::Colours::white, juce::Colour deadColourIn=juce::Colours::grey);

    void setColour(juce::Colour aliveColourIn, juce::Colour deadColourIn=juce::Colours::transparentBlack);
    juce::Colour getAliveColour() const;
    juce::Colour getDeadColour() const;

    virtual void setMaxAge(int ticks);

    virtual juce::Colour renderAliveColour(const MappedHexState& state);

    virtual juce::Colour renderGradientColour(const MappedHexState& state);

    virtual juce::Colour renderCellColour(const MappedHexState& state);

    virtual MappedLumatoneKey renderCellKey(const MappedHexState& state);

    virtual MappedLumatoneKey renderSequencerKey(const MappedHexState& cell, const LumatoneLayout& noteLayout);

    virtual juce::Colour renderNewbornColour(const juce::Array<MappedHexState>& parents);

private:

    void updateGradients();

public:

    const float aliveScalar = 1.0f;
    const float deadScalar = 0.5f;
    const float emptyScalar = 0.1f;

private:
    juce::Colour aliveColour;
    juce::Colour deadColour;
    juce::Colour emptyColour = juce::Colours::black;

    juce::Colour oldColour;
    juce::Colour ageColour;
    int maxAge = 20;

    juce::ColourGradient healthGradient;
    juce::ColourGradient ageGradient;

};
}

#endif
