/*
  ==============================================================================

    hexagon_automata_launcher.cpp
    Created: 27 Aug 2023 9:27:26pm
    Author:  Vincenzo

  ==============================================================================
*/

#include "./hexagon_automata_launcher.h"
#include "./hexagon_automata.h"

//==============================================================================
HexagonAutomata::Component::Component(HexagonAutomata::Game* gameIn)
    : LumatoneSandboxGameComponent(gameIn)
    , game(gameIn)
{
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

    // numSeedsLabel = std::make_unique<juce::Label>("Num random seeds to add", "# Seeds:");
    // numSeedsLabel->setJustificationType(juce::Justification::centredLeft);
    // numSeedsLabel->attachToComponent(addSeedButton.get(), true);
    // addAndMakeVisible(*numSeedsLabel);

    genSpeedSlider = std::make_unique<juce::Slider>(juce::Slider::SliderStyle::LinearHorizontal, juce::Slider::TextEntryBoxPosition::TextBoxLeft);
    genSpeedSlider->setRange(1, 120, 1);
    genSpeedSlider->setSkewFactor(0.5, false);
    genSpeedSlider->setValue(25, juce::NotificationType::dontSendNotification);
    genSpeedSlider->onValueChange = [&]
    {
        game->setTicksPerAsyncGeneration(genSpeedSlider->getValue());
    };
    addAndMakeVisible(*genSpeedSlider);

    genSpeedLabel = std::make_unique<juce::Label>("Generation Speed Label", "Frames per Generation:");
    genSpeedLabel->setJustificationType(juce::Justification::centredLeft);
    genSpeedLabel->attachToComponent(genSpeedSlider.get(), false);
    addAndMakeVisible(*genSpeedLabel);

    bornRuleInput = std::make_unique<juce::TextEditor>("BornRuleText");
    bornRuleInput->setInputFilter(new juce::TextEditor::LengthAndCharacterRestriction(0, "0123456789, "), true);
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
    
    suviveRuleInput = std::make_unique<juce::TextEditor>("SurviveRuleText");
    suviveRuleInput->setInputFilter(new juce::TextEditor::LengthAndCharacterRestriction(0, "0123456789, "), true);
    suviveRuleInput->setText("3,4");
    suviveRuleInput->onTextChange = [&]
    {
        onRulesChange();
    };
    addAndMakeVisible(*suviveRuleInput);

    surviveRuleLabel = std::make_unique<juce::Label>("Cell Survive Rule Label", "Survive Rules:");
    surviveRuleLabel->setJustificationType(juce::Justification::centredLeft);
    surviveRuleLabel->attachToComponent(suviveRuleInput.get(), true);
    addAndMakeVisible(*surviveRuleLabel);

    distanceSlider = std::make_unique<juce::Slider>(juce::Slider::SliderStyle::IncDecButtons, juce::Slider::TextBoxLeft);
    distanceSlider->setRange(0, 7, 1);
    distanceSlider->setValue(1, juce::NotificationType::dontSendNotification);
    distanceSlider->onValueChange = [&]
    {
        game->setNeighborDistance(distanceSlider->getValue());
    };
    addAndMakeVisible(*distanceSlider);

    distanceLabel = std::make_unique<juce::Label>("Neighbor Distance Label", "Neighbor Distance:");
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
}

HexagonAutomata::Component::~Component()
{ 
    game = nullptr;

    genSpeedSlider = nullptr;
    addSeedButton = nullptr;
}

