#pragma once

#include <JuceHeader.h>

#include "SandboxState.h"

#include "./lumatone_editor_library/color/adjust_layout_colour.h"
#include "./lumatone_editor_library/listeners/status_listener.h"

#include "./game_engine/game_engine.h"

class LumatoneController;
class LumatoneKeyboardComponent;
class ConnectionStatus;
class LumatoneSandboxGameEngineComponent;
class LumatoneSandboxGameBase;

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent  : public juce::Component
                     , public LumatoneSandboxState
                     , public LumatoneEditor::StatusListener
                     , public LumatoneSandboxGameEngine::Listener
                     , public juce::ApplicationCommandTarget
{
public:
    //==============================================================================
    MainComponent(const LumatoneSandboxState& stateIn);
    ~MainComponent() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    LumatoneKeyboardComponent* getLumatoneKeyboardComponent() { return lumatoneComponent.get(); }

    void setGameEngine(LumatoneSandboxGameEngine* engineIn);
    // void setGameComponent(LumatoneSandboxGameComponent* gameIn);
    
    bool getShowGameControl() const { return showGameControl; }
    void setShowGameControl(bool showControl);

protected:
    juce::ApplicationCommandTarget* getNextCommandTarget () override { return nullptr; }
    void getAllCommands(juce::Array <juce::CommandID>& commands) override;
	void getCommandInfo(juce::CommandID commandID, juce::ApplicationCommandInfo& result) override;
	bool perform(const juce::ApplicationCommandTarget::InvocationInfo& info) override;

private:
    void gameStatusChanged(LumatoneSandboxGameBase* game, LumatoneGameEngineState::GameStatus status) override;

    void gameLoadedCallback(LumatoneSandboxGameBase* game);

private:
    //==============================================================================
    // Your private member variables go here...

    LumatoneController* controller;

    std::unique_ptr<ConnectionStatus> connectionStatus;
    std::unique_ptr<LumatoneKeyboardComponent> lumatoneComponent;

    AdjustLayoutColour colourAdjust;

    float gameControlHeightRatio = 0.2f;
    std::unique_ptr<LumatoneSandboxGameEngineComponent> gameEngineComponent;
    bool showGameControl = true;

    // UI helpers
    const float connectionStatusHeightRatio = 0.1f;
    const float lumatoneComponentWidthMarginRatio = 0.0667f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
