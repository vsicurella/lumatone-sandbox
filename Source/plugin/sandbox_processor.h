#pragma once

#include <JuceHeader.h>

#include "../shared/SandboxState.h"

class LumatoneApplicationState;
class LumatoneFirmwareDriver;
class DeviceActivityMonitor;
class LumatoneController;
class LumatonePaletteLibrary;
class LumatoneSandboxGameEngine;

class LumatoneSandboxLogTableModel;

//==============================================================================
class LumatoneSandboxProcessor  : public juce::AudioProcessor
                                , private LumatoneSandboxState::Controller
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

    //==============================================================================
private:
    bool isStandalone = false;

	std::shared_ptr<LumatoneFirmwareDriver>	midiDriver;
	juce::UndoManager 		undoManager;

	LumatoneSandboxState		state;

    std::unique_ptr<LumatoneSandboxLogTableModel> logData;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LumatoneSandboxProcessor)
};
