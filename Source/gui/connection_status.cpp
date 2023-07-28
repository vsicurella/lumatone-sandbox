/*
  ==============================================================================

    connection_status.cpp
    Created: 4 Jun 2023 4:54:22pm
    Author:  Vincenzo

  ==============================================================================
*/

#include "connection_status.h"

//==============================================================================
ConnectionStatus::ConnectionStatus()
{

}

ConnectionStatus::~ConnectionStatus()
{

}

void ConnectionStatus::paint(juce::Graphics& graphics)
{
    graphics.setColour(juce::Colours::darkgrey);
    graphics.fillAll();

    graphics.setColour(statusColour);
    graphics.fillEllipse(statusBounds);

    graphics.setColour(juce::Colours::white);
    graphics.drawFittedText(message, messageBounds, juce::Justification::left, 1);
}

void ConnectionStatus::resized()
{
    const float statusHeight = proportionOfHeight(0.25f);
    const float messageHeight = proportionOfHeight(0.3f);

    const float statusRadius = statusHeight * 0.5f;

    const int middleY = getHeight() * 0.5f;
    statusBounds = juce::Rectangle<float>(statusHeight, statusHeight)
                          .withPosition(middleY - statusRadius, middleY - statusRadius);

    const float messageHalfY = messageHeight * 0.5f;
    messageBounds = juce::Rectangle<int>(juce::roundToInt(middleY * 2 - statusRadius),
                                         juce::roundToInt(middleY - messageHalfY), 
                                         getWidth() - statusBounds.getWidth() - statusHeight, 
                                         messageHeight);
}

void ConnectionStatus::handleStatus(LumatoneSandboxProperty::ConnectionState state)
{
    message = stateToMessage(state);
    statusColour = stateToColour(state);
    repaint();
}

juce::String ConnectionStatus::stateToMessage(LumatoneSandboxProperty::ConnectionState state)
{
    switch (state)
    {
    case LumatoneSandboxProperty::ConnectionState::DISCONNECTED:
        return "No Device Present";

    case LumatoneSandboxProperty::ConnectionState::SEARCHING:
        return "Searching...";

    case LumatoneSandboxProperty::ConnectionState::OFFLINE:
        return "Offline";

    case LumatoneSandboxProperty::ConnectionState::ONLINE:
        return "Online";

    case LumatoneSandboxProperty::ConnectionState::BUSY:
        return "Busy";

    case LumatoneSandboxProperty::ConnectionState::ERROR:
        return "Error";
    }

    return juce::String();
}

juce::Colour ConnectionStatus::stateToColour(LumatoneSandboxProperty::ConnectionState state)
{
    switch (state)
    {
    case LumatoneSandboxProperty::ConnectionState::DISCONNECTED:
        return juce::Colours::grey;

    case LumatoneSandboxProperty::ConnectionState::SEARCHING:
        return juce::Colours::orange;

    case LumatoneSandboxProperty::ConnectionState::OFFLINE:
        return juce::Colours::darkgreen;

    case LumatoneSandboxProperty::ConnectionState::ONLINE:
        return juce::Colours::green;

    case LumatoneSandboxProperty::ConnectionState::BUSY:
        return juce::Colours::yellow;

    case LumatoneSandboxProperty::ConnectionState::ERROR:
        return juce::Colours::red;
    }

    return juce::Colour();
}

//==============================================================================
// LumatoneEditor::StatusListener

void ConnectionStatus::connectionFailed()
{
    handleStatus(LumatoneSandboxProperty::ConnectionState::DISCONNECTED);
}

void ConnectionStatus::connectionEstablished(juce::String inputDeviceId, juce::String outputDeviceId)
{
    handleStatus(LumatoneSandboxProperty::ConnectionState::ONLINE);
}

void ConnectionStatus::connectionLost()
{
    handleStatus(LumatoneSandboxProperty::ConnectionState::ERROR);
}

