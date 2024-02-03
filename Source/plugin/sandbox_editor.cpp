#include "sandbox_processor.h"
#include "sandbox_editor.h"

#include "../shared/gui/adjust_colour_panel.h"

#include "../shared/games/random_colors/random_colors_launcher.h"
#include "../shared/games/hex_rings/hex_rings_launcher.h"
#include "../shared/games/hexagon_automata/hexagon_automata_launcher.h"

#include "../shared/MainComponent.h"

#include "../shared/debug/LumatoneSandboxDebugWindow.h"

//==============================================================================
LumatoneSandboxProcessorEditor::LumatoneSandboxProcessorEditor (LumatoneSandboxProcessor& p, const LumatoneSandboxState& stateIn)
    : AudioProcessorEditor (&p), processor (p)
    , LumatoneSandboxState("LumatoneSandboxEditor", stateIn)
    , LumatoneSandboxState::Controller(static_cast<LumatoneSandboxState&>(*this))
{
    juce::ignoreUnused (processor);

    debugWindow = std::make_unique<LumatoneSandboxDebugWindow>(processor.getLogData());
    debugWindow->setSize(800, 500);
    debugWindow->addToDesktop();
    debugWindow->setVisible(true);

    mainComponent = std::make_unique<MainComponent>(*this);
    addAndMakeVisible(*mainComponent);

    commandManager = std::make_unique<juce::ApplicationCommandManager>();
    commandManager->registerAllCommandsForTarget(this);
    commandManager->registerAllCommandsForTarget(mainComponent.get());
    commandManager->setFirstCommandTarget(this);

    menuModel = std::make_unique<LumatoneSandbox::Menu::Model>(commandManager.get());

    // if (showMenu())
    // {
        menuBar = std::make_unique<juce::MenuBarComponent>(menuModel.get());
        addAndMakeVisible(menuBar.get());
    // }
    // else if (JucePlugin_Build_Standalone)
    // {
    // #if JUCE_MAC
    //     LumatoneSandbox::Menu::Model::setMacMainMenu((juce::MenuBarModel*)(nullptr));
    // #endif
    // }

    setSize (1024, 768);

    setResizable(true, true);
}

LumatoneSandboxProcessorEditor::~LumatoneSandboxProcessorEditor()
{
    fileChooser = nullptr;
    constrainer = nullptr;

    menuBar = nullptr;
    menuModel = nullptr;
    mainComponent = nullptr;

    commandManager = nullptr;
    debugWindow = nullptr;
}

//==============================================================================
void LumatoneSandboxProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void LumatoneSandboxProcessorEditor::resized()
{
    int menuHeight = 24;
    if (showMenu())
        menuBar->setBounds(0, 0, getWidth(), menuHeight);
    else
        menuHeight = 0;

    mainComponent->setBounds(getLocalBounds().withTrimmedTop(menuHeight));
}

bool LumatoneSandboxProcessorEditor::showMenu() const
{
    return true;
    // return JucePlugin_Build_Standalone 
    //     && ((juce::SystemStats::getOperatingSystemType() & juce::SystemStats::OperatingSystemType::MacOSX) 
    //         != juce::SystemStats::OperatingSystemType::MacOSX
    //         );
}

juce::ApplicationCommandTarget* LumatoneSandboxProcessorEditor::getNextCommandTarget()
{ 
    return mainComponent.get();
}

