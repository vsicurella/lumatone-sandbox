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

void ConnectionStatus::handleStatus(ConnectionState state)
{
    message = stateToMessage(state);
    statusColour = stateToColour(state);
    repaint();
}

juce::String ConnectionStatus::stateToMessage(ConnectionState state)
{
    switch (state)
    {
    case ConnectionState::DISCONNECTED:
        return "No Device Present";

    case ConnectionState::SEARCHING:
        return "Searching...";

    case ConnectionState::OFFLINE:
        return "Offline";

    case ConnectionState::ONLINE:
        return "Online";

    case ConnectionState::BUSY:
        return "Busy";

    case ConnectionState::ERROR:
        return "Error";
    }

    return juce::String();
}

juce::Colour ConnectionStatus::stateToColour(ConnectionState state)
{
    switch (state)
    {
    case ConnectionState::DISCONNECTED:
        return juce::Colours::grey;

    case ConnectionState::SEARCHING:
        return juce::Colours::orange;

    case ConnectionState::OFFLINE:
        return juce::Colours::darkgreen;

    case ConnectionState::ONLINE:
        return juce::Colours::green;

    case ConnectionState::BUSY:
        return juce::Colours::yellow;

    case ConnectionState::ERROR:
        return juce::Colours::red;
    }

    return juce::Colour();
}

//==============================================================================
// LumatoneEditor::StatusListener

void ConnectionStatus::connectionFailed()
{
    handleStatus(ConnectionState::DISCONNECTED);
}

void ConnectionStatus::connectionEstablished(juce::String inputDeviceId, juce::String outputDeviceId)
{
    handleStatus(ConnectionState::ONLINE);
}

void ConnectionStatus::connectionLost()
{
    handleStatus(ConnectionState::ERROR);
}

