/*
  ==============================================================================

    This file contains the basic startup code for a JUCE application.

  ==============================================================================
*/

#include <JuceHeader.h>
#include "MainComponent.h"
#include "SandboxMenu.h"
#include "lumatone_editor_library/DeviceActivityMonitor.h"

#include "game/game_engine.h"
#include "game/random_colors/random_colors.h"

//==============================================================================
class LumatoneSandboxApp  : public juce::JUCEApplication
{
public:
    //==============================================================================
    LumatoneSandboxApp()
        : treeState(juce::ValueTree(LumatoneEditorProperty::StateTree)) {}

    const juce::String getApplicationName() override       { return ProjectInfo::projectName; }
    const juce::String getApplicationVersion() override    { return ProjectInfo::versionString; }
    bool moreThanOneInstanceAllowed() override             { return true; }

    //==============================================================================
    void initialise (const juce::String& commandLine) override
    {
        // This method is where you should put your application's initialisation code..

        undoManager = std::make_unique<juce::UndoManager>();

        midiDriver = std::make_unique<TerpstraMidiDriver>();
        controller = std::make_unique<LumatoneController>(treeState, *midiDriver, undoManager.get());

        monitor = std::make_unique<DeviceActivityMonitor>(midiDriver.get(), controller.get());

        mainWindow.reset (new MainWindow (getApplicationName(), controller.get()));

        commandManager = std::make_unique<juce::ApplicationCommandManager>();
        commandManager->registerAllCommandsForTarget(this);

        menuModel = std::make_unique<LumatoneSandbox::Menu::Model>(commandManager.get());

    #if JUCE_MAC
        LumatoneSandbox::Menu::Model::setMacMainMenu(menuModel.get());
    #endif

        mainWindow->setMenuBar(menuModel.get(), 32);
        // mainWindow->getMenuBarComponent()->getProperties().set(LumatoneEditorStyleIDs::popupMenuBackgroundColour, 
        // 	lookAndFeel.findColour(LumatoneEditorColourIDs::MenuBarBackground).toString());

        monitor->initializeDeviceDetection();

        gameEngine = std::make_unique<LumatoneSandboxGameEngine>(controller.get(), 30);
    }

    void shutdown() override
    {
        // Add your application's shutdown code here..

        gameEngine = nullptr;
        fileChooser = nullptr;
        
    #if JUCE_MAC
        LumatoneSandbox::Menu::Model::setMacMainMenu(nullptr);
    #endif
        mainWindow->setMenuBarComponent(nullptr);

        menuModel = nullptr;
        commandManager = nullptr;

        mainWindow = nullptr; // (deletes our window)

        monitor = nullptr;
        controller = nullptr;
        midiDriver = nullptr;

        undoManager = nullptr;
    }

    //==============================================================================
    void systemRequestedQuit() override
    {
        // This is called when the app is being asked to quit: you can ignore this
        // request and let the app carry on running, or call quit() to allow the app to close.
        quit();
    }

    void anotherInstanceStarted (const juce::String& commandLine) override
    {
        // When another instance of the app is launched while this one is running,
        // this method is invoked, and the commandLine parameter tells you what
        // the other instance's command-line arguments were.
    }

    //==============================================================================

    void getAllCommands(juce::Array <juce::CommandID>& commands) override
    {
        JUCEApplication::getAllCommands(commands);

        const juce::CommandID ids[] = {
            LumatoneSandbox::Menu::commandIDs::openSysExMapping,
            LumatoneSandbox::Menu::commandIDs::saveSysExMapping,
            LumatoneSandbox::Menu::commandIDs::saveSysExMappingAs,
            LumatoneSandbox::Menu::commandIDs::resetSysExMapping,

            LumatoneSandbox::Menu::commandIDs::deleteOctaveBoard,
            LumatoneSandbox::Menu::commandIDs::copyOctaveBoard,
            LumatoneSandbox::Menu::commandIDs::pasteOctaveBoard,
            LumatoneSandbox::Menu::commandIDs::pasteOctaveBoardChannels,
            LumatoneSandbox::Menu::commandIDs::pasteOctaveBoardNotes,
            LumatoneSandbox::Menu::commandIDs::pasteOctaveBoardColours,
            LumatoneSandbox::Menu::commandIDs::pasteOctaveBoardTypes,
            
            LumatoneSandbox::Menu::commandIDs::undo,
            LumatoneSandbox::Menu::commandIDs::redo,

            LumatoneSandbox::Menu::commandIDs::aboutSysEx
        };

        commands.addArray(ids, 14);
    }