void LumatoneSandboxProcessorEditor::getAllCommands(juce::Array <juce::CommandID>& commands)
{
    commands.add(LumatoneSandbox::Menu::commandIDs::openSysExMapping);
    commands.add(LumatoneSandbox::Menu::commandIDs::saveSysExMapping);
    commands.add(LumatoneSandbox::Menu::commandIDs::saveSysExMappingAs);
    commands.add(LumatoneSandbox::Menu::commandIDs::resetSysExMapping);
    commands.add(LumatoneSandbox::Menu::commandIDs::importSysExMapping);

    commands.add(LumatoneSandbox::Menu::commandIDs::undo);
    commands.add(LumatoneSandbox::Menu::commandIDs::redo);
    commands.add(LumatoneSandbox::Menu::commandIDs::aboutSysEx);

    commands.add(LumatoneSandbox::Menu::commandIDs::openRandomColorsGame);
    commands.add(LumatoneSandbox::Menu::commandIDs::openHexRingsGame);
    commands.add(LumatoneSandbox::Menu::commandIDs::openHexagonAutomata);


    // commands.add(LumatoneSandbox::Menu::commandIDs::deleteOctaveBoard);
    // commands.add(LumatoneSandbox::Menu::commandIDs::copyOctaveBoard);
    // commands.add(LumatoneSandbox::Menu::commandIDs::pasteOctaveBoard);
    // commands.add(LumatoneSandbox::Menu::commandIDs::pasteOctaveBoardChannels);
    // commands.add(LumatoneSandbox::Menu::commandIDs::pasteOctaveBoardNotes);
    // commands.add(LumatoneSandbox::Menu::commandIDs::pasteOctaveBoardColours);
    // commands.add(LumatoneSandbox::Menu::commandIDs::pasteOctaveBoardTypes);

    commands.add(LumatoneSandbox::Menu::commandIDs::adjustColour);

    commands.add(juce::StandardApplicationCommandIDs::quit);
}

void LumatoneSandboxProcessorEditor::getCommandInfo(juce::CommandID commandID, juce::ApplicationCommandInfo& result)
{
    result.setActive(true);

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
             
        case LumatoneSandbox::Menu::commandIDs::importSysExMapping:
            result.setInfo("Import", "Get mapping from connected Lumatone", "File", 0);
            result.addDefaultKeypress('i', juce::ModifierKeys::currentModifiers);
            break;
        case LumatoneSandbox::Menu::commandIDs::undo:
            result.setInfo("Undo", "Undo latest edit", "Edit", 0);
            result.addDefaultKeypress('z', juce::ModifierKeys::commandModifier);
            result.setActive(undoManager->canUndo());
            break;

        case LumatoneSandbox::Menu::commandIDs::redo:
            result.setInfo("Redo", "Redo latest edit", "Edit", 0);
            result.addDefaultKeypress('y', juce::ModifierKeys::commandModifier);
            result.addDefaultKeypress('z', juce::ModifierKeys::commandModifier + juce::ModifierKeys::shiftModifier);
            result.setActive(undoManager->canRedo());
            break;

        case LumatoneSandbox::Menu::commandIDs::openRandomColorsGame:
            result.setInfo("Random Colors", "Open launcher for Random Colors game", "Game", 0);
            result.setActive(true);
            break;

        case LumatoneSandbox::Menu::commandIDs::openHexRingsGame:
            result.setInfo("Hex Rings", "Open launcher for Hex Rings game", "Game", 0);
            break;

        case LumatoneSandbox::Menu::commandIDs::openHexagonAutomata:
            result.setInfo("Hexagon Automata", "Open launcher for hex game of life", "Game", 0);
            break;

        case LumatoneSandbox::Menu::commandIDs::aboutSysEx:
            result.setInfo("About Lumatone Editor", "Shows version and copyright", "Help", 0);
            break;

        case juce::StandardApplicationCommandIDs::quit:
            result.setInfo("Quit", "Close window and terminate application", "File", 0);
            break;
        // case LumatoneSandbox::Menu::commandIDs::deleteOctaveBoard:
        //     result.setInfo("Delete", "Delete section data", "Edit", 0);
        //     result.addDefaultKeypress(juce::KeyPress::deleteKey, juce::ModifierKeys::noModifiers);
        //     break;

        // case LumatoneSandbox::Menu::commandIDs::copyOctaveBoard:
        //     result.setInfo("Copy section", "Copy current octave board data", "Edit", 0);
        //     result.addDefaultKeypress('c', juce::ModifierKeys::commandModifier);
        //     break;

        // case LumatoneSandbox::Menu::commandIDs::pasteOctaveBoard:
        //     result.setInfo("Paste section", "Paste copied section data", "Edit", 0);
        //     result.addDefaultKeypress('v', juce::ModifierKeys::commandModifier);
        //     break;

        // case LumatoneSandbox::Menu::commandIDs::pasteOctaveBoardNotes:
        //     result.setInfo("Paste notes", "Paste copied section notes", "Edit", 0);
        //     result.addDefaultKeypress('v', juce::ModifierKeys::commandModifier | juce::ModifierKeys::shiftModifier);
        //     break;

        // case LumatoneSandbox::Menu::commandIDs::pasteOctaveBoardChannels:
        //     result.setInfo("Paste channels", "Paste copied section channels", "Edit", 0);
        //     result.addDefaultKeypress('v', juce::ModifierKeys::commandModifier | juce::ModifierKeys::altModifier);
        //     break;
                
        // case LumatoneSandbox::Menu::commandIDs::pasteOctaveBoardColours:
        //     result.setInfo("Paste colours", "Paste copied section colours", "Edit", 0);
        //     result.addDefaultKeypress('v', juce::ModifierKeys::altModifier);
        //     break;
                
        // case LumatoneSandbox::Menu::commandIDs::pasteOctaveBoardTypes:
        //     result.setInfo("Paste types", "Paste copied section key types", "Edit", 0);
        //     result.addDefaultKeypress('v', juce::ModifierKeys::altModifier | juce::ModifierKeys::shiftModifier);
        //     break;

        case LumatoneSandbox::Menu::commandIDs::adjustColour:
            result.setInfo("Adjust colours", "Apply adjustments to colours across the layout", "Edit", 0);
            break;

        default:
            result.setInfo("?", "Unknown command", "Unknown", 0);  
            break;
        }
}

