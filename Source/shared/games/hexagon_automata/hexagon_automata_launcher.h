/*
  ==============================================================================

    hexagon_automata_launcher.h
    Created: 27 Aug 2023 9:27:26pm
    Author:  Vincenzo

  ==============================================================================
*/

#pragma once

#include "../game_component.h"

#include "../../lumatone_editor_library/palettes/colour_picker_panel.h"

#include "./hexagon_automata_game_state.h"
namespace HexagonAutomata
{

class Game;

//==============================================================================
/*
*/
class Component : public LumatoneSandboxGameComponent
                , protected ColourSelectionListener
                , private HexagonAutomata::State
{

public:
    Component(HexagonAutomata::Game* gameIn);
    ~Component() override;

    void resized() override;

    void colourChangedCallback(ColourSelectionBroadcaster* source, juce::Colour newColour) override;

private:
    void onGameModeChange();
    void onGenerationModeChange();
    void onRulesChange();

    void updateSpeedSlider();

    void speedSliderCallback();

private:

    inline static const juce::String modeColumnId = juce::String("ModeColumn");
    inline static const juce::String rulesColumnId = juce::String("RulesColumn");
    inline static const juce::String tempoColumnId = juce::String("TempoColumn");
    inline static const juce::String controlsColumnId = juce::String("ControlsColumn");

    inline int getModeColumnWidth(juce::Font font) const;

private:
    void handleStatePropertyChange(juce::ValueTree stateIn, const juce::Identifier& property) override;

private:
    HexagonAutomata::Game* game;

    std::unique_ptr<juce::ComboBox> gameModeSelector;
    std::unique_ptr<juce::Label> gameModeLabel;

    std::unique_ptr<juce::ComboBox> generationModeSelector;
    std::unique_ptr<juce::Label> generationModeLabel;

    std::unique_ptr<juce::ComboBox> rulesModeSelector;
    std::unique_ptr<juce::Label> rulesModeLabel;

    std::unique_ptr<juce::Slider> genSpeedSlider;
    std::unique_ptr<juce::Label> genSpeedLabel;

    std::unique_ptr<juce::TextButton> addSeedButton;
    std::unique_ptr<juce::Slider> numSeedsSlider;
    std::unique_ptr<juce::Label> numSeedsLabel;

    std::unique_ptr<juce::TextEditor> bornRuleInput;
    std::unique_ptr<juce::Label> bornRuleLabel;

    std::unique_ptr<juce::TextEditor> surviveRuleInput;
    std::unique_ptr<juce::Label> surviveRuleLabel;

    std::unique_ptr<juce::Slider> distanceSlider;
    std::unique_ptr<juce::Label> distanceLabel;

    std::unique_ptr<CustomPickerPanel> aliveColourSelector;
    std::unique_ptr<juce::Label> aliveColourLabel;

    std::unique_ptr<CustomPickerPanel> deadColourSelector;
    std::unique_ptr<juce::Label> deadColourLabel;

    std::unique_ptr<juce::ToggleButton> midiClockToggle;
    std::unique_ptr<juce::ToggleButton> sustainNoPassThroughToggle;
    
    std::unique_ptr<juce::Slider> qnRatioSlider;
    std::unique_ptr<juce::Label> qnRatioLabel;

    std::unique_ptr<juce::TextButton> resetButton;

    float marginScalar = 0.1f;

    juce::Rectangle<int> flexArea;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Component)
};

}