	void getCommandInfo(juce::CommandID commandID, juce::ApplicationCommandInfo& result) override
    {
        result.setActive(false);

        switch (commandID)
            {
            case LumatoneSandbox::Menu::commandIDs::openSysExMapping:
                result.setInfo("Load file mapping", "Open a Lumatone key mapping", "File", 0);
                result.addDefaultKeypress('o', juce::ModifierKeys::commandModifier);
                break;

            case LumatoneSandbox::Menu::commandIDs::saveSysExMapping:
                result.setInfo("Save mapping", "Save the current mapping to file", "File", 0);
                result.addDefaultKeypress('s', juce::ModifierKeys::commandModifier);
                break;

            case LumatoneSandbox::Menu::commandIDs::saveSysExMappingAs:
                result.setInfo("Save mapping as...", "Save the current mapping to new file", "File", 0);
                result.addDefaultKeypress('a', juce::ModifierKeys::commandModifier);
                break;

            case LumatoneSandbox::Menu::commandIDs::resetSysExMapping:
                result.setInfo("New", "Start new mapping. Clear all edit fields, do not save current edits.", "File", 0);
                result.addDefaultKeypress('n', juce::ModifierKeys::commandModifier);
                break;

            case LumatoneSandbox::Menu::commandIDs::deleteOctaveBoard:
                result.setInfo("Delete", "Delete section data", "Edit", 0);
                result.addDefaultKeypress(juce::KeyPress::deleteKey, juce::ModifierKeys::noModifiers);
                break;

            case LumatoneSandbox::Menu::commandIDs::copyOctaveBoard:
                result.setInfo("Copy section", "Copy current octave board data", "Edit", 0);
                result.addDefaultKeypress('c', juce::ModifierKeys::commandModifier);
                break;

            case LumatoneSandbox::Menu::commandIDs::pasteOctaveBoard:
                result.setInfo("Paste section", "Paste copied section data", "Edit", 0);
                result.addDefaultKeypress('v', juce::ModifierKeys::commandModifier);
                break;

            case LumatoneSandbox::Menu::commandIDs::pasteOctaveBoardNotes:
                result.setInfo("Paste notes", "Paste copied section notes", "Edit", 0);
                result.addDefaultKeypress('v', juce::ModifierKeys::commandModifier | juce::ModifierKeys::shiftModifier);
                break;

            case LumatoneSandbox::Menu::commandIDs::pasteOctaveBoardChannels:
                result.setInfo("Paste channels", "Paste copied section channels", "Edit", 0);
                result.addDefaultKeypress('v', juce::ModifierKeys::commandModifier | juce::ModifierKeys::altModifier);
                break;
                    
            case LumatoneSandbox::Menu::commandIDs::pasteOctaveBoardColours:
                result.setInfo("Paste colours", "Paste copied section colours", "Edit", 0);
                result.addDefaultKeypress('v', juce::ModifierKeys::altModifier);
                break;
                    
            case LumatoneSandbox::Menu::commandIDs::pasteOctaveBoardTypes:
                result.setInfo("Paste types", "Paste copied section key types", "Edit", 0);
                result.addDefaultKeypress('v', juce::ModifierKeys::altModifier | juce::ModifierKeys::shiftModifier);
                break;
                    
            case LumatoneSandbox::Menu::commandIDs::undo:
                result.setInfo("Undo", "Undo latest edit", "Edit", 0);
                result.addDefaultKeypress('z', juce::ModifierKeys::commandModifier);
                // result.setActive(undoManager.canUndo());
                break;

            case LumatoneSandbox::Menu::commandIDs::redo:
                result.setInfo("Redo", "Redo latest edit", "Edit", 0);
                result.addDefaultKeypress('y', juce::ModifierKeys::commandModifier);
                result.addDefaultKeypress('z', juce::ModifierKeys::commandModifier + juce::ModifierKeys::shiftModifier);
                // result.setActive(undoManager.canRedo());
                break;

            case LumatoneSandbox::Menu::commandIDs::aboutSysEx:
                result.setInfo("About Lumatone Editor", "Shows version and copyright", "Help", 0);
                break;

            default:
                JUCEApplication::getCommandInfo(commandID, result);
                break;
            }
    }

