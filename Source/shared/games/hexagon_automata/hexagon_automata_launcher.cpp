/*
  ==============================================================================

    hexagon_automata_launcher.cpp
    Created: 27 Aug 2023 9:27:26pm
    Author:  Vincenzo

  ==============================================================================
*/

#include "hexagon_automata_launcher.h"
#include "./hexagon_automata.h"

//==============================================================================
HexagonAutomata::Component::Component(HexagonAutomata::Game* gameIn)
    : LumatoneSandboxGameComponent(gameIn)
    , game(gameIn)
    , HexagonAutomata::State(*gameIn)
{
    gameModeSelector = std::make_unique<juce::ComboBox>("Game Mode Selector");
    gameModeSelector->addItem("Classic", static_cast<int>(HexagonAutomata::GameMode::Classic));
    gameModeSelector->addItem("Layout Sequencer", static_cast<int>(HexagonAutomata::GameMode::Sequencer));
    gameModeSelector->setSelectedId(static_cast<int>(game->getGameMode()));
    gameModeSelector->onChange = [&]
    {
        auto mode = static_cast<HexagonAutomata::GameMode>(gameModeSelector->getSelectedId());
        game->setGameMode(mode);
    };
    addAndMakeVisible(*gameModeSelector);

    gameModeLabel = std::make_unique<juce::Label>("Game Mode Label", "Game Mode");
    gameModeLabel->setJustificationType(juce::Justification::centredLeft);
    gameModeLabel->attachToComponent(gameModeSelector.get(), true);
    addAndMakeVisible(*gameModeSelector);

    generationModeSelector = std::make_unique<juce::ComboBox>("Generation Mode Selector");
    generationModeSelector->addItem("Pulse", static_cast<int>(HexagonAutomata::GenerationMode::Synchronous));
    generationModeSelector->addItem("Rhythmic", static_cast<int>(HexagonAutomata::GenerationMode::Asynchronous));
    generationModeSelector->setSelectedId(static_cast<int>(game->getGenerationMode()));
    generationModeSelector->onChange = [&]
    {
        auto mode = static_cast<HexagonAutomata::GenerationMode>(generationModeSelector->getSelectedId());
        game->setGenerationMode(mode);
    };
    addAndMakeVisible(*generationModeSelector);
    
    generationModeLabel = std::make_unique<juce::Label>("Generation Mode Label", "Style");
    generationModeLabel->setJustificationType(juce::Justification::centredLeft);
    generationModeLabel->attachToComponent(generationModeSelector.get(), true);
    addAndMakeVisible(*generationModeLabel);

    rulesModeSelector = std::make_unique<juce::ComboBox>("Rules Mode Selector");
    rulesModeSelector->addItem("Born/Survive", static_cast<int>(HexagonAutomata::RulesMode::BornSurvive));
    rulesModeSelector->addItem("Spiral", static_cast<int>(HexagonAutomata::RulesMode::SpiralRule));
    rulesModeSelector->setSelectedId(static_cast<int>(game->getRulesMode()));
    rulesModeSelector->onChange = [&]
    {
        auto mode = static_cast<HexagonAutomata::RulesMode>(rulesModeSelector->getSelectedId());
        game->setRulesMode(mode);
    };
    addAndMakeVisible(*rulesModeSelector);
    
    rulesModeLabel = std::make_unique<juce::Label>("Generation Mode Label", "Style");
    rulesModeLabel->setJustificationType(juce::Justification::centredLeft);
    rulesModeLabel->attachToComponent(rulesModeSelector.get(), true);
    addAndMakeVisible(*rulesModeLabel);

    addSeedButton = std::make_unique<juce::TextButton>("Add Seeds", "Add a cluster of cells with 50% per cell");
    addSeedButton->onClick = [&]
    {
        game->addSeeds((int)numSeedsSlider->getValue());
    };
    addAndMakeVisible(*addSeedButton);

    numSeedsSlider = std::make_unique<juce::Slider>(juce::Slider::SliderStyle::IncDecButtons, juce::Slider::TextEntryBoxPosition::TextBoxLeft);
    numSeedsSlider->setRange(1, 280, 1);
    numSeedsSlider->setValue(1);
    addAndMakeVisible(*numSeedsSlider);

    genSpeedSlider = std::make_unique<juce::Slider>(juce::Slider::SliderStyle::IncDecButtons, juce::Slider::TextEntryBoxPosition::TextBoxLeft);
    genSpeedSlider->setRange(1, 300, 1);
    genSpeedSlider->setValue(game->getGenerationBpm());
    genSpeedSlider->onValueChange = [&]
    {
        // game->setTicksPerAsyncGeneration(genSpeedSlider->getValue());
        speedSliderCallback();
    };
    addAndMakeVisible(*genSpeedSlider);

    genSpeedLabel = std::make_unique<juce::Label>("Generation Speed Label", "BPM:");
    genSpeedLabel->setJustificationType(juce::Justification::centredLeft);
    genSpeedLabel->attachToComponent(genSpeedSlider.get(), true);
    addAndMakeVisible(*genSpeedLabel);

    bornRuleInput = std::make_unique<juce::TextEditor>("BornRuleText");
    bornRuleInput->setInputRestrictions(0, "0123456789, ");
    bornRuleInput->setText("2");
    bornRuleInput->onTextChange = [&]
    {
        onRulesChange();
    };
    addAndMakeVisible(*bornRuleInput);

    bornRuleLabel = std::make_unique<juce::Label>("Cell Born Rule Label", "Born Rules:");
    bornRuleLabel->setJustificationType(juce::Justification::centredLeft);
    bornRuleLabel->attachToComponent(bornRuleInput.get(), true);
    addAndMakeVisible(*bornRuleLabel);
    
    surviveRuleInput = std::make_unique<juce::TextEditor>("SurviveRuleText");
    surviveRuleInput->setInputRestrictions(0, "0123456789, ");
    surviveRuleInput->setText("3,4");
    surviveRuleInput->onTextChange = [&]
    {
        onRulesChange();
    };
    addAndMakeVisible(*surviveRuleInput);

    surviveRuleLabel = std::make_unique<juce::Label>("Cell Survive Rule Label", "Survive Rules:");
    surviveRuleLabel->setJustificationType(juce::Justification::centredLeft);
    surviveRuleLabel->attachToComponent(surviveRuleInput.get(), true);
    addAndMakeVisible(*surviveRuleLabel);

    distanceSlider = std::make_unique<juce::Slider>(juce::Slider::SliderStyle::IncDecButtons, juce::Slider::TextBoxLeft);
    distanceSlider->setRange(0, 7, 1);
    distanceSlider->setValue(1, juce::NotificationType::dontSendNotification);
    distanceSlider->onValueChange = [&]
    {
        game->setNeighborDistance(static_cast<int>(distanceSlider->getValue()));
    };
    addAndMakeVisible(*distanceSlider);

    distanceLabel = std::make_unique<juce::Label>("Neighbor Distance Label", "Neighbors:");
    distanceLabel->setJustificationType(juce::Justification::centredLeft);
    distanceLabel->attachToComponent(distanceSlider.get(), true);
    addAndMakeVisible(*distanceLabel);

    aliveColourSelector = std::make_unique<CustomPickerPanel>();
    aliveColourSelector->setCurrentColour(game->getAliveColour());
    aliveColourSelector->addColourSelectionListener(this);
    addAndMakeVisible(*aliveColourSelector);

    aliveColourLabel = std::make_unique<juce::Label>("Alive Color Label", "Cell Alive Color:");
    aliveColourLabel->setJustificationType(juce::Justification::centredLeft);
    aliveColourLabel->attachToComponent(aliveColourSelector.get(), false);
    addAndMakeVisible(*aliveColourLabel);

    deadColourSelector = std::make_unique<CustomPickerPanel>();
    deadColourSelector->setCurrentColour(game->getDeadColour());
    deadColourSelector->addColourSelectionListener(this);
    addAndMakeVisible(*deadColourSelector);

    deadColourLabel = std::make_unique<juce::Label>("Dead Color Label", "Cell Dead Color:");
    deadColourLabel->setJustificationType(juce::Justification::centredLeft);
    deadColourLabel->attachToComponent(deadColourSelector.get(), false);
    addAndMakeVisible(*deadColourLabel);

    game->setRulesMode(HexagonAutomata::RulesMode::SpiralRule);
}

