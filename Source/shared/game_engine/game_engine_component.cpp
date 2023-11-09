#include "./game_engine_component.h"
#include "../games/game_component.h"

//==============================================================================
LumatoneSandboxGameEngineComponent::LumatoneSandboxGameEngineComponent(LumatoneSandboxGameEngine* gameEngineIn)
    : gameEngine(gameEngineIn)
{
    startPauseButton = std::make_unique<juce::TextButton>("Start", "Begin game");
    startPauseButton->setClickingTogglesState(true);
    startPauseButton->onClick = [&]
    {
        if (startPauseButton->getToggleState())
        {
            gameEngine->startGame();
            startPauseButton->setButtonText("Pause");
            startPauseButton->setTooltip("Pause game");
        }
        else
        {
            gameEngine->pauseGame();
            startPauseButton->setButtonText("Start");
            startPauseButton->setTooltip("Begin game");
        }
    };
    addAndMakeVisible(*startPauseButton);

    resetButton = std::make_unique<juce::TextButton>("Reset", "Reset to beginning of game");
    resetButton->onClick = [&]
    {
        startPauseButton->setToggleState(false, juce::NotificationType::dontSendNotification);
        startPauseButton->setButtonText("Start");
        startPauseButton->setTooltip("Begin game");

        gameEngine->resetGame();
    };
    addAndMakeVisible(*resetButton);

    endButton = std::make_unique<juce::TextButton>("End", "End game and reset to before game started.");
    endButton->onClick = [&]
    {
        startPauseButton->setToggleState(false, juce::NotificationType::dontSendNotification);
        startPauseButton->setButtonText("Start");
        startPauseButton->setTooltip("Begin game");

        gameEngine->endGame();
    };
    addAndMakeVisible(*endButton);

    fpsSlider = std::make_unique<juce::Slider>(juce::Slider::SliderStyle::IncDecButtons, juce::Slider::TextEntryBoxPosition::TextBoxLeft);
    fpsSlider->setRange(1, 60, 1);
    fpsSlider->setValue(gameEngine->getFps());
    fpsSlider->onValueChange = [&]
    {
        gameEngine->forceFps(fpsSlider->getValue());
    };
    addAndMakeVisible(*fpsSlider);

    fpsLabel = std::make_unique<juce::Label>("FPS Label", "FPS:");
    fpsLabel->setJustificationType(juce::Justification::centred);
    fpsLabel->attachToComponent(fpsSlider.get(), true);
    addAndMakeVisible(*fpsLabel);
}

LumatoneSandboxGameEngineComponent::~LumatoneSandboxGameEngineComponent()
{
    fpsSlider = nullptr;

    resetButton = nullptr;
    startPauseButton = nullptr;
}

void LumatoneSandboxGameEngineComponent::paint (juce::Graphics& g)
{
    auto backgroundColour = getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId);
    g.setColour(backgroundColour);
    g.fillRect(gameControlsArea);

    if (gameComponent == nullptr)
    {
        g.setColour(backgroundColour.contrasting(1.0f));
        g.drawFittedText("No game loaded", gameControlsArea, juce::Justification::centred, 1);
    }

    // g.setColour(juce::Colours::red);
    // g.drawRect(controlsArea);
    // g.setColour(juce::Colours::blue);
    // g.drawRect(gameControlsArea);
}

void LumatoneSandboxGameEngineComponent::resized()
{
    controlsArea = getLocalBounds().withRight(juce::roundToInt(proportionOfWidth(engineControlsWidth)));
    gameControlsArea = getLocalBounds().withLeft(controlsArea.getRight());

    int sectionMargin = 24;

    int controlHeight = 30;
    int controlMargin = 12;

    int labelMargin = controlMargin / 2;

    int buttonPadding = 8;

    float buttonMargin = controlMargin / 2;
    float buttonWidth = controlsArea.proportionOfWidth(0.5f) - controlMargin * 2;
    auto buttonFont = getLookAndFeel().getTextButtonFont(*startPauseButton, controlHeight);

    startPauseButton->setBounds(controlMargin, sectionMargin, buttonWidth, controlHeight);
    resetButton->setBounds(startPauseButton->getRight() + controlMargin, sectionMargin, buttonWidth, controlHeight);
    endButton->setBounds(controlMargin, startPauseButton->getBottom() + controlMargin, buttonWidth, controlHeight);

    auto labelFont = getLookAndFeel().getLabelFont(*fpsLabel);
    int labelWidth = labelFont.getStringWidth(fpsLabel->getText() + "__");
    fpsLabel->setBounds(controlMargin, endButton->getBottom() + controlMargin, labelWidth, controlHeight);

    int fpsSliderX = fpsLabel->getRight() + labelMargin;
    int fpsSliderWidth = controlsArea.getWidth() - fpsSliderX - controlMargin;
    fpsSlider->setBounds(fpsSliderX, endButton->getBottom() + controlMargin, fpsSliderWidth, controlHeight);

    if (gameComponent != nullptr)
    {
        gameComponent->setBounds(gameControlsArea);
    }
}

LumatoneSandboxGameEngine* LumatoneSandboxGameEngineComponent::getGameEngine()
{
    return gameEngine;
}

void LumatoneSandboxGameEngineComponent::setGameComponent(LumatoneSandboxGameComponent *gameComponentIn)
{
    if (gameComponent != nullptr)
    {
        startPauseButton->setToggleState(false, juce::NotificationType::sendNotification);
    }

    gameComponent.reset(gameComponentIn);
    addAndMakeVisible(*gameComponent);
}

void LumatoneSandboxGameEngineComponent::endGame()
{
    gameEngine->endGame();
    gameComponent = nullptr;
    resized();
}