void HexagonAutomata::Component::resized()
{
    LumatoneSandboxGameComponent::resized();

    juce::Array<HexagonAutomata::Component::Parameter> layoutParams = 
    {
        HexagonAutomata::Component::Parameter::FramesPerGeneration, 
        HexagonAutomata::Component::Parameter::AddSeed,
        HexagonAutomata::Component::Parameter::BornRule,
        HexagonAutomata::Component::Parameter::SurviveRule,
        HexagonAutomata::Component::Parameter::NeighborDistance,
        HexagonAutomata::Component::Parameter::AliveColour,
        HexagonAutomata::Component::Parameter::DeadColour
    };

    juce::FlexBox box;
    box.flexDirection = juce::FlexBox::Direction::row;

    int margin = controlsArea.proportionOfHeight(marginScalar);
    int labelMargin = margin / 2;
    int controlLeft = controlsArea.getX();

    flexArea = controlsArea.withTrimmedTop(margin * 2);

    float controlHeightScalar = 0.15f;
    int controlHeight = juce::roundToInt(flexArea.proportionOfHeight(controlHeightScalar));

    float buttonHeightScalar = 0.18f;
    int buttonHeight = juce::roundToInt(flexArea.proportionOfHeight(buttonHeightScalar));

    auto labelFont = getLookAndFeel().getLabelFont(*genSpeedLabel).withHeight(controlHeight);

    float genSpeedLabelWidth = labelFont.getStringWidthFloat(genSpeedLabel->getText() );
    // float numSeedsLabelWidth = labelFont.getStringWidthFloat(numSeedsLabel->getText() );
    float bornRuleLabelWidth = labelFont.getStringWidthFloat(bornRuleLabel->getText() );
    float surviveRuleLabelWidth = labelFont.getStringWidthFloat(surviveRuleLabel->getText() );
    float distanceLabelWidth = labelFont.getStringWidthFloat(distanceLabel->getText());
    
    genSpeedLabel->setSize(genSpeedLabelWidth, controlHeight);
    // numSeedsLabel->setSize(numSeedsLabelWidth, buttonHeight);
    bornRuleLabel->setSize(bornRuleLabelWidth, buttonHeight);
    surviveRuleLabel->setSize(surviveRuleLabelWidth, buttonHeight);
    distanceLabel->setSize(distanceLabelWidth, controlHeight);

    auto buttonFont = getLookAndFeel().getTextButtonFont(*addSeedButton, buttonHeight);
    int addLength = juce::roundToInt(buttonFont.getStringWidthFloat(addSeedButton->getButtonText()) * 1.5f);

    int controlsWidth = flexArea.proportionOfWidth(0.4f) - margin;
    auto textLength = buttonFont.getStringWidth("1, 2, 3, 4");

    juce::FlexBox controlsBox;
    controlsBox.flexDirection = juce::FlexBox::Direction::column;
    
    juce::FlexBox seedsBox;
    
    juce::Grid grid;
    
    juce::Array<juce::FlexItem> controlItems;

    juce::Grid controlsGrid;
    juce::Grid seedsGrid;

    for (int i = 0; i < layoutParams.size(); i++)
    {
        auto param = layoutParams[i];
        // auto item = juce::FlexItem(controlsWidth, controlHeight);

        juce::FlexItem item;
        juce::GridItem gItem;

        item.margin.left = margin * 3;
        // item.margin.top = margin;
 
        gItem.width = controlsWidth;
        gItem.height = controlHeight;
        gItem.margin.left = surviveRuleLabelWidth;
        gItem.margin.bottom = margin * 0.3f;

        switch (param)
        {
        case HexagonAutomata::Component::Parameter::FramesPerGeneration:
            // item.margin.left = 0;
            // item.associatedComponent = genSpeedSlider.get();
            // item.maxWidth = controlsWidth;

            gItem.margin.left = 0;
            gItem.associatedComponent = genSpeedSlider.get();
            gItem.maxWidth = controlsWidth;
            break;
        case HexagonAutomata::Component::Parameter::AddSeed:
            // item.margin.left = margin;
            // item.associatedFlexBox = &seedsBox;
            // item.associatedFlexBox->items.add(juce::FlexItem(*addSeedButton).withWidth(addLength).withHeight(buttonHeight));
            // item.associatedFlexBox->items.add(juce::FlexItem(*numSeedsSlider).withWidth(addLength).withHeight(buttonHeight));
            // item.maxHeight = buttonHeight;

            gItem.margin.left = 0;
            gItem.height = buttonHeight;
            gItem.width = addLength;
            gItem.associatedComponent = addSeedButton.get();
            break;
        case HexagonAutomata::Component::Parameter::BornRule:
            // item.margin.left = bornRuleLabelWidth;
            // item.associatedComponent = bornRuleInput.get();
            // item.maxWidth = textLength;

            gItem.associatedComponent = bornRuleInput.get();
            gItem.width = textLength;
            gItem.maxWidth = textLength * 2;
            break;
        case HexagonAutomata::Component::Parameter::SurviveRule:
            // item.margin.left = surviveRuleLabelWidth;
            // item.associatedComponent = suviveRuleInput.get();
            // item.maxWidth = textLength;

            gItem.associatedComponent = suviveRuleInput.get();
            gItem.width = textLength;
            gItem.maxWidth = textLength * 2;
            break;
        case HexagonAutomata::Component::Parameter::NeighborDistance:
            // item.margin.left = distanceLabelWidth;
            // item.associatedComponent = distanceSlider.get();
            // item.maxWidth = textLength;

            gItem.associatedComponent = distanceSlider.get();
            gItem.width = textLength * 2;
            break;
        }

        controlsBox.items.add(item.withWidth(controlsWidth).withHeight(controlHeight));
        controlsGrid.items.add(gItem);
    }


    // controlsBox.alignContent = juce::FlexBox::AlignContent::flexStart;
    // controlsBox.items.add(juce::FlexItem(*genSpeedSlider).withWidth(controlsArea.getWidth() - genSpeedLabel->getRight() - margin).withHeight(controlHeight));
    

    // juce::FlexBox seedsBox(juce::FlexBox::JustifyContent::flexStart);
    // // seedsBox.flexDirection = juce::FlexBox::Direction::column;
    // seedsBox.items.add(juce::FlexItem(*addSeedButton).withWidth(addLength).withHeight(buttonHeight));
    // seedsBox.items.add(juce::FlexItem(*numSeedsSlider).withWidth(addLength * 2).withHeight(buttonHeight));
    // controlsBox.items.add(juce::FlexItem(seedsBox));

    // controlsBox.items.add(juce::FlexItem(*bornRuleInput).withWidth(textLength).withHeight(controlHeight));
    // controlsBox.items.add(juce::FlexItem(*suviveRuleInput).withWidth(textLength).withHeight(controlHeight));
    // controlsBox.items.add(juce::FlexItem(*distanceSlider).withWidth(addLength * 2).withHeight(controlHeight));

    // auto controlBoxItem = juce::FlexItem(controlsWidth, flexArea.getHeight(), controlsBox);
    // controlBoxItem.margin.top = margin;
    // box.items.add(controlBoxItem);

    // int colourSelectorsWidth = controlsArea.getWidth() - numSeedsSlider->getRight() - margin * 2;
    int colourSelectorsWidth = flexArea.getWidth() - controlsWidth - margin;
    int selectorWidth = colourSelectorsWidth / 2;

    // juce::FlexBox colourBoxes(juce::FlexBox::JustifyContent::spaceAround);
    // colourBoxes.items.add(juce::FlexItem(*aliveColourSelector).withWidth(selectorWidth).withAlignSelf(juce::FlexItem::AlignSelf::stretch));
    // colourBoxes.items.add(juce::FlexItem(*deadColourSelector).withWidth(selectorWidth).withAlignSelf(juce::FlexItem::AlignSelf::stretch));
    // box.items.add(juce::FlexItem(colourSelectorsWidth, flexArea.getHeight(), colourBoxes));

    // box.performLayout(flexArea);

    auto leftControlsArea = flexArea.withWidth(controlsWidth);
    controlsGrid.performLayout(leftControlsArea);
    numSeedsSlider->setBounds(addSeedButton->getRight() + margin, addSeedButton->getY(), addLength * 2, buttonHeight);
    numSeedsSlider->setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxLeft, false, controlsWidth * 0.3f, buttonHeight);

    distanceSlider->setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxLeft, false, distanceSlider->getWidth() * 0.4f, distanceSlider->getHeight());


    juce::Grid coloursGrid;
    coloursGrid.items.add(juce::GridItem(aliveColourSelector.get()).withWidth(selectorWidth).withHeight(flexArea.getHeight()).withAlignSelf(juce::GridItem::AlignSelf::stretch));
    coloursGrid.items.add(juce::GridItem(deadColourSelector.get()).withWidth(selectorWidth).withHeight(flexArea.getHeight()).withAlignSelf(juce::GridItem::AlignSelf::stretch));
    coloursGrid.autoFlow = juce::Grid::AutoFlow::column;

    coloursGrid.performLayout(flexArea.withLeft(leftControlsArea.getRight()).withWidth(flexArea.getWidth() - controlsWidth));
    
    // int speedSliderX = margin;
    // genSpeedLabel->setBounds(speedSliderX, controlsArea.getY(), labelFont.getStringWidth(genSpeedLabel->getText() + "_"), controlHeight);
    // genSpeedSlider->setBounds(genSpeedLabel->getRight() + labelMargin, controlsArea.getY(), controlsArea.getWidth() - genSpeedLabel->getRight() - margin, controlHeight);

    // addSeedButton->setBounds(controlsArea.getX(), genSpeedSlider->getBottom() + margin, addLength, buttonHeight);

    // bornRuleLabel->setBounds(controlLeft, numSeedsSlider->getBottom() + margin, labelFont.getStringWidth(bornRuleLabel->getText() + "_"), controlHeight);
    // bornRuleInput->setBounds(bornRuleLabel->getRight() + labelMargin, numSeedsSlider->getBottom() + margin, textLength, controlHeight);

    // surviveRuleLabel->setBounds(controlLeft /*bornRuleInput->getRight() + margin*/, bornRuleLabel->getBottom() + margin, labelFont.getStringWidth(surviveRuleLabel->getText()), controlHeight);
    // suviveRuleInput->setBounds(surviveRuleLabel->getRight() + labelMargin, surviveRuleLabel->getY(), textLength, controlHeight);

    // distanceLabel->setBounds(controlLeft, suviveRuleInput->getBottom() + margin, labelFont.getStringWidth(distanceLabel->getText() + "_"), controlHeight);
    // distanceSlider->setBounds(distanceLabel->getRight() + labelMargin, distanceLabel->getY(), controlsArea.getWidth() * 0.15f, controlHeight);



    // aliveColourLabel->setBounds(numSeedsSlider->getRight() + margin, addSeedButton->getY() + margin, selectorWidth, controlHeight);
    // deadColourLabel->setBounds(aliveColourSelector->getRight() + margin, addSeedButton->getY() + margin, selectorWidth, controlHeight);


    // aliveColourSelector->setBounds(numSeedsSlider->getRight() + margin, aliveColourLabel->getY() + margin, selectorWidth, selectorHeight);
    // deadColourSelector->setBounds(aliveColourSelector->getRight() + margin, deadColourLabel->getY() + margin,  selectorWidth, selectorHeight);
}

void HexagonAutomata::Component::colourChangedCallback(ColourSelectionBroadcaster* source, juce::Colour newColour)
{
    if (source == aliveColourSelector.get())
        game->setAliveColour(newColour);

    else if (source == deadColourSelector.get())
        game->setDeadColour(newColour);
}

void HexagonAutomata::Component::onRulesChange()
{
    game->setBornSurviveRules(bornRuleInput->getText(), suviveRuleInput->getText());
}
