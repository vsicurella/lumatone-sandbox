/*
  ==============================================================================

    connection_status.h
    Created: 4 Jun 2023 4:54:22pm
    Author:  Vincenzo

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../lumatone_editor_library/ApplicationListeners.h"
#include "../lumatone_editor_library/data/lumatone_state.h"

//==============================================================================
/*
*/
class ConnectionStatus : public juce::Component,
    public LumatoneEditor::StatusListener
{
public:
    ConnectionStatus();
    ~ConnectionStatus() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    void handleStatus(ConnectionState state);

    //==============================================================================
    // LumatoneEditor::StatusListener

    // virtual void connectionFailed() override;
    void connectionStateChanged(ConnectionState newState) override;
    // virtual void connectionEstablished(juce::String inputDeviceId, juce::String outputDeviceId) override;
    // virtual void connectionLost() override;

private:

    juce::String stateToMessage(ConnectionState state);
    juce::Colour stateToColour(ConnectionState state);

private:

    juce::String message;
    juce::Colour statusColour;

    juce::Rectangle<int> messageBounds;
    juce::Rectangle<float> statusBounds;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ConnectionStatus)

};
