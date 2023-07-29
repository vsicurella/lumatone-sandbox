#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent(LumatoneController* controllerIn)
    : controller(controllerIn)
{
    connectionStatus = std::make_unique<ConnectionStatus>();
    controller->addStatusListener(connectionStatus.get());
    addAndMakeVisible(*connectionStatus);
    
    lumatoneComponent = std::make_unique<LumatoneKeyboardComponent>((LumatoneState)*controller);
    controller->addEditorListener(lumatoneComponent.get());
    addAndMakeVisible(*lumatoneComponent);

    connectionStatus->handleStatus(ConnectionState::DISCONNECTED);

    setSize (600, 400);
}

MainComponent::~MainComponent()
{
    controller->removeStatusListener(connectionStatus.get());
    controller->removeEditorListener(lumatoneComponent.get());

    lumatoneComponent = nullptr;
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
    const float connectionStatusHeight = proportionOfHeight(connectionStatusHeightRatio);
    connectionStatus->setBounds(0, 0, getWidth(), connectionStatusHeight);

    lumatoneComponent->setBounds(getLocalBounds().withTop(connectionStatus->getBottom()));
}
