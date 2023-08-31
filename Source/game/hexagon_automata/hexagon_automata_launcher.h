/*
  ==============================================================================

    hexagon_automata_launcher.h
    Created: 27 Aug 2023 9:27:26pm
    Author:  Vincenzo

  ==============================================================================
*/

#pragma once

#include "./hexagon_automata.h"
#include "../game_component.h"
#include "../../lumatone_editor_library/palettes/ColourSelectionPanels.h"

//==============================================================================
/*
*/
class HexagonAutomataComponent  : public LumatoneSandboxGameComponent
                                , protected ColourSelectionListener
{

    enum class Parameter
    {
        FramesPerGeneration, 
        AddSeed,
        NumAddSeeds,
        BornRule,
        SurviveRule,
        NeighborDistance,
        AliveColour,
        DeadColour
    };

public:
    HexagonAutomataComponent(LumatoneSandboxGameEngine* gameEngine);
    ~HexagonAutomataComponent() override;

    // void paint(juce::Graphics& g) override
    // {
    //     LumatoneSandboxGameComponent::paint(g);

    //     g.setColour(juce::Colours::red);
    //     g.drawRect(flexArea, 1.5f);
    // }

    void resized() override;

    void colourChangedCallback(ColourSelectionBroadcaster* source, juce::Colour newColour) override;

private:

    void onRulesChange();

private:
    
    HexagonAutomata::Game* game;

    std::unique_ptr<juce::Slider> genSpeedSlider;
    std::unique_ptr<juce::Label> genSpeedLabel;

    std::unique_ptr<juce::TextButton> addSeedButton;
    std::unique_ptr<juce::Slider> numSeedsSlider;
    std::unique_ptr<juce::Label> numSeedsLabel;

    std::unique_ptr<juce::TextEditor> bornRuleInput;
    std::unique_ptr<juce::Label> bornRuleLabel;

    std::unique_ptr<juce::TextEditor> suviveRuleInput;
    std::unique_ptr<juce::Label> surviveRuleLabel;

    std::unique_ptr<juce::Slider> distanceSlider;
    std::unique_ptr<juce::Label> distanceLabel;

    std::unique_ptr<CustomPickerPanel> aliveColourSelector;
    std::unique_ptr<juce::Label> aliveColourLabel;

    std::unique_ptr<CustomPickerPanel> deadColourSelector;
    std::unique_ptr<juce::Label> deadColourLabel;

    float marginScalar = 0.1f;

    juce::Rectangle<int> flexArea;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HexagonAutomataComponent)
};
