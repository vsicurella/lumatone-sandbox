/*
  ==============================================================================

    hex_rings_launcher.h
    Created: 6 Aug 2023 9:48:57pm
    Author:  Vincenzo

  ==============================================================================
*/

#pragma once
#include "./hex_rings.h"
#include "../game_engine.h"

class HexRingLauncher : public juce::Component
{
public:
    HexRingLauncher(LumatoneSandboxGameEngine* gameEngine);
    ~HexRingLauncher() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:

    LumatoneSandboxGameEngine* gameEngine;
    HexRings* game;

    std::unique_ptr<juce::TextButton> toggleButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HexRingLauncher)
};