	bool perform(const juce::ApplicationCommandTarget::InvocationInfo& info) override
    {
        switch (info.commandID)
        {
        case LumatoneSandbox::Menu::commandIDs::openSysExMapping:
        {
            fileChooser.reset(new juce::FileChooser("Open .LTN file", juce::File::getSpecialLocation(juce::File::SpecialLocationType::userDocumentsDirectory), "*.ltn"));
            fileChooser->launchAsync(
                juce::FileBrowserComponent::FileChooserFlags::canSelectFiles + juce::FileBrowserComponent::FileChooserFlags::openMode,
                [&](const juce::FileChooser& chooser)
                {
                    auto file = chooser.getResult();
                    controller->loadLayoutFromFile(file);
                    
                });
            return true;
        }

        default:
            return JUCEApplication::perform(info);
        }
    }


    //==============================================================================
    /*
        This class implements the desktop window that contains an instance of
        our MainComponent class.
    */
    class MainWindow    : public juce::DocumentWindow
    {
    public:
        MainWindow (juce::String name, LumatoneController* controller)
            : DocumentWindow (name,
                              juce::Desktop::getInstance().getDefaultLookAndFeel()
                                                          .findColour (juce::ResizableWindow::backgroundColourId),
                              DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar (false);
            setContentOwned (new MainComponent(controller), true);

            setResizable(true, true);

           //#if JUCE_IOS || JUCE_ANDROID
           // setFullScreen(true);
           //#else
            centreWithSize (getWidth(), getHeight());
           //#endif

            setVisible (true);
        }

        juce::BorderSize<int> getBorderThickness() override { return juce::BorderSize<int>(1); }

        void closeButtonPressed() override
        {
            // This is called when the user tries to close this window. Here, we'll just
            // ask the app to quit when this happens, but you can change this to do
            // whatever you need.
            JUCEApplication::getInstance()->systemRequestedQuit();
        }

        /* Note: Be careful if you override any DocumentWindow methods - the base
           class uses a lot of them, so by overriding you might break its functionality.
           It's best to do all your work in your content component instead, but if
           you really have to override any DocumentWindow methods, make sure your
           subclass also calls the superclass's method.
        */

    private:
        std::unique_ptr<juce::ComponentBoundsConstrainer> constrainer;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };

private:

    std::unique_ptr<juce::UndoManager> undoManager;
    std::unique_ptr<juce::ApplicationCommandManager> commandManager;
    std::unique_ptr<MainWindow> mainWindow;

    juce::ValueTree treeState;

    std::unique_ptr<LumatoneSandbox::Menu::Model> menuModel;

    std::unique_ptr<TerpstraMidiDriver> midiDriver;
    std::unique_ptr<LumatoneController> controller;
    std::unique_ptr<DeviceActivityMonitor> monitor;

    std::unique_ptr<juce::FileChooser> fileChooser;

    std::unique_ptr<LumatoneSandboxGameEngine> gameEngine;

};

//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION (LumatoneSandboxApp)
