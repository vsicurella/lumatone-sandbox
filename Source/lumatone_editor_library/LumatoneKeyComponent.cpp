/*
  ==============================================================================

    LumatoneKeyComponent.cpp
    Created: 26 Jul 2023 11:51:13pm
    Author:  Vincenzo

  ==============================================================================
*/

#include "LumatoneKeyComponent.h"

//==============================================================================

LumatoneKeyDisplay::LumatoneKeyDisplay(int newBoardIndex, int newKeyIndex, LumatoneKey keyDataIn)
    : Component("LumatoneKeyDisplay_" + LumatoneKeyCoord::toString(newBoardIndex, newKeyIndex))
{
    uiMode = UiMode::GraphicInteractive;

    boardIndex = newBoardIndex;
    keyIndex = newKeyIndex;

    keyData = keyDataIn;
    
    clearUiState();
}

LumatoneKeyDisplay::~LumatoneKeyDisplay()
{
}

void LumatoneKeyDisplay::setUiMode(LumatoneKeyDisplay::UiMode uiModeIn)
{
    uiMode = uiModeIn;
}

void LumatoneKeyDisplay::paint(juce::Graphics& g)
{
    juce::Colour hexagonColour = getKeyColour();
    // juce::Colour hexagonColour = findColour(LumatoneKeyEdit::backgroundColourId).overlaidWith(getKeyColour());
    // if (hexagonColour.getPerceivedBrightness() >= 0.6f)
    //    hexagonColour = hexagonColour.darker((1.0f - hexagonColour.getPerceivedBrightness()));

    // set color
    juce::Colour c = hexagonColour;

    switch (uiMode)
    {
    default:
        break;

    case UiMode::ShapeInteractive:
    case UiMode::GraphicInteractive:
        if(isSelected)
        {
            c = c.overlaidWith(juce::Colours::slateblue.withAlpha(0.2f));
        }
        if (isNoteOn || isClicked)
        {
            c = hexagonColour.contrasting(0.5f);
        }
        else if (mouseIsOver)
        {
            c = c.contrasting(0.25f);
        }
        break;
    }

    g.setColour(c);

    // draw
    switch (uiMode)
    {
    default:
        break;

    case UiMode::Shape:
    case UiMode::ShapeInteractive:
        break; // TODO

    case UiMode::Graphic:
    case UiMode::GraphicInteractive:
    {
        if (colourGraphic.isValid() && shadowGraphic.isValid())
        {
            // int x = juce::roundToInt((getWidth() - colourGraphic->getWidth()) * 0.5f);
            // int y = juce::roundToInt((getHeight() - colourGraphic->getHeight()) * 0.5f);

            int x = 0;
            int y = 0;

            g.drawImageAt(colourGraphic, x, y, true);
            
            g.beginTransparencyLayer(0.7f);
            // g.drawImageAt(shadowGraphic, x, y);
            g.drawImageAt(shadowGraphic, x, y);
            g.endTransparencyLayer();
        }
    }
    }
}

void LumatoneKeyDisplay::resized()
{
}

void LumatoneKeyDisplay::parentHierarchyChanged()
{
    //if (getParentComponent())
    //{
    //    removeMouseListener(this);
    //}
    //else
    //{
    //    addMouseListener(this, true);
    //}
}

void LumatoneKeyDisplay::mouseEnter(const juce::MouseEvent& e)
{
    mouseIsOver = true;
    repaintIfInteractive();
}

void LumatoneKeyDisplay::mouseExit(const juce::MouseEvent& e)
{
    mouseIsOver = false;
    repaintIfInteractive();
}

void LumatoneKeyDisplay::mouseDown(const juce::MouseEvent& e)
{
    //DBG("KEY MOUSE DOWN");
    
    mouseIsOver = true;

    if (e.mods.isRightButtonDown())
    {
        // Right mouse click: popup menu
        //juce::PopupMenu menu;
        //TerpstraSysExApplication::getApp().getMainMenu()->createEditMenu(menu);
        //menu.showMenuAsync(PopupMenu::Options());
    }

    // TODO integrate interaction through LumatoneController

    //else
    //{
    //    // NoteOn MIDI message
    //    auto keyData = getKeyData();
    //    if (keyData != nullptr && keyData->channelNumber > 0)
    //    {
    //        if (keyData->keyType == LumatoneKeyType::velocityInterval)
    //        {
    //            // Send "note on" event
    //            //TerpstraSysExApplication::getApp().getMidiDriver().sendNoteOnMessage(keyData->noteNumber, keyData->channelNumber, 60);
    //        }
    //        // ToDo if keyType is "continuous controller": send controller event?
    //    }
    //}
}

