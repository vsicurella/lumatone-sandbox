#include "colour_edit_panel.h"

ColourEditPanel::Box::Box(juce::Colour colourIn)
    : colour(colourIn)
{

}

ColourEditPanel::Box::~Box()
{

}

void ColourEditPanel::Box::setColour(juce::Colour newColour)
{

}

void ColourEditPanel::Box::paint(juce::Graphics& g)
{
    juce::Colour border = juce::Colours::black;
    if (isMouseOver())
        border = border.contrasting(border.brighter(), colour);

    g.fillAll(colour);

    g.setColour(border);
    g.drawRect(getLocalBounds(), 2);
}

void ColourEditPanel::Box::resized()
{

}


ColourEditPanel::ColourEditPanel()
{

}

ColourEditPanel::ColourEditPanel(const juce::Array<juce::Colour>& colourSet)
{
    reconfigureColours(colourSet);
}

ColourEditPanel::~ColourEditPanel()
{
    colourBoxes.clear();
}

void ColourEditPanel::setColours(const juce::Array<juce::Colour>& colourSet)
{
    reconfigureColours(colourSet);
}

void ColourEditPanel::paint(juce::Graphics& g)
{
}

void ColourEditPanel::resized()
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
        if (boxBounds.intersects(controlBounds))
        {
            boxX = controlBounds.getX();
            boxY += boxSize + boxMargin;
            boxBounds.setPosition(boxX, boxY);
        }

        box->setBounds(boxBounds);

        boxX += boxSize + boxMargin;
    }
}

void ColourEditPanel::mouseMove(const juce::MouseEvent& e)
{
    
}
void ColourEditPanel::mouseDown(const juce::MouseEvent& e)
{
    
}

void ColourEditPanel::reconfigureColours(const juce::Array<juce::Colour>& colourSet)
{
    bool doResize = colourSet.size() != colours.size();

    colours = colourSet;

    int boxesToDelete = colours.size() - colourBoxes.size();

    for (int i = 0; i < colours.size(); i++)
    {
        auto c = colours[i];

        if (i < colourBoxes.size())
            colourBoxes.getUnchecked(i)->setColour(c);
        else
            colourBoxes.add(new Box(c));
    }

    if (boxesToDelete > 0)
        colourBoxes.removeLast(boxesToDelete);

    if (doResize)
        resized();
    else 
        repaint();
}