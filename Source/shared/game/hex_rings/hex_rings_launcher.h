/*
  ==============================================================================

    hex_rings_launcher.h
    Created: 6 Aug 2023 9:48:57pm
    Author:  Vincenzo

  ==============================================================================
*/

#pragma once
#include "./hex_rings.h"
#include "../game_component.h"

class HexRingLauncher : public LumatoneSandboxGameComponent
{
public:
    HexRingLauncher(LumatoneSandboxGameEngine* gameEngine);
    ~HexRingLauncher() override;

    void resized() override;

private:

    HexRings* game;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HexRingLauncher)
};
