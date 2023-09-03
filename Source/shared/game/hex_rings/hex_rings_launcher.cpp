/*
  ==============================================================================

    hex_rings_launcher.cpp
    Created: 6 Aug 2023 9:48:57pm
    Author:  Vincenzo

  ==============================================================================
*/

#include "hex_rings_launcher.h"

//==============================================================================
HexRingLauncher::HexRingLauncher(LumatoneSandboxGameEngine* gameEngineIn)
    : LumatoneSandboxGameComponent(gameEngineIn)
{
    game = new HexRings(gameEngine->getController());
    registerGameWithEngine(game);
}

HexRingLauncher::~HexRingLauncher()
{
    game = nullptr;
    gameEngine->endGame();
}

void HexRingLauncher::resized()
{
    LumatoneSandboxGameComponent::resized();

    // This method is where you should set the bounds of any child
    // components that your component contains..

    // const int margin = 24;
    // const int buttonHeight = 30;
    // const int buttonMargin = 12;

    // const int buttonPadding = 8;

    // auto buttonFont = toggleButton->getLookAndFeel().getTextButtonFont(*toggleButton, buttonHeight);
    // toggleButton->setBounds(margin, margin, buttonFont.getStringWidth("Start") + buttonPadding, buttonHeight);

    //resetButton->setBounds(toggleButton->getRight() + buttonMargin, margin, buttonFont.getStringWidth("Reset") + buttonPadding, buttonHeight);

    //const int sliderWidth = proportionOfWidth(1.0f) - margin * 2;
    //speedSlider->setBounds(margin, toggleButton->getBottom() + buttonMargin, sliderWidth, buttonHeight);
}
