#include "colour_edit_textbox.h"

//====================================================================================================

ColourTextEditor::ColourTextEditor(juce::String componentName, juce::String initialString)
    : TextEditor(componentName)
{
    setMultiLine(false);
    setJustification(juce::Justification::centredLeft);
    setTooltip(juce::translate("ColourHexValueEditorTool"));
    setInputRestrictions(6, "0123456789ABCDEFabcdef");
    addListener(this);

    setText(initialString, juce::NotificationType::dontSendNotification);
    checkInputAndUpdate();
}

juce::String ColourTextEditor::checkInputAndUpdate(bool sendSelectorListenerUpdate)
{
    juce::String text = parseTextToColourString(getText().toLowerCase());
    if (text.length() > 0)
    {
        // only accept RGB
        if (text.length() > 6)
            text = text.substring(text.length() - 6);

        // restrict to lowercase display
        setText(text, juce::NotificationType::dontSendNotification);

        if (sendSelectorListenerUpdate)
        {
            juce::String opaque = "ff" + text;
            lastBroadcastedColour = juce::Colour(opaque.getHexValue32());
            selectorListeners.call(&ColourSelectionListener::colourChangedCallback, this, lastBroadcastedColour);
        }
    }

    return text;
}

void ColourTextEditor::resetToLastUpdated(bool sendSelectorListenerUpdate)
{
    juce::NotificationType notification = (sendSelectorListenerUpdate) 
        ? juce::NotificationType::sendNotification 
        : juce::NotificationType::dontSendNotification;

    setText(lastBroadcastedColour.toDisplayString(true).toLowerCase(), notification);
}

/** Called when the user changes the text in some way. */
void ColourTextEditor::textEditorTextChanged(juce::TextEditor&)
{
    // confirm changes
    checkInputAndUpdate();
}

/** Called when the user presses the return key. */
void ColourTextEditor::textEditorReturnKeyPressed(juce::TextEditor&)
{
    // confirm changes
    checkInputAndUpdate();
}

/** Called when the user presses the escape key. */
void ColourTextEditor::textEditorEscapeKeyPressed(juce::TextEditor&)
{
    // reset changes
    resetToLastUpdated();
}

/** Called when the text editor loses focus. */
void ColourTextEditor::textEditorFocusLost(juce::TextEditor& editor)
{
    // confirm changes
    checkInputAndUpdate();
}

juce::Colour ColourTextEditor::getSelectedColour()
{
    juce::String opaque = "ff" + parseTextToColourString(getText());
    return juce::Colour(opaque.getHexValue32());
}

void ColourTextEditor::colourChangedCallback(ColourSelectionBroadcaster* source, juce::Colour newColour)
{
    // Safeguard
    if (this != source)
        setText(newColour.toString().substring(2), false);
}

juce::Colour ColourTextEditor::getLastUpdatedColour() const
{
    return lastBroadcastedColour;
}

juce::String ColourTextEditor::parseTextToColourString(juce::String textIn)
{
    // Skip odd-numbered lengths and those less than 6 for RGB
    if (textIn.length() % 2 == 1 || textIn.length() != 6)
    {
        return juce::String();
    }

    return textIn;
}
