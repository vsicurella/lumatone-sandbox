/*
  ==============================================================================

    RandomColorsComponent.cpp
    Created: 30 Jul 2023 8:29:19pm
    Author:  Vincenzo

  ==============================================================================
*/

#include "random_colors_launcher.h"
#include "random_colors.h"

//==============================================================================
RandomColorsComponent::RandomColorsComponent(RandomColors* gameIn)
    : LumatoneSandboxGameComponent(gameIn)
{
    auto options = game->getOptions();
    speedSlider = std::make_unique<juce::Slider>(juce::Slider::SliderStyle::LinearHorizontal, juce::Slider::TextEntryBoxPosition::TextBoxLeft);
    speedSlider->setRange(2, 60, 1);
    speedSlider->setValue(options.nextStepTicks);
    speedSlider->onValueChange = [&]
    {
        auto options = RandomColors::Options();
        options.nextStepTicks = (int)speedSlider->getValue();
        game->setOptions(options);
    };
    addAndMakeVisible(*speedSlider);
}

RandomColorsComponent::~RandomColorsComponent()
{
    game = nullptr;
}

void RandomColorsComponent::paint (juce::Graphics& g)
{
    /* This demo code just fills the component's background and
       draws some placeholder text to get you started.

       You should replace everything in this method with your own
       drawing code..
    */

    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background
}

void RandomColorsComponent::resized()
{
    LumatoneSandboxGameComponent::resized();

    // This method is where you should set the bounds of any child
    // components that your component contains..

    const int margin = 24;
    const int buttonHeight = 30;
    const int buttonMargin = 12;

    const int buttonPadding = 8;

    const int sliderWidth = controlsArea.proportionOfWidth(1.0f) - margin * 2;
    speedSlider->setBounds(margin, buttonMargin, sliderWidth, buttonHeight);
}