bool LumatoneSandboxProcessorEditor::perform(const juce::ApplicationCommandTarget::InvocationInfo& info)
{
    switch (info.commandID)
    {
    case LumatoneSandbox::Menu::commandIDs::openSysExMapping:
    {
        fileChooser.reset(new juce::FileChooser("Open .LTN file", getLastOpenedMappingsDirectory(), "*.ltn"));
        fileChooser->launchAsync(
            juce::FileBrowserComponent::FileChooserFlags::canSelectFiles | juce::FileBrowserComponent::FileChooserFlags::openMode,
            [&](const juce::FileChooser& chooser)
            {
                auto file = chooser.getResult();
                setCurrentFile(file);
                resetToCurrentFile();
                
            });
        return true;
    }

    case LumatoneSandbox::Menu::commandIDs::saveSysExMappingAs:
    {
        fileChooser.reset(new juce::FileChooser("Save .LTN file", getLastOpenedMappingsDirectory(), "*.ltn"));
        fileChooser->launchAsync(
            juce::FileBrowserComponent::FileChooserFlags::canSelectFiles | juce::FileBrowserComponent::FileChooserFlags::saveMode,
            [&](const juce::FileChooser& chooser)
            {
                auto file = chooser.getResult();
                saveMappingToFile(file);
            });
        return true;
    }

    case LumatoneSandbox::Menu::commandIDs::importSysExMapping:
    {
        requestMappingFromDevice();
        return true;
    }

    case LumatoneSandbox::Menu::commandIDs::adjustColour:
    {
        juce::DialogWindow::LaunchOptions launch;
        launch.dialogTitle = "Adjust Colours";
        launch.content.setOwned(new AdjustColourPanel(*this));
        launch.content->setSize(600, 400);

        launch.componentToCentreAround = mainComponent.get();
        launch.launchAsync();

        return true;
    }

    case LumatoneSandbox::Menu::commandIDs::undo:
        undoManager->undo();
        return true;

    case LumatoneSandbox::Menu::commandIDs::redo:
        undoManager->redo();
        return true;

    case LumatoneSandbox::Menu::commandIDs::openRandomColorsGame:
    {
        auto gameId = LumatoneSandbox::GameNameToString(LumatoneSandbox::GameName::RandomColors);
        getGameEngine()->loadGame(gameId);
        return true;
    }
    
    case LumatoneSandbox::Menu::commandIDs::openHexRingsGame:
    {
        auto gameId = LumatoneSandbox::GameNameToString(LumatoneSandbox::GameName::HexRings);
        getGameEngine()->loadGame(gameId);
        return true;
    }

    case LumatoneSandbox::Menu::commandIDs::openHexagonAutomata:
    {
        auto gameId = LumatoneSandbox::GameNameToString(LumatoneSandbox::GameName::HexagonAutomata);
        getGameEngine()->loadGame(gameId);
        return true;
    }

    case juce::StandardApplicationCommandIDs::quit:
        juce::JUCEApplication::quit();
        return true;


    default:
        return false;
    }

    return false;
}
