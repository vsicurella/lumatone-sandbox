#include "adjust_colour_panel.h"

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


AdjustColourPanel::AdjustColourPanel(LumatoneController* controllerIn,  LumatonePaletteLibrary* libraryIn)
    : controller(controllerIn)
    , paletteLibrary(libraryIn)
{
    controller->addEditorListener(this);
    addMouseListener(this, true);
    reconfigureColours();
}

AdjustColourPanel::~AdjustColourPanel()
{
    controller->removeEditorListener(this);
    palettePanel = nullptr;
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
    int boxMargin = juce::roundToInt(getWidth() * boxMarginX);
    int boxSize = juce::roundToInt(getWidth() * boxSizeX);

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
}

void AdjustColourPanel::setSelectedBox(Box* box)
{
    box->setSelected(true);
    selectedBox = colourBoxes.indexOf(box);
    keySelection = controller->getMappingData()->getKeysWithColour(box->getColour());
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

    if (e.eventComponent->getParentComponent() == this)
    {
        auto box = (Box*)e.eventComponent;
        setSelectedBox(box);

        palettePanel = std::make_unique<ColourPaletteWindow>(paletteLibrary->getPalettesReference());
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

void AdjustColourPanel::reconfigureColours()
{
    juce::Array<juce::Colour> colourSet = controller->getMappingData()->getLayoutColours();
    
    bool doResize = colourSet.size() != colours.size();

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
        resized();
    else 
        repaint();
}

void AdjustColourPanel::sendColourUpdate(juce::Colour oldColour, juce::Colour newColour)
{
    // auto keyCoords = controller->getMappingData()->getKeysWithColour(oldColour);
    juce::Array<MappedLumatoneKey> keyUpdates;

    for (auto coord : keySelection)
    {
        auto key = controller->getKey(coord);
        keyUpdates.add(MappedLumatoneKey(key->withColour(newColour), coord));
    }

    auto updateAction = new LumatoneEditAction::MultiKeyAssignAction(controller, keyUpdates);
    controller->performUndoableAction(updateAction);
}

void AdjustColourPanel::completeMappingLoaded(LumatoneLayout mappingData)
{
    reconfigureColours();
}

void AdjustColourPanel::boardChanged(LumatoneBoard boardData)
{
    reconfigureColours();
}

void AdjustColourPanel::keyChanged(int boardIndex, int keyIndex, LumatoneKey lumatoneKey)
{
    reconfigureColours();
}

void AdjustColourPanel::colourChangedCallback(ColourSelectionBroadcaster* source, juce::Colour newColour)
{
    jassert(selectedBox >= 0);
    auto box = colourBoxes[selectedBox];

    auto oldColour = box->getColour();
    box->setColour(newColour);

    sendColourUpdate(oldColour, newColour);
}