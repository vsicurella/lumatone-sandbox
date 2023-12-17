#pragma once

#include "game_engine.h"

class LumatoneSandboxGameComponent;

class LumatoneSandboxGameEngineComponent : public juce::Component, public LumatoneSandboxGameEngine::Listener
{
public:
    LumatoneSandboxGameEngineComponent(LumatoneSandboxGameEngine* gameEngineIn);
    ~LumatoneSandboxGameEngineComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    LumatoneSandboxGameEngine* getGameEngine();

    // Set the game component to display, takes ownership
    void setGameComponent(LumatoneSandboxGameComponent* gameComponentIn);

    void endGame();

protected:

    void gameStatusChanged(LumatoneSandboxGameBase* game, LumatoneGameEngineState::GameStatus status) override;
    void gameEnded() override;

private:
    void showNoGameLoaded();
    void showStartButton();
    void showPauseButton();

private:

    LumatoneSandboxGameEngine* gameEngine;
    std::unique_ptr<LumatoneSandboxGameComponent> gameComponent;

    std::unique_ptr<juce::TextButton> startPauseButton;
    // std::unique_ptr<juce::TextButton> resetButton;
    std::unique_ptr<juce::TextButton> endButton;

    std::unique_ptr<juce::Slider> fpsSlider;
    std::unique_ptr<juce::Label> fpsLabel;

    std::unique_ptr<juce::TextButton> testButton;

    float engineControlsWidth = 0.2f;
    juce::Rectangle<int> controlsArea;
    juce::Rectangle<int> gameControlsArea;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LumatoneSandboxGameEngineComponent)
};
