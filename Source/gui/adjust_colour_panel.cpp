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
    juce::Colour border = juce::Colours::black;
    if (isMouseOver())
        border = border.contrasting(border.brighter(), colour);

    g.fillAll(colour);

    g.setColour(border);
    g.drawRect(getLocalBounds(), 2);
}

void AdjustColourPanel::Box::resized()
{

}

AdjustColourPanel::AdjustColourPanel(LumatoneController* controllerIn)
    : controller(controllerIn)
{
    controller->addEditorListener(this);
    reconfigureColours();
}

AdjustColourPanel::~AdjustColourPanel()
{
    controller->removeEditorListener(this);
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

void AdjustColourPanel::mouseMove(const juce::MouseEvent& e)
{
    
}

void AdjustColourPanel::mouseDown(const juce::MouseEvent& e)
{
    
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
            colourBoxes.add(new AdjustColourPanel::Box(c));
            addAndMakeVisible(colourBoxes.getUnchecked(i));
        }
    }

    if (numToDelete > 0)
        colourBoxes.removeLast(numToDelete);

    if (doResize)
        resized();
    else 
        repaint();
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
