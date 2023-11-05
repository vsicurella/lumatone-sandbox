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
        
        int nextStepTicks = 15;
        KeyColorConstrainer* keyColourConstrainer = nullptr;
    };

    struct KeyState
    {
        int boardIndex = -1;
        int keyIndex = -1;
        juce::Colour colour;
    };

public:

    RandomColors(LumatoneController* controllerIn, RandomColors::Options options=RandomColors::Options());

    void reset(bool clearQueue) override;

    void nextTick() override;

    void setOptions(RandomColors::Options newOptions);

    RandomColors::Options getOptions() const;

private:

    LumatoneAction* renderFrame() const override;

private:

    void nextRandomKey();

private:

    juce::Random random;

    int ticks = 0;

    // Options
    int nextStepTicks = 60;

    KeyColorConstrainer* keyColorConstrainer;

    RandomColors::KeyState nextKeyState;
};
