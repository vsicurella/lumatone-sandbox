#include "MainComponent.h"
#include "SandboxMenu.h"

#include "./lumatone_editor_library/device/lumatone_controller.h"
#include "./lumatone_editor_library/ui/keyboard_component.h"
#include "./gui/connection_status.h"

#include "./game_engine/game_engine_component.h"
#include "./games/game_base.h"

//==============================================================================
MainComponent::MainComponent(const LumatoneSandboxState& stateIn)
    : LumatoneSandboxState("MainComponent", stateIn)
    , colourAdjust(*this)
{
    connectionStatus = std::make_unique<ConnectionStatus>();
    addStatusListener(connectionStatus.get());

    addAndMakeVisible(*connectionStatus);
    connectionStatus->handleStatus(getConnectionState());
    
    lumatoneComponent = std::make_unique<LumatoneKeyboardComponent>(*this);
    addAndMakeVisible(*lumatoneComponent);

    setGameEngine(getGameEngine());
}

MainComponent::~MainComponent()
{
    removeStatusListener(connectionStatus.get());

    if (gameEngineComponent != nullptr && gameEngineComponent->getGameEngine() != nullptr)
    {
        gameEngineComponent->getGameEngine()->removeEngineListener(this);
    }

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
    engineIn->addEngineListener(this);
    engineIn->setVirtualKeyboard(lumatoneComponent.get());

    jassert(gameEngineComponent.get() == nullptr);
    gameEngineComponent = std::make_unique<LumatoneSandboxGameEngineComponent>(engineIn);

    if (showGameControl)
        addAndMakeVisible(*gameEngineComponent);
    else
        addChildComponent(*gameEngineComponent);

    resized();
}


void MainComponent::gameLoadedCallback(LumatoneSandboxGameBase* game)
{
    jassert(gameEngineComponent != nullptr);

    gameEngineComponent->setGameComponent(game->createController());

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

void MainComponent::gameStatusChanged(LumatoneSandboxGameBase* game, LumatoneGameEngineState::GameStatus status)
{
    switch (status)
    {
    case LumatoneGameEngineState::GameStatus::Loaded:
        gameLoadedCallback(game);
        lumatoneComponent->setUiMode(LumatoneKeyboardComponent::UiMode::Controller);

        break;
    case LumatoneGameEngineState::GameStatus::Stopped:
        lumatoneComponent->setUiMode(LumatoneKeyboardComponent::UiMode::Perform);

    default: 
        return;
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
