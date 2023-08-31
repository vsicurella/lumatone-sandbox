#pragma once

#include "game_engine.h"

class LumatoneSandboxGameComponent;

class LumatoneSandboxGameEngineComponent : public juce::Component
{
public:
    LumatoneSandboxGameEngineComponent(LumatoneSandboxGameEngine* gameEngineIn);
    ~LumatoneSandboxGameEngineComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    // Set the game component to display, takes ownership
    void setGameComponent(LumatoneSandboxGameComponent* gameComponentIn);

private:

    LumatoneSandboxGameEngine* gameEngine;
    std::unique_ptr<LumatoneSandboxGameComponent> gameComponent;

    std::unique_ptr<juce::TextButton> startPauseButton;
    std::unique_ptr<juce::TextButton> resetButton;

    std::unique_ptr<juce::Slider> fpsSlider;
    std::unique_ptr<juce::Label> fpsLabel;

    float engineControlsWidth = 0.2f;
    juce::Rectangle<int> controlsArea;
    juce::Rectangle<int> gameControlsArea;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LumatoneSandboxGameEngineComponent)
};
