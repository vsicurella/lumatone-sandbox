#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent(LumatoneController* controllerIn)
    : controller(controllerIn)
{
    connectionStatus = std::make_unique<ConnectionStatus>();
    controller->addStatusListener(connectionStatus.get());
    addAndMakeVisible(*connectionStatus);
    
    setSize (600, 400);

    connectionStatus->handleStatus(ConnectionState::DISCONNECTED);
}

MainComponent::~MainComponent()
{
    controller->removeStatusListener(connectionStatus.get());

    connectionStatus = nullptr;
    controller = nullptr;
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setFont (juce::Font (16.0f));
    g.setColour (juce::Colours::white);
    g.drawText ("Hello World!", getLocalBounds(), juce::Justification::centred, true);
}

void MainComponent::resized()
{
    // This is called when the MainComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.

    connectionStatus->setBounds(0, 0, getWidth(), proportionOfHeight(0.1));
}
