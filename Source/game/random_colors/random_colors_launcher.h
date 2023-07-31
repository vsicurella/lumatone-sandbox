/*
  ==============================================================================

    RandomColorsComponent.h
    Created: 30 Jul 2023 8:29:19pm
    Author:  Vincenzo

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "./random_colors.h"
#include "../game_engine.h"

//==============================================================================
/*
*/
class RandomColorsComponent  : public juce::Component
{
public:
    RandomColorsComponent(LumatoneSandboxGameEngine* gameEngine);
    ~RandomColorsComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:

    LumatoneSandboxGameEngine* gameEngine;
    RandomColors* game;

    std::unique_ptr<juce::TextButton> toggleButton;
    std::unique_ptr<juce::TextButton> resetButton;

    std::unique_ptr<juce::Slider> speedSlider;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RandomColorsComponent)
};
