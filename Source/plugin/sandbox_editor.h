
#pragma once

#include "sandbox_processor.h"

#include "../shared/SandboxMenu.h"

class MainComponent;
class LumatoneSandboxDebugWindow;

//==============================================================================
class LumatoneSandboxProcessorEditor  : public juce::AudioProcessorEditor
                                      , public juce::ApplicationCommandTarget
{
public:
    explicit LumatoneSandboxProcessorEditor (LumatoneSandboxProcessor&);
    ~LumatoneSandboxProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    bool showMenu() const;

    //==============================================================================

    juce::ApplicationCommandTarget* getNextCommandTarget() override;

    void getAllCommands(juce::Array<juce::CommandID>& commands) override;

    void getCommandInfo(juce::CommandID commandID, juce::ApplicationCommandInfo &result) override;

    bool perform(const juce::ApplicationCommandTarget::InvocationInfo &info) override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    LumatoneSandboxProcessor& processor;

    std::unique_ptr<LumatoneSandboxDebugWindow> debugWindow;

    LumatoneController* controller;
    juce::UndoManager* undoManager;
    juce::ApplicationCommandManager* commandManager;

    LumatonePaletteLibrary* paletteLibrary;
    LumatoneSandboxGameEngine* gameEngine;

    std::unique_ptr<MainComponent> mainComponent;

    std::unique_ptr<LumatoneSandbox::Menu::Model> menuModel;
    std::unique_ptr<juce::MenuBarComponent> menuBar;

    std::unique_ptr<juce::ComponentBoundsConstrainer> constrainer;
    juce::OpenGLContext glContext;

    std::unique_ptr<juce::FileChooser> fileChooser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LumatoneSandboxProcessorEditor)
};
