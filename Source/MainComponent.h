#pragma once

#include <JuceHeader.h>
#include "lumatone_editor_library/LumatoneController.h"

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

private:
    //==============================================================================
    // Your private member variables go here...

    LumatoneController* controller;

    std::unique_ptr<ConnectionStatus> connectionStatus;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
