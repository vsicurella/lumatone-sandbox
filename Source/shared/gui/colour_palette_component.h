/*
  ==============================================================================

    colour_palette_component.h
    Created: 18 Dec 2020 1:48:01am
    Author:  Vincenzo

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "../lumatone_editor_library/palettes/hex_group_palette.h"
#include "../lumatone_editor_library/palettes/colour_selection_group.h"

#include "../lumatone_editor_library/palettes/colour_palette_file.h"

// #include "LumatoneEditorStyleCommon.h"

//==============================================================================
/*
* Inherits from PolygonPalette for Lumatone Editor specific functionality
*/
class ColourPaletteComponent  : public TenHexagonPalette
                              , public ColourSelectionBroadcaster
{
public:
    ColourPaletteComponent(juce::String nameIn);
    ColourPaletteComponent(LumatoneEditorColourPalette paletteIn);
    ColourPaletteComponent(const ColourPaletteComponent& paletteToCopy);
    ~ColourPaletteComponent() override;

    //==========================================================================
    // PolygonPalette overrides

    void setSelectedSwatchNumber(int swatchIndex) override;

    // Add disabled/new palette functionality
    void setColourPalette(juce::Array<juce::Colour> colourPaletteIn) override;

    void setSwatchColour(int swatchNumber, juce::Colour newColour) override;

    //==========================================================================
    // ColourSelectionBroadcaster implementation

    juce::Colour getSelectedColour() override;

    void deselectColour() override;
};

//==============================================================================
/*
    Container object for controlling colour palettes
*/
class PaletteControlGroup
{
public:

    PaletteControlGroup(LumatoneEditorColourPalette newPaletteIn);

    ~PaletteControlGroup() {}

    ColourPaletteComponent* getPaletteComponent() { return &palette; }

    juce::TextButton* getEditButton() { return &editButton; }

    juce::ImageButton* getCloneButton() { return &cloneButton; }

    juce::ImageButton* getTrashButton() { return &trashButton; }

private:

  juce::Image getCloneImage() const;

private:

    ColourPaletteComponent palette;
    juce::TextButton editButton;
    juce::ImageButton cloneButton;
    juce::ImageButton trashButton;
};
