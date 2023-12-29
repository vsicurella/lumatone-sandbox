
#ifndef LUMATONE_COLOUR_EDIT_TEXTBOX_H
#define LUMATONE_COLOUR_EDIT_TEXTBOX_H

#include <JuceHeader.h>
#include "colour_selection_group.h"

/**

    TextEditor container for parsing colour strings

*/

class ColourTextEditor :
    public juce::TextEditor,
    public ColourSelectionListener,
    public ColourSelectionBroadcaster,
    private juce::TextEditor::Listener
{
public:

    ColourTextEditor(juce::String componentName, juce::String initialString);

    juce::String checkInputAndUpdate(bool sendSelectorListenerUpdate = true);

    void resetToLastUpdated(bool sendSelectorListenerUpdate = true);

    juce::Colour getLastUpdatedColour() const;

    // ColourSelectionBroadcaster Implementation
    juce::Colour getSelectedColour() override;

    void deselectColour() override {};

    // ColourSelectionListener Implementation
    void colourChangedCallback(ColourSelectionBroadcaster* source, juce::Colour newColour) override;

    static juce::String parseTextToColourString(juce::String textIn);

private:
    // TextEditor::Listener Implementation
    /** Called when the user changes the text in some way. */
    void textEditorTextChanged(juce::TextEditor&) override;

    /** Called when the user presses the return key. */
    void textEditorReturnKeyPressed(juce::TextEditor&) override;

    /** Called when the user presses the escape key. */
    void textEditorEscapeKeyPressed(juce::TextEditor&) override;

    /** Called when the text editor loses focus. */
    void textEditorFocusLost(juce::TextEditor&) override;

private:

    juce::Colour lastBroadcastedColour;
};

#endif LUMATONE_COLOUR_EDIT_TEXTBOX_H
