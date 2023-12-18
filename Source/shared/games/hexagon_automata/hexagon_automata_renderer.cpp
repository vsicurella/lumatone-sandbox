/*

    File: hexagon_automata_renderer.h
    Author: Vito
    Date: 2023/11/01

*/

#include "./hexagon_automata_renderer.h"

#include "../../lumatone_editor_library/color/adjust_layout_colour.h"
#include "hexagon_automata_renderer.h"

HexagonAutomata::Renderer::Renderer(juce::Colour aliveColourIn, juce::Colour deadColourIn)
{
    setColour(aliveColourIn, deadColourIn);
}

void HexagonAutomata::Renderer::setColour(juce::Colour aliveColourIn, juce::Colour deadColourIn)
{
    aliveColour = aliveColourIn;
    oldColour = aliveColour.contrasting(1.0f);

    if (deadColourIn != juce::Colours::transparentBlack)
        deadColour = deadColourIn;

    // ageGradient = juce::ColourGradient(aliveColour, 0.0f, 0.0f, oldColour, 1.0f, 1.0f, false);
    healthGradient = juce::ColourGradient(deadColour, 0.0f, 0.0f, aliveColour, 1.0f, 1.0f, false);
}

void HexagonAutomata::Renderer::setMaxAge(int ticks)
{
    maxAge = ticks;
}

juce::Colour HexagonAutomata::Renderer::getBinaryCellColour(const MappedHexState& state)
{
    if (static_cast<const HexagonAutomata::HexState&>(state).isEmpty())
        return emptyColour;
    if (state.isAlive())
        return aliveColour;
    return deadColour;
}

juce::Colour HexagonAutomata::Renderer::getGradientColour(const MappedHexState& state)
{
    // if (state.HexagonAutomata::HexState::isEmpty())
    //     return emptyColour;
    // if (state.isDead())
    //     return deadColour;
    // return healthGradient.getColourAtPosition(state.health);
    return juce::Colour::fromHSL(state.health, 1.0f, 0.62f, 1.0f);
}

void HexagonAutomata::Renderer::renderCellColour(MappedHexState& state, bool gradient)
{
    if (gradient)
        state.cellColor = getGradientColour(state);
    else
        state.cellColor = getBinaryCellColour(state);
}

MappedLumatoneKey HexagonAutomata::Renderer::renderCellKey(const MappedHexState& state)
{
    auto key = static_cast<const MappedLumatoneKey&>(state);
    // key.colour = getBinaryCellColour(state);
    key.colour = state.cellColor;
    return key;
}

MappedLumatoneKey HexagonAutomata::Renderer::renderSequencerKey(const MappedHexState& cell, const LumatoneLayout& noteLayout)
{
    auto key = static_cast<const MappedLumatoneKey&>(cell);

    auto noteKey = noteLayout.readKey(cell.boardIndex, cell.keyIndex);
    key.colour = noteKey->colour;

    if (cell.isAlive())
    {
        float range = aliveScalar - emptyScalar;
        float scalarOut = aliveScalar - range * (1.0f - cell.health);
        
        AdjustLayoutColour::multiplyBrightness(scalarOut, key);
    }
    // else if (cell.isDead())
    //     AdjustLayoutColour::multiplyBrightness(deadScalar, key);
    else
        AdjustLayoutColour::multiplyBrightness(emptyScalar, key);

    return key;
}

juce::Colour HexagonAutomata::Renderer::renderNewbornColour(const juce::Array<MappedHexState>& parents)
{
    if (parents.size() == 0)
        return juce::Colours::white;
        
    float hue = 0;
    float saturation = 0;
    float value = 0;

    for (auto cell : parents)
    {
        auto colour = cell.cellColor;

        hue += colour.getHue();
        saturation += colour.getSaturation();
        value += colour.getBrightness();
    }

    return juce::Colour(hue - (int)hue, saturation / parents.size(), value / parents.size(), (juce::uint8)0xff);
    // return parents[0].HexagonAutomata::HexState::colour;
}
void HexagonAutomata::Renderer::updateGradients()
{

}
