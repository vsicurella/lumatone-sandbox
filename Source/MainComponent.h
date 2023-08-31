#pragma once

#include <JuceHeader.h>

#include "lumatone_editor_library/LumatoneController.h"
#include "lumatone_editor_library/LumatoneKeyboardComponent.h"
#include "lumatone_editor_library/color/adjust_layout_colour.h"

#include "gui/connection_status.h"

#include "game/game_engine_component.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent  : public juce::Component,
                       public LumatoneEditor::StatusListener,
                       public juce::ApplicationCommandTarget
{
public:
    //==============================================================================
    MainComponent(LumatoneController* controllerIn);
    ~MainComponent() override;



    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    void addKeyboardComponentListener(LumatoneMidiState::Listener* listener) { lumatoneComponent->addListener(listener); }
    void removeKeyboardComponentListener(LumatoneMidiState::Listener* listener) { lumatoneComponent->removeListener(listener); }

    void setGameEngine(LumatoneSandboxGameEngine* engineIn);
    void setGameComponent(LumatoneSandboxGameComponent* gameIn);
    
    void gameLoadedCallback();

    bool getShowGameControl() const { return showGameControl; }
    void setShowGameControl(bool showControl);

protected:

    juce::ApplicationCommandTarget* getNextCommandTarget () override { return nullptr; }
    void getAllCommands(juce::Array <juce::CommandID>& commands) override;
	void getCommandInfo(juce::CommandID commandID, juce::ApplicationCommandInfo& result) override;
	bool perform(const juce::ApplicationCommandTarget::InvocationInfo& info) override;

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
