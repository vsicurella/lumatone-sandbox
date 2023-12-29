/*
  ==============================================================================

    colour_palette_component.cpp
    Created: 18 Dec 2020 1:48:01am
    Author:  Vincenzo

  ==============================================================================
*/

#include <JuceHeader.h>
#include "colour_palette_component.h"
#include "../graphics/lumatone_assets.h"

//==============================================================================
// ColourPaletteComponent Definitions

ColourPaletteComponent::ColourPaletteComponent(juce::String nameIn)
    : TenHexagonPalette()
{
    setName(nameIn);
    setColourPalette(juce::Array<juce::Colour>());
}

ColourPaletteComponent::ColourPaletteComponent(LumatoneEditorColourPalette paletteIn)
    : TenHexagonPalette()
{
    setName(paletteIn.getName());
    setColourPalette(*paletteIn.getColours());
}

ColourPaletteComponent::ColourPaletteComponent(const ColourPaletteComponent& paletteToCopy)
    : TenHexagonPalette()
{
    setName(paletteToCopy.getName());
    setColourPalette(paletteToCopy.getColourPalette());
}

ColourPaletteComponent::~ColourPaletteComponent()
{

}

//==========================================================================

void ColourPaletteComponent::setSelectedSwatchNumber(int swatchIndex)
{
    Palette::setSelectedSwatchNumber(swatchIndex);
    selectorListeners.call(&ColourSelectionListener::colourChangedCallback, this, getSelectedSwatchColour());
}

void ColourPaletteComponent::setColourPalette(juce::Array<juce::Colour> colourPaletteIn)
{
    if (colourPaletteIn.size() == 0)
    {
        setEnabled(false);
    }
    else
    {
        setEnabled(true);
    }

    while (colourPaletteIn.size() < getNumberOfSwatches())
    {
        colourPaletteIn.add(juce::Colour(0xff1b1b1b));
    }

    Palette::setColourPalette(colourPaletteIn);

    if (isEnabled())
    {
        int selectedSwatch = getSelectedSwatchNumber();
        if (selectedSwatch >= 0 && selectedSwatch < getNumberOfSwatches())
                selectorListeners.call(&ColourSelectionListener::colourChangedCallback, this, getSelectedSwatchColour());
    }
    else
    {
        deselectColour();
    }
}

void ColourPaletteComponent::setSwatchColour(int swatchNumber, juce::Colour newColour)
{
    Palette::setSwatchColour(swatchNumber, newColour);

    if (getSelectedSwatchNumber() == swatchNumber)
        selectorListeners.call(&ColourSelectionListener::colourChangedCallback, this, getSelectedSwatchColour());
}

//==========================================================================

juce::Colour ColourPaletteComponent::getSelectedColour()
{
    return Palette::getSelectedSwatchColour();
}

void ColourPaletteComponent::deselectColour()
{
    Palette::setSelectedSwatchNumber(-1);
}

//==============================================================================
// PaletteControlGroup Definitions

PaletteControlGroup::PaletteControlGroup(LumatoneEditorColourPalette newPaletteIn)
    : palette(ColourPaletteComponent(newPaletteIn)),
      editButton("EditButton_" + newPaletteIn.getName()),
      cloneButton("CloneButton_" + newPaletteIn.getName()),
      trashButton("TrashButton_" + newPaletteIn.getName())
{
    editButton.setButtonText("Edit");
    // editButton.getProperties().set(LumatoneEditorStyleIDs::textButtonHyperlinkFlag, 1);
    editButton.setTooltip(juce::translate("EditButtonTip"));

    const juce::Image cloneIcon = getCloneImage();
    cloneButton.setImages(false, true, true,
        cloneIcon, 1.0f, juce::Colour(),
        cloneIcon, 1.0f, juce::Colours::white.withAlpha(0.4f),
        cloneIcon, 1.0f, juce::Colour()
    );
    cloneButton.setTooltip(juce::translate("CloneButtonTip"));

    const juce::Image trashIcon = juce::ImageCache::getFromHashCode((juce::int64)LumatoneAssets::ID::TrashCanIcon);
    trashButton.setImages(false, true, true,
        trashIcon, 1.0f, juce::Colour(),
        trashIcon, 1.0f, juce::Colours::white.withAlpha(0.4f),
        trashIcon, 1.0f, juce::Colour()
    );
    trashButton.setTooltip(juce::translate("TrashButtonTip"));
}

juce::Image PaletteControlGroup::getCloneImage() const
{
    auto cloneImg = juce::ImageCache::getFromHashCode((juce::int64)LumatoneAssets::ID::CloneIcon);
    if (cloneImg.isValid())
        return cloneImg;

    // Create duplicate icon
    float xMargin = 0.1f;
    float yMargin = 0.1f;
    float size = 0.5f;

    auto leftRect = juce::Rectangle<float>(xMargin, 1.0f - yMargin - size, size, size);
    auto rightRect = juce::Rectangle<float>(1.0f - xMargin - size, yMargin, size, size);

    auto cloneIcon = juce::Path();
    cloneIcon.addRoundedRectangle(leftRect, 0.1f, 0.1f);
    cloneIcon.addRoundedRectangle(rightRect, 0.1f, 0.1f);

    // auto cloneIcon = getCloneIconPath();
    cloneIcon.scaleToFit(0, 0, 80, 80, true);

    cloneImg = juce::Image(juce::Image::PixelFormat::ARGB, 100, 100, true);
    juce::Graphics cloneG(cloneImg);
    cloneG.setColour(juce::Colours::white.darker(0.1f));
    cloneG.setOrigin(juce::Point<int>(10, 10));
    auto stroke = juce::PathStrokeType(8.0f, juce::PathStrokeType::JointStyle::curved);
    cloneG.strokePath(cloneIcon, stroke);
    juce::ImageCache::addImageToCache(cloneImg, (juce::int64)LumatoneAssets::ID::CloneIcon);
    return cloneImg;
}