HexagonAutomata::Component::~Component()
{ 
    game = nullptr;

    genSpeedSlider = nullptr;
    addSeedButton = nullptr;
}

inline int HexagonAutomata::Component::getModeColumnWidth(juce::Font font) const
{
    int modeWidth = 0;

    juce::ComboBox* boxes[] = { gameModeSelector.get(), generationModeSelector.get(), nullptr };
    juce::ComboBox** box = boxes;

    do 
    {
        // auto font = getLookAndFeel().getComboBoxFont(**box);
        for (int i = 0; i < (*box)->getNumItems(); i++)
        {
            auto r =(*box)->getItemText(i);
            int itemWidth = font.getStringWidth(r);
            if (itemWidth > modeWidth)
                modeWidth = itemWidth;
        }
    } 
    while (*(++box) != nullptr);

    return modeWidth;
}

void HexagonAutomata::Component::resized()
{
    LumatoneSandboxGameComponent::resized();

    int margin = controlsArea.proportionOfHeight(marginScalar);
    int labelMargin = margin / 2;
    int controlLeft = controlsArea.getX();

    flexArea = controlsArea.withTrimmedTop(labelMargin);

    float controlHeightScalar = 0.16f;
    int controlHeight = juce::roundToInt(flexArea.proportionOfHeight(controlHeightScalar));

    float buttonHeightScalar = 0.18f;
    int buttonHeight = juce::roundToInt(flexArea.proportionOfHeight(buttonHeightScalar));

    auto labelFont = getLookAndFeel().getLabelFont(*genSpeedLabel).withHeight(controlHeight);

    float gameModeLabelWidth = labelFont.getStringWidthFloat(gameModeLabel->getText());
    float generationModeWidth = labelFont.getStringWidthFloat(generationModeLabel->getText());
    float genSpeedLabelWidth = labelFont.getStringWidthFloat(genSpeedLabel->getText());
    // float numSeedsLabelWidth = labelFont.getStringWidthFloat(numSeedsLabel->getText() );
    float bornRuleLabelWidth = labelFont.getStringWidthFloat(bornRuleLabel->getText());
    float surviveRuleLabelWidth = labelFont.getStringWidthFloat(surviveRuleLabel->getText());
    float distanceLabelWidth = labelFont.getStringWidthFloat(distanceLabel->getText());
    
    gameModeLabel->setSize(gameModeLabelWidth, controlHeight);
    generationModeLabel->setSize(generationModeWidth, controlHeight);
    genSpeedLabel->setSize(genSpeedLabelWidth, controlHeight);
    // numSeedsLabel->setSize(numSeedsLabelWidth, buttonHeight);
    bornRuleLabel->setSize(bornRuleLabelWidth, buttonHeight);
    surviveRuleLabel->setSize(surviveRuleLabelWidth, buttonHeight);
    distanceLabel->setSize(distanceLabelWidth, controlHeight);

    int modeWidth = getModeColumnWidth(labelFont);

    auto buttonFont = getLookAndFeel().getTextButtonFont(*addSeedButton, buttonHeight);
    int addLength = juce::roundToInt(buttonFont.getStringWidthFloat(addSeedButton->getButtonText()) * 1.5f);

    int modeControlsMaxWidth = flexArea.proportionOfWidth(0.4f) - margin;

    auto textLength = buttonFont.getStringWidth("1, 2, 3, 4");
    int controlsMaxWidth = textLength * 2;

    juce::Grid grid;

    grid.autoRows = juce::Grid::TrackInfo(juce::Grid::Px(controlHeight + margin));
    // grid.alignItems = juce::Grid::AlignItems::stretch;
    
    int modeWidthMargin = 0;
    if (gameModeLabelWidth > modeWidthMargin)
        modeWidthMargin = gameModeLabelWidth; 
    if (generationModeWidth > modeWidthMargin)
        modeWidthMargin = generationModeWidth;
    // if (genSpeedLabelWidth > modeWidthMargin)
    //     modeWidthMargin = genSpeedLabelWidth;
    grid.templateColumns.add(juce::Grid::TrackInfo(modeColumnId + "margin", juce::Grid::Px(modeWidthMargin)));
    grid.templateColumns.add(juce::Grid::TrackInfo(modeColumnId, juce::Grid::Px(modeWidth)));
    juce::GridItem::StartAndEndProperty modeColumnProperties = { 2 };

    int rulesWidthMargin = 0;
    if (bornRuleLabelWidth > rulesWidthMargin)
        rulesWidthMargin = bornRuleLabelWidth; 
    if (surviveRuleLabelWidth > rulesWidthMargin)
        rulesWidthMargin = surviveRuleLabelWidth; 
    if (distanceLabelWidth > rulesWidthMargin)
        rulesWidthMargin = distanceLabelWidth;
    grid.templateColumns.add(juce::Grid::TrackInfo(rulesColumnId + "margin", juce::Grid::Px(rulesWidthMargin)));
    grid.templateColumns.add(juce::Grid::TrackInfo(rulesColumnId, juce::Grid::Px(controlsMaxWidth)));
    juce::GridItem::StartAndEndProperty rulesColumnProperties = { 4 };

    juce::GridItem gItemTemplate;
    gItemTemplate.height = controlHeight;
    gItemTemplate.alignSelf = juce::GridItem::AlignSelf::center;
    gItemTemplate.justifySelf = juce::GridItem::JustifySelf::start;

    auto gameModeItem = juce::GridItem(gItemTemplate);
    gameModeItem.associatedComponent = gameModeSelector.get();

    juce::GridItem genModeItem(gItemTemplate);
    genModeItem.associatedComponent = generationModeSelector.get();

    juce::GridItem rulesModeItem(gItemTemplate);
    rulesModeItem.associatedComponent = rulesModeSelector.get();

    juce::GridItem addSeedItem(gItemTemplate);
    addSeedItem.associatedComponent = addSeedButton.get();
    addSeedItem.height = buttonHeight;
    addSeedItem.width = addLength;

    juce::GridItem bornItem(gItemTemplate);
    bornItem.associatedComponent = bornRuleInput.get();
    bornItem.width = textLength;
    // bornItem.maxWidth = controlsMaxWidth;

    juce::GridItem surviveItem(gItemTemplate);
    surviveItem.associatedComponent = surviveRuleInput.get();
    surviveItem.width = textLength;
    // surviveItem.maxWidth = controlsMaxWidth;

    juce::GridItem neighborsItem(gItemTemplate);
    neighborsItem.associatedComponent = distanceSlider.get();
    // neighborsItem.width = controlsMaxWidth;

    juce::GridItem genSpeedItem(gItemTemplate);
    genSpeedItem.associatedComponent = genSpeedSlider.get();
    genSpeedItem.height = buttonHeight;

    int rowNum = 1;

    grid.items.add(gameModeItem.withColumn(modeColumnProperties).withRow({ rowNum++ }));
    grid.items.add(genModeItem.withColumn(modeColumnProperties).withRow({ rowNum++ }));
    grid.items.add(rulesModeItem.withColumn(modeColumnProperties).withRow({ rowNum++ }));

    rowNum = 1;
    grid.items.add(bornItem.withColumn(rulesColumnProperties).withRow({ rowNum++ }));
    grid.items.add(surviveItem.withColumn(rulesColumnProperties).withRow({ rowNum++ }));
    grid.items.add(neighborsItem.withColumn(rulesColumnProperties).withRow({ rowNum++ }));

    grid.items.add(addSeedItem.withColumn({ 1, 3 }).withRow({ rowNum }));
    grid.items.add(genSpeedItem.withColumn({ 4, 5 }).withRow({ rowNum }));
    rowNum++;

    auto leftControlsArea = flexArea.withWidth(modeControlsMaxWidth);
    grid.performLayout(leftControlsArea);

    // Post-mode control resize tweaks

    distanceSlider->setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxLeft, false, distanceSlider->getWidth() * 0.4f, distanceSlider->getHeight());

    numSeedsSlider->setBounds(addSeedButton->getRight(), addSeedButton->getY(), (modeWidth + modeWidthMargin) - addSeedButton->getWidth(), buttonHeight);
    numSeedsSlider->setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxLeft, false, modeWidth * 0.4f, buttonHeight);

    if (game->getGameMode() == HexagonAutomata::GameMode::Classic)
    {
        int colourControlsWidth = flexArea.getWidth() - modeControlsMaxWidth - margin;
        juce::Grid coloursGrid;
        coloursGrid.templateColumns.add(juce::Grid::TrackInfo(controlsColumnId + "margin", juce::Grid::Px(labelMargin)));
        coloursGrid.templateColumns.add(juce::Grid::TrackInfo(controlsColumnId + "1", juce::Grid::Px(colourControlsWidth / 2)));
        coloursGrid.templateColumns.add(juce::Grid::TrackInfo(controlsColumnId + "1", juce::Grid::Px(colourControlsWidth / 2)));

        coloursGrid.autoRows = juce::Grid::TrackInfo(juce::Grid::Px(flexArea.getHeight()));

        juce::GridItem colorTemplateItem;
        colorTemplateItem.margin.left = surviveRuleLabelWidth;
        colorTemplateItem.margin.bottom = margin * 0.3f;
        colorTemplateItem.alignSelf = juce::GridItem::AlignSelf::stretch;

        juce::GridItem aliveColourItem(colorTemplateItem);
        aliveColourItem.associatedComponent = aliveColourSelector.get();
        aliveColourItem.width = colourControlsWidth = 0.5f;

        juce::GridItem deadColourItem(colorTemplateItem);
        deadColourItem.associatedComponent = deadColourSelector.get();
        deadColourItem.width = colourControlsWidth = 0.5f;

        coloursGrid.items.add(aliveColourItem.withColumn({ 2 }));
        coloursGrid.items.add(deadColourItem.withColumn({ 4 }));
        coloursGrid.performLayout(flexArea.withLeft(leftControlsArea.getRight()).withWidth(flexArea.getWidth() - modeControlsMaxWidth));
    }
}

