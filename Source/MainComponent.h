#pragma once

#include <JuceHeader.h>
#include "lumatone_editor_library/LumatoneController.h"
#include "lumatone_editor_library/LumatoneKeyboardComponent.h"

#include "gui/connection_status.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent  : public juce::Component,
                       public LumatoneEditor::StatusListener
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

private:
    //==============================================================================
    // Your private member variables go here...

    LumatoneController* controller;

    std::unique_ptr<ConnectionStatus> connectionStatus;
    std::unique_ptr<LumatoneKeyboardComponent> lumatoneComponent;

    // UI helpers
    const float connectionStatusHeightRatio = 0.1f;
    const float lumatoneComponentWidthMarginRatio = 0.0667f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
