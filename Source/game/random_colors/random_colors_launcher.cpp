/*
  ==============================================================================

    RandomColorsComponent.cpp
    Created: 30 Jul 2023 8:29:19pm
    Author:  Vincenzo

  ==============================================================================
*/

#include <JuceHeader.h>
#include "random_colors_launcher.h"
#include "random_colors.h"

//==============================================================================
RandomColorsComponent::RandomColorsComponent(LumatoneSandboxGameEngine* gameEngineIn)
    : gameEngine(gameEngineIn)
{
    game = new RandomColors(gameEngine->getController());
    auto gamePtr = dynamic_cast<LumatoneSandboxGameBase*>(game);
    gameEngine->setGame(gamePtr);

    toggleButton = std::make_unique<juce::TextButton>("Start", "Begin random colors");
    toggleButton->setClickingTogglesState(true);
    toggleButton->onClick = [&]
    {
        if (toggleButton->getToggleState())
        {
            gameEngine->startGame();
            toggleButton->setButtonText("Stop");
            toggleButton->setTooltip("Stop sending random colors");
        }
        else
        {
            gameEngine->endGame();
            toggleButton->setButtonText("Start");
            toggleButton->setTooltip("Begin random colors");
        }
    };
    addAndMakeVisible(*toggleButton);

    resetButton = std::make_unique<juce::TextButton>("Reset", "Reset to before random colors were sent");
    resetButton->onClick = [&]
    {
        gameEngine->resetGame();
        auto controller = gameEngine->getController();
        auto layout = game->getLastLayout();
        auto resetAction = new LumatoneEditAction::SectionEditAction(controller, 0, *layout.getBoard(0));
        for (int i = 1; i < controller->getNumBoards(); i++)
        {
            resetAction->createCoalescedAction(new LumatoneEditAction::SectionEditAction(controller, i, *layout.getBoard(i)));
        }

        controller->performUndoableAction(resetAction, true, "Game Reset: " + game->getName());
    };
    addAndMakeVisible(*resetButton);

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
    gameEngine->endGame();

    speedSlider = nullptr;

    resetButton = nullptr;
    toggleButton = nullptr;
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
    // This method is where you should set the bounds of any child
    // components that your component contains..

    const int margin = 24;
    const int buttonHeight = 30;
    const int buttonMargin = 12;

    const int buttonPadding = 8;

    auto buttonFont = toggleButton->getLookAndFeel().getTextButtonFont(*toggleButton, buttonHeight);
    toggleButton->setBounds(margin, margin, buttonFont.getStringWidth("Start") + buttonPadding, buttonHeight);
    
    resetButton->setBounds(toggleButton->getRight() + buttonMargin, margin, buttonFont.getStringWidth("Reset") + buttonPadding, buttonHeight);

    const int sliderWidth = proportionOfWidth(1.0f) - margin * 2;
    speedSlider->setBounds(margin, toggleButton->getBottom() + buttonMargin, sliderWidth, buttonHeight);
}