void LumatoneKeyDisplay::mouseUp(const juce::MouseEvent& e)
{
    //DBG("KEY MOUSE UP");
    if (!isMouseOver() && mouseIsOver)
    {
        mouseIsOver = false;
    }

    repaintIfInteractive();
}

//void LumatoneKeyDisplay::handleMidiMessage(const juce::MidiMessage& msg)
//{
//    auto keyData = getKeyData();
//    if (keyData != nullptr && msg.getChannel() == keyData->channelNumber && msg.getNoteNumber() == keyData->noteNumber)
//    {
//        isMouseOver = msg.isNoteOn();
//        repaint();
//    }
//}

void LumatoneKeyDisplay::mouseMove(const juce::MouseEvent& e)
{
    if (!mouseIsOver)
    {
        mouseIsOver = true;
        repaintIfInteractive();
    }
}

void LumatoneKeyDisplay::mouseDrag(const juce::MouseEvent& e)
{
    //DBG("KEY DRAG on " + getCoord().toString() + ": " + e.getPosition().toString
    //mouseDragInternal(e);
}

void LumatoneKeyDisplay::startDrag()
{
    if (!isClicked)
    {
        isClicked = true;
        noteOn();
        repaintIfInteractive();
    }
}

void LumatoneKeyDisplay::endDrag()
{
    if (isClicked)
    {
        isClicked = false;
        
        if (noteOffMode != NoteOffModifier::Sustain)
        {
            noteOff();
            repaintIfInteractive();
        }

    }
}

const LumatoneKey* LumatoneKeyDisplay::getKeyData() const
{
    return &keyData;
}

juce::Colour LumatoneKeyDisplay::getKeyColour() const
{
    if (keyColour != juce::Colour())
        return keyColour;

    auto config = getKeyData();
    if (config != nullptr)
        return config->colour;
    else
        // return findColour(LumatoneKeyEdit::backgroundColourId);
        return juce::Colours::slategrey;
}

void LumatoneKeyDisplay::setLumatoneKey(const LumatoneKey& lumatoneKey, int boardIdx, int keyIdx)
{
    keyData = lumatoneKey;
    boardIndex = boardIdx;
    keyIndex = keyIdx;
    // repaint();
}

void LumatoneKeyDisplay::setDisplayColour(const juce::Colour& colour)
{
    keyColour = colour;
    // redrawRender();
}

void LumatoneKeyDisplay::setKeyGraphics(juce::Image& colourGraphicIn, juce::Image& shadowGraphicIn)
{
    colourGraphic = colourGraphicIn;
    shadowGraphic = shadowGraphicIn;
}

void LumatoneKeyDisplay::setSelected(bool selected)
{
    isSelected = selected;
    repaintIfInteractive();
}

void LumatoneKeyDisplay::clearUiState()
{
    isSelected = false;
    mouseIsOver = false; // mouseMove doesn't fix this atm
    isNoteOn = false;
    isClicked = false;
    repaintIfInteractive();
}

void LumatoneKeyDisplay::repaintIfInteractive()
{
    switch (uiMode)
    {
    default:
        break;
    case UiMode::ShapeInteractive:
    case UiMode::GraphicInteractive:
        if (mouseIsOver != isMouseOver())
        {
            mouseIsOver = isMouseOver();
        }

        repaint();
        break;
    }
}

void LumatoneKeyDisplay::noteOn()
{
    isNoteOn = true;
    isClicked = true;

    repaintIfInteractive();
}

void LumatoneKeyDisplay::noteOff()
{
    isNoteOn = false;
    isClicked = false;

    repaintIfInteractive();
}
