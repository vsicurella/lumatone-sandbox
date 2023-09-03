#include "MainComponent.h"
#include "SandboxMenu.h"

//==============================================================================
MainComponent::MainComponent(LumatoneController* controllerIn)
    : controller(controllerIn)
    , colourAdjust(controllerIn)
{
    connectionStatus = std::make_unique<ConnectionStatus>();
    controller->addStatusListener(connectionStatus.get());
    addAndMakeVisible(*connectionStatus);
    
    lumatoneComponent = std::make_unique<LumatoneKeyboardComponent>((LumatoneState)*controller);
    addAndMakeVisible(*lumatoneComponent);

    controller->addEditorListener(lumatoneComponent.get());
    controller->addMidiListener(lumatoneComponent.get());

    connectionStatus->handleStatus(ConnectionState::DISCONNECTED);

    setSize (600, 400);
}

MainComponent::~MainComponent()
{
    controller->removeStatusListener(connectionStatus.get());
    controller->removeEditorListener(lumatoneComponent.get());

    gameEngineComponent = nullptr;

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
    const float connectionStatusHeight = 32.0f;// proportionOfHeight(connectionStatusHeightRatio);
    connectionStatus->setBounds(0, 0, getWidth(), connectionStatusHeight);

    bool gameIsShown = (gameEngineComponent != nullptr) && showGameControl;
    float lumatoneHeight = 1.0f;
    if (gameIsShown)
        lumatoneHeight -= gameControlHeightRatio;

    const float widthMargin = proportionOfWidth(lumatoneComponentWidthMarginRatio * 0.5f);
    lumatoneComponent->setBounds(widthMargin, connectionStatus->getBottom(), getWidth() - widthMargin * 2, getHeight() * lumatoneHeight - connectionStatusHeight * 2);

    if (gameIsShown)
    {
        auto gameHeight = getHeight() - lumatoneComponent->getBottom();
        gameEngineComponent->setBounds(0, lumatoneComponent->getBottom(), getWidth(), gameHeight);
    }
}

void MainComponent::setGameEngine(LumatoneSandboxGameEngine* engineIn)
{
    jassert(gameEngineComponent.get() == nullptr);
    gameEngineComponent = std::make_unique<LumatoneSandboxGameEngineComponent>(engineIn);
    if (showGameControl)
        addAndMakeVisible(*gameEngineComponent);
    else
        addChildComponent(*gameEngineComponent);

    resized();
}

void MainComponent::setGameComponent(LumatoneSandboxGameComponent *gameIn)
{
    gameEngineComponent->setGameComponent(gameIn);
    gameLoadedCallback();
}

void MainComponent::gameLoadedCallback()
{
    jassert(gameEngineComponent != nullptr);
    if (showGameControl)
        gameEngineComponent->resized();
}

void MainComponent::getAllCommands(juce::Array <juce::CommandID>& commands)
{
    commands.add(LumatoneSandbox::Menu::commandIDs::setRenderModeKeys);
    commands.add(LumatoneSandbox::Menu::commandIDs::setRenderModeMaxRes);
}

void MainComponent::getCommandInfo(juce::CommandID commandID, juce::ApplicationCommandInfo& result)
{
    // result.setActive(false);

    switch (commandID)
        {
        case LumatoneSandbox::Menu::commandIDs::setRenderModeKeys:
            result.setInfo("Render by key", "Render Lumatone graphic as individual components", "View", 0);
            result.setTicked(lumatoneComponent->getRenderMode() != LumatoneComponentRenderMode::MaxRes);
            break;

        case LumatoneSandbox::Menu::commandIDs::setRenderModeMaxRes:
            result.setInfo("Render max size", "Render Lumatone graphic at max quality", "View", 0);
            result.setTicked(lumatoneComponent->getRenderMode() == LumatoneComponentRenderMode::MaxRes);
            break;

        default:
            break;
        }
}

bool MainComponent::perform(const juce::ApplicationCommandTarget::InvocationInfo& info)
{
    switch (info.commandID)
    {
    default:
        return false;

    case LumatoneSandbox::Menu::commandIDs::setRenderModeKeys:
    {
        lumatoneComponent->setRenderMode(LumatoneComponentRenderMode::GraphicInteractive);
        return true;
    }
    case LumatoneSandbox::Menu::commandIDs::setRenderModeMaxRes:
    {
        lumatoneComponent->setRenderMode(LumatoneComponentRenderMode::MaxRes);
        return true;
    }
    }
}

void MainComponent::setShowGameControl(bool showControls)
{
    if (gameEngineComponent != nullptr && gameEngineComponent->isVisible() != showControls)
    {
        gameEngineComponent->setVisible(showControls);
        resized();
    }
}
