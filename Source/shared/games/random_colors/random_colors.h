/*
  ==============================================================================

    random_colors.h
    Created: 30 Jul 2023 3:44:03pm
    Author:  Vincenzo

  ==============================================================================
*/

#pragma once
#include "../game_base.h"
#include "../game_base_state.h"

class RandomColors : public LumatoneSandboxGameBase
                    , protected LumatoneGameBaseState
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

    RandomColors(LumatoneGameEngineState& gameEngineState, RandomColors::Options options=RandomColors::Options());

    bool reset(bool clearQueue) override;

    bool nextTick() override;

    void setOptions(RandomColors::Options newOptions);

    RandomColors::Options getOptions() const;

private:
    LumatoneEditor::LayoutAction renderFrame() const override;

    void nextRandomKey();

public:
    LumatoneSandboxGameComponent* createController() override;

private:
    juce::ValueTree loadStateProperties(juce::ValueTree stateIn) override { }

private:

    juce::Random random;

    int ticks = 0;

    // Options
    int nextStepTicks = 60;

    KeyColorConstrainer* keyColorConstrainer;

    RandomColors::KeyState nextKeyState;
};
