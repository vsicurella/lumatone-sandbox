/*
  ==============================================================================

    random_colors.h
    Created: 30 Jul 2023 3:44:03pm
    Author:  Vincenzo

  ==============================================================================
*/

#pragma once
#include "../game_base.h"

class RandomColors : public LumatoneSandboxGameBase
{
public:

    struct Options
    {
        Options() {}
        
        int nextStepTicks = 60;
        KeyColorConstrainer* keyColourConstrainer = nullptr;
    };

public:

    RandomColors(LumatoneController* controllerIn, RandomColors::Options options=RandomColors::Options());
    ~RandomColors() {}

    void reset(bool clearQueue) override;

    void nextTick() override;

    void setOptions(RandomColors::Options newOptions);

private:

    juce::UndoableAction* renderFrame() override;

private:

    int ticks = 0;

    LumatoneLayout lastLayout;

    // Options
    int nextStepTicks = 60;

    KeyColorConstrainer* keyColorConstrainer;
};