/*
  ==============================================================================

    hex_rings_launcher.h
    Created: 6 Aug 2023 9:48:57pm
    Author:  Vincenzo

  ==============================================================================
*/

#pragma once

#include "../game_component.h"

class HexRings;

class HexRingLauncher : public LumatoneSandboxGameComponent
{
public:
    HexRingLauncher(HexRings* gameIn);
    ~HexRingLauncher() override;

    void resized() override;

private:
    HexRings* game;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HexRingLauncher)
};
