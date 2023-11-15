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

    ageGradient = juce::ColourGradient(aliveColour, 0.0f, 0.0f,
        oldColour, 1.0f, 1.0f, false);
}

void HexagonAutomata::Renderer::setMaxAge(int ticks)
{
    maxAge = ticks;
}

juce::Colour HexagonAutomata::Renderer::getCellColour(const MappedHexState& state)
{
    if (static_cast<const HexagonAutomata::HexState&>(state).isEmpty())
        return emptyColour;
    if (state.isAlive())
        return aliveColour;
    return deadColour;
}

juce::Colour HexagonAutomata::Renderer::renderGradientColour(const MappedHexState& state)
{
    if (state.HexagonAutomata::HexState::isEmpty())
        return emptyColour;

    if (state.isDead())
        return deadColour;

    auto ageFactor = (double)state.age / (double)maxAge;
    auto colour = state.HexagonAutomata::HexState::colour;

    // if (ageFactor <= 1.0f)
    //     colour = ageGradient.getColourAtPosition(ageFactor);



    healthGradient = juce::ColourGradient(deadColour, 0.0f, 0.0f,
                                            colour, 1.0f, 1.0f, false);
    return healthGradient.getColourAtPosition(state.health);
}

void HexagonAutomata::Renderer::renderCellColour(MappedHexState& state)
{
    state.HexagonAutomata::HexState::colour = getCellColour(state);
}

MappedLumatoneKey HexagonAutomata::Renderer::renderCellKey(const MappedHexState& state)
{
    auto key = static_cast<const MappedLumatoneKey&>(state);
    key.colour = getCellColour(state);
    return key;
}

MappedLumatoneKey HexagonAutomata::Renderer::renderSequencerKey(const MappedHexState& cell, const LumatoneLayout& noteLayout)
{
    auto key = static_cast<const MappedLumatoneKey&>(cell);

    auto noteKey = noteLayout.readKey(cell.boardIndex, cell.keyIndex);
    key.colour = noteKey->colour;

    if (cell.isAlive())
        AdjustLayoutColour::multiplyBrightness(aliveScalar, key);
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
        auto colour = cell.HexagonAutomata::HexState::colour;

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
