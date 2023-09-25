#include "sandbox_processor.h"
#include "sandbox_editor.h"

#include "../shared/gui/adjust_colour_panel.h"

#include "../shared/game/random_colors/random_colors_launcher.h"
#include "../shared/game/hex_rings/hex_rings_launcher.h"
#include "../shared/game/hexagon_automata/hexagon_automata_launcher.h"

//==============================================================================
LumatoneSandboxProcessorEditor::LumatoneSandboxProcessorEditor (LumatoneSandboxProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
    , controller(p.getLumatoneController())
    , undoManager(p.getUndoManager())
    , commandManager(p.getCommandManager())
    , paletteLibrary(p.getPaletteLibrary())
    , gameEngine(p.getGameEngine())
{
    juce::ignoreUnused (processorRef);

    mainComponent = std::make_unique<MainComponent>(controller);
    mainComponent->setGameEngine(gameEngine);
    addAndMakeVisible(*mainComponent);

    commandManager->registerAllCommandsForTarget(this);
    commandManager->registerAllCommandsForTarget(mainComponent.get());
    commandManager->setFirstCommandTarget(this);

    menuModel = std::make_unique<LumatoneSandbox::Menu::Model>(commandManager);

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

    // commands.add(LumatoneSandbox::Menu::commandIDs::deleteOctaveBoard);
    // commands.add(LumatoneSandbox::Menu::commandIDs::copyOctaveBoard);
    // commands.add(LumatoneSandbox::Menu::commandIDs::pasteOctaveBoard);
    // commands.add(LumatoneSandbox::Menu::commandIDs::pasteOctaveBoardChannels);
    // commands.add(LumatoneSandbox::Menu::commandIDs::pasteOctaveBoardNotes);
    // commands.add(LumatoneSandbox::Menu::commandIDs::pasteOctaveBoardColours);
    // commands.add(LumatoneSandbox::Menu::commandIDs::pasteOctaveBoardTypes);

    commands.add(LumatoneSandbox::Menu::commandIDs::adjustColour);
    
    commands.add(LumatoneSandbox::Menu::commandIDs::openRandomColorsGame);
    commands.add(LumatoneSandbox::Menu::commandIDs::openHexRingsGame);
    commands.add(LumatoneSandbox::Menu::commandIDs::openHexagonAutomata);
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
                
        case LumatoneSandbox::Menu::commandIDs::openRandomColorsGame:
            result.setInfo("Random Colors", "Open launcher for Random Colors game", "Game", 0);
            break;

        case LumatoneSandbox::Menu::commandIDs::openHexRingsGame:
            result.setInfo("Hex Rings", "Open launcher for Hex Rings game", "Game", 0);
            break;

        case LumatoneSandbox::Menu::commandIDs::openHexagonAutomata:
            result.setInfo("Hexagon Automata", "Open launcher for hex game of life", "Game", 0);
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
        auto directory = controller->getLastMappingsDirectory();
        fileChooser.reset(new juce::FileChooser("Open .LTN file", directory, "*.ltn"));
        fileChooser->launchAsync(
            juce::FileBrowserComponent::FileChooserFlags::canSelectFiles | juce::FileBrowserComponent::FileChooserFlags::openMode,
            [&](const juce::FileChooser& chooser)
            {
                auto file = chooser.getResult();
                controller->loadLayoutFromFile(file);
                
            });
        return true;
    }

    case LumatoneSandbox::Menu::commandIDs::saveSysExMappingAs:
    {
        auto directory = controller->getLastMappingsDirectory();
        fileChooser.reset(new juce::FileChooser("Save .LTN file", directory, "*.ltn"));
        fileChooser->launchAsync(
            juce::FileBrowserComponent::FileChooserFlags::canSelectFiles | juce::FileBrowserComponent::FileChooserFlags::saveMode,
            [&](const juce::FileChooser& chooser)
            {
                auto file = chooser.getResult();
                auto layoutString = controller->getMappingData()
                                              ->toStringArray()
                                              .joinIntoString(juce::newLine);
                
                auto tempFile = file.createTempFile("ltn.tmp");
                tempFile.appendText(layoutString);
                tempFile.moveFileTo(file);
            });
        return true;
    }

    case LumatoneSandbox::Menu::commandIDs::importSysExMapping:
    {
        controller->sendGetCompleteMappingRequest();
        return true;
    }

    case LumatoneSandbox::Menu::commandIDs::adjustColour:
    {
        juce::DialogWindow::LaunchOptions launch;
        launch.dialogTitle = "Adjust Colours";
        launch.content.setOwned(new AdjustColourPanel(controller, paletteLibrary));
        launch.content->setSize(600, 400);

        launch.componentToCentreAround = mainComponent.get();
        launch.launchAsync();

        return true;
    }

    case LumatoneSandbox::Menu::commandIDs::openRandomColorsGame:
    {
        mainComponent->setGameComponent(new RandomColorsComponent(gameEngine));
        return true;
    }
    
    case LumatoneSandbox::Menu::commandIDs::openHexRingsGame:
    {
        mainComponent->setGameComponent(new HexRingLauncher(gameEngine));
        return true;
    }

    case LumatoneSandbox::Menu::commandIDs::openHexagonAutomata:
    {
        mainComponent->setGameComponent(new HexagonAutomataComponent(gameEngine));
        return true;
    }

    default:
        return false;
    }

    return false;
}