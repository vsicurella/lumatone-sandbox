#include "adjust_colour_panel.h"

#include "./ColourPaletteWindow.h"


AdjustColourPanel::Box::Box(juce::Colour colourIn)
    : colour(colourIn)
{
    
}

AdjustColourPanel::Box::~Box()
{

}

void AdjustColourPanel::Box::setColour(juce::Colour newColour)
{
    colour = newColour;
    repaint();
}

juce::Colour AdjustColourPanel::Box::getColour() const
{
    return colour;
}

void AdjustColourPanel::Box::paint(juce::Graphics& g)
{
    juce::Colour border = (selected) 
        ? juce::Colours::lightgrey 
        : juce::Colours::black;

    if (isMouseOver())
        border = border.contrasting(border.brighter(), colour);

    g.fillAll(colour);

    g.setColour(border);
    g.drawRect(getLocalBounds(), 2);
}

void AdjustColourPanel::Box::resized()
{

}

void  AdjustColourPanel::Box::setSelected(bool selectedIn)
{
    selected = selectedIn;
    repaint();
}


AdjustColourPanel::AdjustColourPanel(const LumatoneSandboxState& stateIn)
    : LumatoneSandboxState("AdjustColourPanel", stateIn)
    , LumatoneSandboxState::Controller(static_cast<LumatoneSandboxState&>(*this))
    , colourAdjuster(*this)
{
    addEditorListener(this);
    addMouseListener(this, true);
    reconfigureColours(false);

    hueSlider = std::make_unique<juce::Slider>(juce::Slider::SliderStyle::LinearHorizontal, juce::Slider::TextEntryBoxPosition::TextBoxLeft);
    hueSlider->setRange(-1.0, 1.0, 0.001);
    hueSlider->onValueChange = [&]() { hueValueCallback(); };
    addAndMakeVisible(hueSlider.get());

    brightnessSlider = std::make_unique<juce::Slider>(juce::Slider::SliderStyle::LinearHorizontal, juce::Slider::TextEntryBoxPosition::TextBoxLeft);
    brightnessSlider->setRange(0.0, 2.0, 0.01);
    brightnessSlider->setValue(1.0);
    brightnessSlider->onValueChange = [&]() { brightnessValueCallback(); };
    addAndMakeVisible(brightnessSlider.get());

    satSlider = std::make_unique<juce::Slider>(juce::Slider::SliderStyle::LinearHorizontal, juce::Slider::TextEntryBoxPosition::TextBoxLeft);
    satSlider->setRange(0.0, 2.0, 0.01);
    satSlider->setValue(1.0);
    satSlider->onValueChange = [&]() { saturationValueCallback(); };
    addAndMakeVisible(satSlider.get());

    kelvinSlider = std::make_unique<juce::Slider>(juce::Slider::SliderStyle::LinearHorizontal, juce::Slider::TextEntryBoxPosition::TextBoxLeft);
    kelvinSlider->setRange(1000, 40000, 1);
    kelvinSlider->setSkewFactor(1.0f / 3.0f);
    kelvinSlider->setValue(6500);
    kelvinSlider->onValueChange = [&]() { adjustWhiteValueCallback(); };
    addAndMakeVisible(kelvinSlider.get());
}

AdjustColourPanel::~AdjustColourPanel()
{
    removeEditorListener(this);
    palettePanel = nullptr;

    brightnessSlider = nullptr;
    hueSlider = nullptr;

    colourBoxes.clear();
}

void AdjustColourPanel::paint(juce::Graphics& g)
{
}

void AdjustColourPanel::resized()
{
    controlBounds = getLocalBounds().reduced(juce::roundToInt(getWidth() * marginX));

    int boxX = controlBounds.getX();
    int boxY = controlBounds.getY();
    int boxMargin = juce::roundToInt(getHeight() * boxMarginX);
    int boxSize = juce::roundToInt(getHeight() * boxSizeX);

    for (int i = 0; i < colourBoxes.size(); i++)
    {
        auto box = colourBoxes.getUnchecked(i);

        juce::Rectangle<int> boxBounds(boxX, boxY, boxSize, boxSize);
        if (boxBounds.getRight() > getLocalBounds().getRight())
        {
            boxX = controlBounds.getX();
            boxY += boxSize + boxMargin;
            boxBounds.setPosition(boxX, boxY);
        }

        box->setBounds(boxBounds);

        boxX += boxSize + boxMargin;
    }

    hueSlider->setBounds(controlBounds.getX(), boxY + boxSize * 2 + boxMargin, getWidth() * 0.5, boxSize);
    brightnessSlider->setBounds(controlBounds.getX(), hueSlider->getY() + boxSize + boxMargin, getWidth() * 0.5, boxSize);
    satSlider->setBounds(controlBounds.getX(), brightnessSlider->getY() + boxSize + boxMargin, getWidth() * 0.5, boxSize);
    kelvinSlider->setBounds(controlBounds.getX(), satSlider->getY() + boxSize + boxMargin, getWidth() * 0.5, boxSize);
}

