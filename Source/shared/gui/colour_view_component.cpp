/*
  ==============================================================================

  This is an automatically generated GUI class created by the Projucer!

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Created with Projucer version: 6.0.4

  ------------------------------------------------------------------------------

  The Projucer is part of the JUCE library.
  Copyright (c) 2020 - Raw Material Software Limited.

  ==============================================================================
*/

//[Headers] You can add your own extra header files here...
#include "../lumatone_editor_library/graphics/view_constants.h"
//[/Headers]

#include "colour_view_component.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...

/*
==============================================================================
ColourComboBox class
==============================================================================
*/

// ColourComboBox::ColourComboBox(juce::StringRef componentName) : juce::ComboBox(componentName)
// {
// }

// void ColourComboBox::setTextFieldToColourAsObject(juce::Colour newColourAsObject, juce::NotificationType notification)
// {
// 	setText(newColourAsObject.toDisplayString(false));

// 	// getLookAndFeel().setColour(juce::ComboBox::arrowColourId, newColourAsObject.contrasting(1.0)); Doesn't work XXX

// 	// ToDo Add to box
// }

// juce::String ColourComboBox::getColourAsStringFromText(colourComboboxOptions boxOptions)
// {
// 	juce::String colourAsString = getText();

// 	// ToDo validation of colour value

// 	if (boxOptions == colourComboboxOptions::AddColourToComboBox && !colourAsString.isEmpty())
// 		addColourToBox(colourAsString);

// 	return colourAsString;
// }

// int ColourComboBox::getColourAsNumberFromText(colourComboboxOptions boxOptions)
// {
// 	juce::String colourString = getColourAsStringFromText(boxOptions);

// 	// ToDo validation of colour value

// 	int colourAsNumber = colourString.getHexValue32();

// 	return colourAsNumber;
// }

// // Add colour to combo box
// void ColourComboBox::addColourToBox(juce::String newColourAsString)
// {
// 	int pos;
// 	for (pos = 0; pos < getNumItems(); pos++)
// 	{
// 		if (getItemText(pos) == newColourAsString)
// 			break;
// 	}

// 	if (pos >= getNumItems())
// 	{
// 		// juce::Colour is not in list yet - add it
// 		addItem(newColourAsString, pos + 1);
// 	}
// }

// juce::Colour ColourComboBox::getColourAsObjectFromText(colourComboboxOptions boxOptions)
// {
// 	int colourAsNumber = getColourAsNumberFromText(boxOptions);
// 	return juce::Colour(colourAsNumber);
// }


/*
==============================================================================
juce::ColourComboLookAndFeel class
==============================================================================
*/

// void ColourComboLookAndFeel::drawPopupMenuItem (juce::Graphics g, const juce::Rectangle<int>& area,
//                                         const bool isSeparator, const bool isActive,
//                                         const bool isHighlighted, const bool isTicked,
//                                         const bool hasSubMenu, const juce::String& text,
//                                         const juce::String& shortcutKeyText,
//                                         const juce::Drawable* icon, const juce::Colour* const textColourToUse)
// {
//     if (isSeparator)
//     {
//         juce::LookAndFeel_V4::drawPopupMenuItem(
//             g, area, isSeparator, isActive, isHighlighted, isTicked, hasSubMenu, text, shortcutKeyText, icon, textColourToUse);
//     }
//     else
//     {
//         //  auto bgColour = findColour(TerpstraKeyEdit::backgroundColourId).overlaidWith(
//         //     juce::Colour(text.getHexValue32())
//         //         .withAlpha(isHighlighted && isActive ? TERPSTRASINGLEKEYCOLOURALPHA : TERPSTRASINGLEKEYCOLOURUNSELECTEDMINIALPHA));
//         auto bgColour = juce::Colours::slategrey;
//         auto textColour = bgColour.contrasting(1.0);

//         auto r  = area.reduced (1);

//         g.setColour (bgColour);
//         g.fillRect (r);

//         g.setColour (textColour);

//         r.reduce (juce::jmin(5, area.getWidth() / 20), 0);

//         auto font = getPopupMenuFont();

//         auto maxFontHeight = r.getHeight() / 1.3f;

//         if (font.getHeight() > maxFontHeight)
//             font.setHeight (maxFontHeight);

//         g.setFont (font);

//         auto iconArea = r.removeFromLeft (juce::roundToInt (maxFontHeight)).toFloat();

//         if (icon != nullptr)
//         {
//             icon->drawWithin (g, iconArea, juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize, 1.0f);
//             r.removeFromLeft (juce::roundToInt (maxFontHeight * 0.5f));
//         }
//         else if (isTicked)
//         {
//             auto tick = getTickShape (1.0f);
//             g.fillPath (tick, tick.getTransformToScaleToFit (iconArea.reduced (iconArea.getWidth() / 5, 0).toFloat(), true));
//         }