void HexagonAutomata::Component::colourChangedCallback(ColourSelectionBroadcaster* source, juce::Colour newColour)
{
    if (source == aliveColourSelector.get())
        game->setAliveColour(newColour);

    else if (source == deadColourSelector.get())
        game->setDeadColour(newColour);
}

void HexagonAutomata::Component::onGenerationModeChange()
{
}

void HexagonAutomata::Component::onGameModeChange()
{
    gameModeSelector->setSelectedId(static_cast<int>(game->getGameMode()), juce::NotificationType::dontSendNotification);

    bool showClassic = game->getGameMode() == HexagonAutomata::GameMode::Classic;
    bool showSequencer = game->getGameMode() == HexagonAutomata::GameMode::Sequencer;

    aliveColourSelector->setVisible(showClassic);
    deadColourSelector->setVisible(showClassic);
}

void HexagonAutomata::Component::onRulesChange()
{
    switch (game->getRulesMode())
    {
    default:
    case HexagonAutomata::RulesMode::BornSurvive:
    {
        auto bornInputCleaned = bornRuleInput->getText().trimCharactersAtStart(", \t").trimCharactersAtEnd(", \t");
        auto surviveInputCleaned = surviveRuleInput->getText().trimCharactersAtStart(", \t").trimCharactersAtEnd(", \t");

        if (bornInputCleaned != game->getBornRules() || surviveInputCleaned != game->getSurviveRules())
            game->setBornSurviveRules(bornRuleInput->getText(), surviveRuleInput->getText());
    }
    break;

    case HexagonAutomata::RulesMode::SpiralRule:
        break;
    }
}

