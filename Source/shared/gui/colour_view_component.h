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

#pragma once

//[Headers]     -- You can add your own extra header files here --
#include "../lumatone_editor_library/palettes/colour_selection_group.h"

/*
==============================================================================
Combo box with colours - unused with new GUI design
==============================================================================
*/

#define ADDCOLOURTOCOMBOBOX true
#define DONTADDCOLOURTOCOMBOBOX false

// class ColourComboBox : public juce::ComboBox
// {
// public:
// 	enum colourComboboxOptions
// 	{
// 		DoNotAddColourToCombobox = 0,
// 		AddColourToComboBox
// 	};

// 	explicit ColourComboBox(juce::StringRef componentName = juce::String());

// 	void setTextFieldToColourAsObject(juce::Colour newColourAsObject,
// 		juce::NotificationType notification = juce::NotificationType::sendNotificationAsync);
// 	juce::String getColourAsStringFromText(colourComboboxOptions boxOptions);
// 	int getColourAsNumberFromText(colourComboboxOptions boxOptions);
// 	juce::Colour getColourAsObjectFromText(colourComboboxOptions boxOptions);

// 	void addColourToBox(juce::String newColourAsString);
// 	void addColourToBox(juce::Colour newColourAsObject) { addColourToBox(newColourAsObject.toDisplayString(false)); }
// 	void addColourToBox(int newColourAsNumber) { addColourToBox(juce::Colour(newColourAsNumber)); }
// };

/*
==============================================================================
Draw each item in ColourComboBox's drop-down list in "its" colour
==============================================================================
// */
// class ColourComboLookAndFeel : public juce::LookAndFeel_V4
// {
// public:
//     void drawPopupMenuItem (juce::Graphics&, const juce::Rectangle<int>& area,
//                             bool isSeparator, bool isActive, bool isHighlighted, bool isTicked, bool hasSubMenu,
//                             const juce::String& text, const juce::String& shortcutKeyText,
//                             const juce::Drawable* icon, const juce::Colour* textColour) override;

// };


//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Projucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class ColourViewComponent  : public juce::Button,
                             public juce::ChangeBroadcaster,
                             public ColourSelectionListener
{
public:
    //==============================================================================
    ColourViewComponent (juce::Colour initialColour = juce::Colour(0xff5c7cf2));
    ~ColourViewComponent() override;

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
	//void changeListenerCallback(juce::ChangeBroadcaster *source) override;

	void setColour(juce::String colourAsString, bool sendChangeMessage = true);
	juce::String getColourAsString();
	int getColourAsNumber();
    juce::Colour getColourAsObject();
	//void addColourToBox(int newColourAsNumber);
    //[/UserMethods]

    //void paint (juce::Graphics g) override;
    //void resized() override;
    //void buttonClicked (juce::Button* buttonThatWasClicked) override;
    //void comboBoxChanged (juce::ComboBox* comboBoxThatHasChanged) override;

    void paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

    //==============================================================================
    // ColourSelectionListener implementation

    void colourChangedCallback(ColourSelectionBroadcaster* source, juce::Colour newColour) override;


private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    //std::unique_ptr<ColourComboLookAndFeel> colourComboLookAndFeel;

    juce::Colour currentColour = juce::Colour(0xffd1d1d1);
    juce::String currentColourAsString = currentColour.toString();
    float brightnessAdjust = 0.0f;

    //[/UserVariables]

    //==============================================================================
    //std::unique_ptr<juce::TextButton> btnColourPicker;
    //std::unique_ptr<ColourComboBox> colourCombo;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ColourViewComponent)
};

//[EndFile] You can add extra defines here...

//[/EndFile]
