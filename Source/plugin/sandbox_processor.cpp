#include "sandbox_processor.h"
#include "sandbox_editor.h"

#include "../shared/game/game_engine.h"
#include "../shared/lumatone_editor_library/lumatone_midi_driver/lumatone_midi_driver.h"
#include "../shared/lumatone_editor_library/palettes/palette_library.h"
#include "../shared/lumatone_editor_library/DeviceActivityMonitor.h"
#include "../shared/lumatone_editor_library/LumatoneController.h"
#include "../shared/SandboxMenu.h"

//==============================================================================
LumatoneSandboxProcessor::LumatoneSandboxProcessor()
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
{
    treeState = juce::ValueTree(LumatoneStateProperty::StateTree);
    appState = std::make_unique<LumatoneApplicationState>("LumatoneSandboxProcessor", treeState);

    undoManager = std::make_unique<juce::UndoManager>();

    paletteLibrary = std::make_unique<LumatonePaletteLibrary>();

    isStandalone = (juce::PluginHostType::getPluginLoadedAs() == AudioProcessor::wrapperType_Standalone);

    midiDriver = std::make_unique<LumatoneFirmwareDriver>(isStandalone
               ? LumatoneFirmwareDriver::HostMode::Driver
               : LumatoneFirmwareDriver::HostMode::Plugin);

    controller = std::make_unique<LumatoneController>(*appState, *midiDriver, undoManager.get());

    monitor = std::make_unique<DeviceActivityMonitor>(midiDriver.get(), (LumatoneApplicationState)*controller.get()); 
    monitor->addStatusListener(controller.get());
    monitor->startDeviceDetection();

    commandManager = std::make_unique<juce::ApplicationCommandManager>();
    commandManager->registerAllCommandsForTarget(this);
  
    gameEngine = std::make_unique<LumatoneSandboxGameEngine>(controller.get(), 30);
}

LumatoneSandboxProcessor::~LumatoneSandboxProcessor()
{
    gameEngine = nullptr;
    monitor = nullptr;
    
    controller = nullptr;
    midiDriver = nullptr;

    paletteLibrary = nullptr;

    commandManager = nullptr;
    undoManager = nullptr;
}

//==============================================================================
const juce::String LumatoneSandboxProcessor::getName() const
{
    return JucePlugin_Name;
}

bool LumatoneSandboxProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool LumatoneSandboxProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool LumatoneSandboxProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double LumatoneSandboxProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int LumatoneSandboxProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int LumatoneSandboxProcessor::getCurrentProgram()
{
    return 0;
}

void LumatoneSandboxProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String LumatoneSandboxProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void LumatoneSandboxProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

//==============================================================================
void LumatoneSandboxProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    juce::ignoreUnused (sampleRate, samplesPerBlock);
}

void LumatoneSandboxProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool LumatoneSandboxProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}

void LumatoneSandboxProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    if (!isStandalone)
    {
        for (auto msg : midiMessages)
        {
            midiDriver->handleIncomingMidiMessage(nullptr, msg.getMessage());
        }
    }

    midiDriver->readNextBuffer(midiMessages);
}

//==============================================================================
bool LumatoneSandboxProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* LumatoneSandboxProcessor::createEditor()
{
    return new LumatoneSandboxProcessorEditor (*this);
}

//==============================================================================
void LumatoneSandboxProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream stream(destData, false);
    treeState.writeToStream(stream);
    DBG("Wrote to memory:\n" + treeState.toXmlString());
}

void LumatoneSandboxProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto state = juce::ValueTree::readFromData(data, sizeInBytes);
    DBG("Read from memory:\n" + treeState.toXmlString());

    controller->loadStateProperties(state);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new LumatoneSandboxProcessor();
}

juce::ApplicationCommandTarget* LumatoneSandboxProcessor::getNextCommandTarget()
{ 
    return this;
}

void LumatoneSandboxProcessor::getAllCommands(juce::Array <juce::CommandID>& commands)
{
    commands.add(LumatoneSandbox::Menu::commandIDs::undo);
    commands.add(LumatoneSandbox::Menu::commandIDs::redo);
    commands.add(LumatoneSandbox::Menu::commandIDs::aboutSysEx);
    commands.add(juce::StandardApplicationCommandIDs::quit);
    // commands.add(LumatoneSandbox::Menu::commandIDs::aboutSysEx);
}

void LumatoneSandboxProcessor::getCommandInfo(juce::CommandID commandID, juce::ApplicationCommandInfo& result)
{
    result.setActive(true);

    switch (commandID)
        {
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

        case LumatoneSandbox::Menu::commandIDs::aboutSysEx:
            result.setInfo("About Lumatone Editor", "Shows version and copyright", "Help", 0);
            break;

        case juce::StandardApplicationCommandIDs::quit:
            result.setInfo("Quit", "Close window and terminate application", "File", 0);
            break;

         default:
            result.setInfo("?", "Unknown command", "Unknown", 0);  
            break;
        }
}

bool LumatoneSandboxProcessor::perform(const juce::ApplicationCommandTarget::InvocationInfo& info)
{
    switch (info.commandID)
    {
    
    case LumatoneSandbox::Menu::commandIDs::undo:
        undoManager->undo();
        return true;

    case LumatoneSandbox::Menu::commandIDs::redo:
        undoManager->redo();
        return true;

    case juce::StandardApplicationCommandIDs::quit:
        juce::JUCEApplication::quit();
        return true;

    default:
        return false;
    }

    return false;
}