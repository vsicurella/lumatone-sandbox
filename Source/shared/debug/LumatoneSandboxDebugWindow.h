#ifndef LUMATONE_SANDBOX_DEBUG_WINDOW_H
#define LUMATONE_SANDBOX_DEBUG_WINDOW_H

#include <JuceHeader.h>

class LumatoneSandboxLogTableModel;

class LumatoneSandboxDebugWindow : public juce::DocumentWindow, private juce::ChangeListener
{
public:

    LumatoneSandboxDebugWindow(LumatoneSandboxLogTableModel* logTableModelIn);
    ~LumatoneSandboxDebugWindow() override {}

private:

    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

private:

    LumatoneSandboxLogTableModel* logModel;
    // std::unique_ptr<juce::TableListBox> logTable;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LumatoneSandboxDebugWindow)
};

#endif // LUMATONE_SANDBOX_DEBUG_WINDOW_H
