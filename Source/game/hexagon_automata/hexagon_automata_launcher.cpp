/*
  ==============================================================================

    hexagon_automata_launcher.cpp
    Created: 27 Aug 2023 9:27:26pm
    Author:  Vincenzo

  ==============================================================================
*/

#include "hexagon_automata_launcher.h"

//==============================================================================
HexagonAutomataComponent::HexagonAutomataComponent(LumatoneSandboxGameEngine* gameEngineIn)
    : gameEngine(gameEngineIn)
{
    game = new HexagonAutomata::Game(gameEngine->getController());
    gameEngine->setGame((LumatoneSandboxGameBase*)game);

    startButton = std::make_unique<juce::TextButton>("Start", "Start advancing frames");
    startButton->setClickingTogglesState(true);
    startButton->onClick = [&]
    {
        if (startButton->getToggleState())
        {
            gameEngine->startGame();
            startButton->setButtonText("Pause");
        }
        else
        {
            gameEngine->pauseGame();
            startButton->setButtonText("Start");
        }
    };
    addAndMakeVisible(*startButton);

    resetButton = std::make_unique<juce::TextButton>("Reset", "Restart game");
    resetButton->onClick = [&]
    {
        gameEngine->endGame();
        gameEngine->resetGame();
    };
    addAndMakeVisible(*resetButton);

    addSeedButton = std::make_unique<juce::TextButton>("Add Seed", "Add a cluster of cells with 50% per cell");
    addSeedButton->onClick = [&]
    {
        game->addSeeds((int)numSeedsSlider->getValue());
    };
    addAndMakeVisible(*addSeedButton);

    numSeedsSlider = std::make_unique<juce::Slider>(juce::Slider::SliderStyle::IncDecButtons, juce::Slider::TextEntryBoxPosition::TextBoxLeft);
    numSeedsSlider->setRange(1, 280, 1);
    numSeedsSlider->setValue(1);
    addAndMakeVisible(*numSeedsSlider);

    speedSlider = std::make_unique<juce::Slider>(juce::Slider::SliderStyle::LinearHorizontal, juce::Slider::TextEntryBoxPosition::TextBoxLeft);
    speedSlider->setRange(5, 1200, 1);
    speedSlider->setSkewFactor(0.5, false);
    speedSlider->setValue(25, juce::NotificationType::dontSendNotification);
    speedSlider->onValueChange = [&]
    {
        game->setTicksPerGeneration(speedSlider->getValue());
    };
    addAndMakeVisible(*speedSlider);

    bornText = std::make_unique<juce::TextEditor>("BornRuleText");
    bornText->setInputFilter(new juce::TextEditor::LengthAndCharacterRestriction(0, "0123456789, "), true);
    bornText->setText("2");
    bornText->onTextChange = [&]
    {
        onRulesChange();
    };
    addAndMakeVisible(*bornText);
    
    surviveText = std::make_unique<juce::TextEditor>("SurviveRuleText");
    surviveText->setInputFilter(new juce::TextEditor::LengthAndCharacterRestriction(0, "0123456789, "), true);
    surviveText->setText("3,4");
    surviveText->onTextChange = [&]
    {
        onRulesChange();
    };
    addAndMakeVisible(*surviveText);

    distanceSlider = std::make_unique<juce::Slider>(juce::Slider::SliderStyle::IncDecButtons, juce::Slider::TextBoxLeft);
    distanceSlider->setRange(0, 7, 1);
    distanceSlider->setValue(1, juce::NotificationType::dontSendNotification);
    distanceSlider->onValueChange = [&]
    {
        game->setNeighborDistance(distanceSlider->getValue());
    };
    addAndMakeVisible(*distanceSlider);

    paletteWindow = std::make_unique<ColourPaletteWindow>(palettesDummy);
    paletteWindow->listenToColourSelection(this);
    addAndMakeVisible(*paletteWindow);
}

HexagonAutomataComponent::~HexagonAutomataComponent()
{ 
    game = nullptr;

    speedSlider = nullptr;
    resetButton = nullptr;
    addSeedButton = nullptr;
    resetButton = nullptr;
    startButton = nullptr;
}

void HexagonAutomataComponent::paint (juce::Graphics& g)
{

}

void HexagonAutomataComponent::resized()
{
    int margin = getHeight() * marginScalar;
    float controlHeightScalar = 0.12f;
    int controlHeight = juce::roundToInt(getHeight() * controlHeightScalar);

    auto buttonFont = getLookAndFeel().getTextButtonFont(*startButton, controlHeight);
    int startLength = buttonFont.getStringWidth(startButton->getButtonText() + "__");
    startButton->setBounds(margin, margin, startLength, controlHeight);

    int resetLength = buttonFont.getStringWidth(resetButton->getButtonText() + "__");
    resetButton->setBounds(startButton->getRight() + margin, margin, resetLength, controlHeight);

    int speedSliderX = resetButton->getRight() + margin;
    speedSlider->setBounds(speedSliderX, 0, getWidth() - speedSliderX - margin, controlHeight);

    int addLength = buttonFont.getStringWidth(addSeedButton->getButtonText() + "__");
    addSeedButton->setBounds(margin, controlHeight + margin * 2, addLength, controlHeight);

    numSeedsSlider->setBounds(addSeedButton->getRight() + margin, addSeedButton->getY(), addLength * 2, controlHeight);

    auto textLength = buttonFont.getStringWidth("1, 2, 3, 4");
    bornText->setBounds(margin, numSeedsSlider->getBottom() + margin, textLength, controlHeight);
    surviveText->setBounds(bornText->getRight() + margin, numSeedsSlider->getBottom() + margin, textLength, controlHeight);

    distanceSlider->setBounds(margin, bornText->getBottom() + margin, getWidth() * 0.33, controlHeight);

    paletteWindow->setBounds(numSeedsSlider->getRight() + margin, addSeedButton->getY() + margin, getWidth() - numSeedsSlider->getRight() - margin, getHeight() - addSeedButton->getY() - margin);
}

void HexagonAutomataComponent::colourChangedCallback(ColourSelectionBroadcaster* source, juce::Colour newColour)
{
    game->setAliveColour(newColour);
}

void HexagonAutomataComponent::onRulesChange()
{
    game->setBornSurviveRules(bornText->getText(), surviveText->getText());
}