#pragma once

#include <JuceHeader.h>
#include "./game_engine.h"

class LumatoneSandboxGameComponent : public juce::GroupComponent
{
public:

    LumatoneSandboxGameComponent(LumatoneSandboxGameEngine* engineIn)
        : gameEngine(engineIn) {}

    virtual ~LumatoneSandboxGameComponent() override { gameEngine = nullptr; }

    const LumatoneSandboxGameBase* getGameBase() const { return gameEngine->getGameRunning(); }

    virtual void resized() override { controlsArea = juce::Rectangle<int>(xAreaMargin, yAreaMargin, getWidth() - 2*xAreaMargin, getHeight() - 2*yAreaMargin); }

protected:

    virtual void registerGameWithEngine(LumatoneSandboxGameBase* gameBase)
    {
        jassert(gameBase != nullptr);
        
        setText(gameBase->getName());
        gameEngine->setGame(gameBase);
    }

protected:
    LumatoneSandboxGameEngine* gameEngine;

    const int xAreaMargin = 10;
    const int yAreaMargin = 15;

    juce::Rectangle<int> controlsArea;
};