void HexagonAutomata::Component::updateSpeedSlider()
{
    genSpeedSlider->setValue(game->getGenerationBpm(), juce::NotificationType::dontSendNotification);
}

void HexagonAutomata::Component::speedSliderCallback()
{
    int bpm = genSpeedSlider->getValue();
    game->setGenerationBpm(bpm);
}


void HexagonAutomata::Component::handleStatePropertyChange(juce::ValueTree stateIn, const juce::Identifier &property)
{
    if (property == HexagonAutomata::ID::GameMode)
    {
        onGameModeChange();
    }
    else if (property == HexagonAutomata::ID::GenerationMode)
    {
        onGenerationModeChange();
    }
    else if (property == HexagonAutomata::ID::GenerationMs)
    {
        updateSpeedSlider();
    }
    else if (property == HexagonAutomata::ID::RulesMode)
    {
        onRulesChange();
    }
    else if (property == HexagonAutomata::ID::AliveColour)
    {
        // render->setColour(aliveColour, deadColour);
    }
    else if (property == HexagonAutomata::ID::DeadColour)
    {
        // render->setColour(aliveColour, deadColour);
    }
    else if (property == HexagonAutomata::ID::BornRule)
    {
        onRulesChange();
    }
    else if (property == HexagonAutomata::ID::SurviveRule)
    {
        onRulesChange();
    }
    else if (property == HexagonAutomata::ID::NeighborShape)
    {
        
    }
}
