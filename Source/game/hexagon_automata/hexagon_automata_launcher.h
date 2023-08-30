/*
  ==============================================================================

    hexagon_automata_launcher.h
    Created: 27 Aug 2023 9:27:26pm
    Author:  Vincenzo

  ==============================================================================
*/

#pragma once

#include "./hexagon_automata.h"
#include "../game_engine.h"
#include "../../lumatone_editor_library/palettes/ColourPaletteWindow.h"

//==============================================================================
/*
*/
class HexagonAutomataComponent  : public juce::Component,
                                  protected ColourSelectionListener
{
public:
    HexagonAutomataComponent(LumatoneSandboxGameEngine* gameEngine);
    ~HexagonAutomataComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void colourChangedCallback(ColourSelectionBroadcaster* source, juce::Colour newColour) override;

private:

    void onRulesChange();

private:

    LumatoneSandboxGameEngine* gameEngine;
    HexagonAutomata::Game* game;

    std::unique_ptr<juce::TextButton> startButton;
    std::unique_ptr<juce::TextButton> resetButton;

    std::unique_ptr<juce::TextButton> addSeedButton;

    std::unique_ptr<juce::Slider> speedSlider;

    std::unique_ptr<juce::TextEditor> bornText;
    std::unique_ptr<juce::TextEditor> surviveText;

    std::unique_ptr<juce::Slider> distanceSlider;

    std::unique_ptr<juce::Slider> numSeedsSlider;

    juce::Array<LumatoneEditorColourPalette> palettesDummy;
    std::unique_ptr<ColourPaletteWindow> paletteWindow;

    float marginScalar = 0.08f;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HexagonAutomataComponent)
};