//         if (hasSubMenu)
//         {
//             auto arrowH = 0.6f * getPopupMenuFont().getAscent();

//             auto x = static_cast<float> (r.removeFromRight ((int) arrowH).getX());
//             auto halfH = static_cast<float> (r.getCentreY());

//             juce::Path path;
//             path.startNewSubPath (x, halfH - arrowH * 0.5f);
//             path.lineTo (x + arrowH * 0.6f, halfH);
//             path.lineTo (x, halfH + arrowH * 0.5f);

//             g.strokePath (path, juce::PathStrokeType (2.0f));
//         }

//         r.removeFromRight (3);
//         g.drawFittedText (text, r, juce::Justification::centredLeft, 1);

//         if (shortcutKeyText.isNotEmpty())
//         {
//             auto f2 = font;
//             f2.setHeight (f2.getHeight() * 0.75f);
//             f2.setHorizontalScale (0.95f);
//             g.setFont (f2);

//             g.drawText (shortcutKeyText, r, juce::Justification::centredRight, true);
//         }
//     }
// }

//[/MiscUserDefs]

//==============================================================================
ColourViewComponent::ColourViewComponent (juce::Colour initialColour)
    : Button("ColourViewComponent")
{
    //[Constructor_pre] You can add your own custom stuff here..
    //[/Constructor_pre]

    //[UserPreSize]
    //[/UserPreSize]

    //setSize (120, 32);


    //[Constructor] You can add your own custom stuff here..

    currentColour = initialColour;
    currentColourAsString = currentColour.toString();

    setButtonText("v");
    //[/Constructor]
}

ColourViewComponent::~ColourViewComponent()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}


//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...

void ColourViewComponent::paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    juce::Colour backgroundColour = findColour(juce::TextButton::ColourIds::buttonColourId);
    juce::Colour textColour       = backgroundColour.contrasting();

    if (!isEnabled())
    {
        backgroundColour = backgroundColour.darker().withMultipliedSaturation(0.3f);
        textColour = textColour.overlaidWith(juce::Colours::darkgrey.withAlpha(0.6f));
    }

    if (shouldDrawButtonAsHighlighted)
    {
        backgroundColour = (isMouseButtonDown())
            ? backgroundColour.darker(0.25f)
            : backgroundColour.brighter(0.25f);
    }

    g.setColour(backgroundColour);
    g.fillRoundedRectangle(getLocalBounds().toFloat(), getHeight() / 3.0f);

    g.setFont(juce::Font().withHeight(proportionOfHeight(0.75f)).withHorizontalScale(2.0f));
    g.setColour(textColour);
    g.drawFittedText(getButtonText(), getLocalBounds(), juce::Justification::centred, 1);
}

void ColourViewComponent::setColour(juce::String colourAsString, bool sendChange)
{
	//jassert(colourCombo != nullptr);

    currentColourAsString = colourAsString;
    if (currentColourAsString.length() == 6)
        currentColourAsString = "FF" + colourAsString;

    currentColour = juce::Colour::fromString(currentColourAsString);
    Component::setColour(juce::TextButton::ColourIds::buttonColourId, currentColour);
    repaint();

    // Notify parent that value has changed and can be sent to MIDI controller
    if (sendChange)
        sendChangeMessage();
}

juce::String ColourViewComponent::getColourAsString()
{
    return currentColourAsString;
}

int ColourViewComponent::getColourAsNumber()
{
	return currentColourAsString.getHexValue32();
}

juce::Colour ColourViewComponent::getColourAsObject()
{
    return currentColour;
}

void ColourViewComponent::colourChangedCallback(ColourSelectionBroadcaster* source, juce::Colour newColour)
{
    setColour(newColour.toString());
}


//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Projucer information section --

    This is where the Projucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="ColourViewComponent" componentName=""
                 parentClasses="public Component, public juce::ChangeListener, public juce::ChangeBroadcaster"
                 constructorParams="" variableInitialisers="" snapPixels="8" snapActive="1"
                 snapShown="1" overlayOpacity="0.330" fixedSize="1" initialWidth="120"
                 initialHeight="32">
  <BACKGROUND backgroundColour="ffb8d0de"/>
  <TEXTBUTTON name="btnColourPicker" id="fb8f62a75c5cd9ec" memberName="btnColourPicker"
              virtualName="" explicitFocusOrder="0" pos="88 0 24 24" buttonText=".."
              connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <COMBOBOX name="colourCombo" id="86628debb1bafc04" memberName="colourCombo"
            virtualName="ColourComboBox" explicitFocusOrder="0" pos="0 0 79 24"
            editable="1" layout="33" items="" textWhenNonSelected="" textWhenNoItems="(no choices)"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
