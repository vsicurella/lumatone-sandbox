#pragma once

#include <JuceHeader.h>

class LumatoneApplicationState;
class LumatoneFirmwareDriver;
class DeviceActivityMonitor;
class LumatoneController;
class LumatonePaletteLibrary;
class LumatoneSandboxGameEngine;

class LumatoneSandboxLogTableModel;

//==============================================================================
class LumatoneSandboxProcessor  : public juce::AudioProcessor
                                // , public juce::ApplicationCommandTarget
{
public:
    //==============================================================================
    LumatoneSandboxProcessor();
    ~LumatoneSandboxProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================

    LumatoneSandboxLogTableModel*       getLogData() { return logData.get(); }

    juce::UndoManager*                  getUndoManager() { return undoManager.get(); }
    // juce::ApplicationCommandManager*    getCommandManager() { return commandManager.get(); }

    LumatonePaletteLibrary*         getPaletteLibrary() { return paletteLibrary.get(); }
    
    LumatoneController*             getLumatoneController() { return controller.get(); }
    DeviceActivityMonitor*          getDeviceMonitor() { return monitor.get(); }

    LumatoneSandboxGameEngine*      getGameEngine() { return gameEngine.get(); }

    //==============================================================================

    // juce::ApplicationCommandTarget* getNextCommandTarget() override;

    // void getAllCommands(juce::Array<juce::CommandID>& commands) override;

    // void getCommandInfo(juce::CommandID commandID, juce::ApplicationCommandInfo &result) override;

    // bool perform(const juce::ApplicationCommandTarget::InvocationInfo &info) override;


private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LumatoneSandboxProcessor)

    bool isStandalone = false;

    std::unique_ptr<LumatoneSandboxLogTableModel> logData;
    
    std::unique_ptr<juce::UndoManager> undoManager;
    // std::unique_ptr<juce::ApplicationCommandManager> commandManager;

    juce::ValueTree treeState;
    std::unique_ptr<LumatoneApplicationState> appState;

    std::unique_ptr<LumatonePaletteLibrary> paletteLibrary;

    std::unique_ptr<LumatoneFirmwareDriver> midiDriver;
    std::unique_ptr<LumatoneController> controller;
    std::unique_ptr<DeviceActivityMonitor> monitor;

    std::unique_ptr<LumatoneSandboxGameEngine> gameEngine;
};