void AdjustColourPanel::setSelectedBox(Box* box)
{
    box->setSelected(true);
    selectedBox = colourBoxes.indexOf(box);
    keySelection = getMappingData()->getKeysWithColour(box->getColour());
    DBG("select colour");
}

void AdjustColourPanel::deselectBox()
{
    DBG("deselect colour");
    colourBoxes[selectedBox]->setSelected(false);
    selectedBox = -1;
    keySelection.clear();
    reconfigureColours();
}

void AdjustColourPanel::mouseMove(const juce::MouseEvent& e)
{
    
}

void AdjustColourPanel::mouseDown(const juce::MouseEvent& e)
{
    // if (callout.get() != nullptr && e.eventComponent != callout.get() || e.eventComponent->getParentComponent() != callout.get())
    //     callout->dismiss();

    auto box = dynamic_cast<Box*>(e.eventComponent);
    if (box != nullptr)
    {
        setSelectedBox(box);

        palettePanel = std::make_unique<ColourPaletteWindow>(*this);
        palettePanel->setSize(400, 400);
        palettePanel->listenToColourSelection(this);

        callout.reset(new juce::CallOutBox(*palettePanel, box->getBounds(), this));
        callout->setDismissalMouseClicksAreAlwaysConsumed(true);
    }
    else if (e.eventComponent == this)
    {
        if (selectedBox >= 0)
        {
            deselectBox();
        }
    }
}

void AdjustColourPanel::reconfigureColours(bool doResize)
{
    juce::Array<juce::Colour> colourSet = getMappingData()->getLayoutColours();
    
    bool sizeChanged = colourSet.size() != colours.size();
    colours = colourSet;

    int numToDelete = colourBoxes.size() - colours.size();

    for (int i = 0; i < colours.size(); i++)
    {
        auto c = colours[i];
        if (i < colourBoxes.size())
            colourBoxes.getUnchecked(i)->setColour(c);
        else
        {
            auto box = new AdjustColourPanel::Box(c);
            colourBoxes.add(box);
            addAndMakeVisible(box);
        }
    }

    if (numToDelete > 0)
        colourBoxes.removeLast(numToDelete);

    if (doResize)
    {
        if (sizeChanged)
            resized();
        else 
            repaint();
    }
}

void AdjustColourPanel::completeMappingLoaded(const LumatoneLayout& mappingData)
{
    reconfigureColours();
}

void AdjustColourPanel::boardChanged(const LumatoneBoard& boardData)
{
    reconfigureColours();
}

void AdjustColourPanel::keyChanged(int boardIndex, int keyIndex, const LumatoneKey& lumatoneKey)
{
    reconfigureColours();
}

void AdjustColourPanel::colourChangedCallback(ColourSelectionBroadcaster* source, juce::Colour newColour)
{
    jassert(selectedBox >= 0);
    auto box = colourBoxes[selectedBox];

    auto oldColour = box->getColour();
    box->setColour(newColour);

    colourAdjuster.replaceColour(oldColour, newColour);
}

void AdjustColourPanel::hueValueCallback()
{
    colourAdjuster.rotateHue(hueSlider->getValue());
}

void AdjustColourPanel::saturationValueCallback()
{
    colourAdjuster.multiplySaturation(satSlider->getValue());
}

void AdjustColourPanel::brightnessValueCallback()
{
    colourAdjuster.multiplyBrightness(brightnessSlider->getValue());
}

void AdjustColourPanel::adjustWhiteValueCallback()
{
    colourAdjuster.adjustWhiteBalance(kelvinSlider->getValue());
}
