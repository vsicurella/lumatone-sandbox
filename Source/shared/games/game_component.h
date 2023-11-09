#pragma once

#include <JuceHeader.h>

class LumatoneSandboxGameBase;

class LumatoneSandboxGameComponent : public juce::GroupComponent
{
public:

    LumatoneSandboxGameComponent(LumatoneSandboxGameBase* gameIn) { }
        // : expectedGameId(expectedGameIdIn) {}

    virtual ~LumatoneSandboxGameComponent() override { }

    // const LumatoneSandboxGameBase* getGameBase() const { return game->getGameLoaded(); }

    virtual void resized() override { controlsArea = juce::Rectangle<int>(xAreaMargin, yAreaMargin, getWidth() - 2*xAreaMargin, getHeight() - 2*yAreaMargin); }

protected:

    // virtual void registerGameWithEngine(LumatoneSandboxGameBase* gameBase)
    // {
    //     jassert(gameBase != nullptr);
        
    //     setText(gameBase->getName());
    //     game->setGame(gameBase);
    // }

protected:
    juce::String expectedGameId;

    const int xAreaMargin = 10;
    const int yAreaMargin = 15;

    juce::Rectangle<int> controlsArea;
};
