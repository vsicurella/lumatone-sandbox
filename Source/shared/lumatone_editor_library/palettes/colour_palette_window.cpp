/*
  ==============================================================================

    colour_palette_window.cpp
    Created: 14 Dec 2020 11:32:03pm
    Author:  Vincenzo

  ==============================================================================
*/

#include "colour_palette_window.h"

#include "./colour_palette_component.h"

#include "./palette_selection_panel.h"
#include "./colour_picker_panel.h"
#include "./palette_edit_panel.h"

//==============================================================================
// ColourPaletteWindow Definitions

ColourPaletteWindow::ColourPaletteWindow(juce::Array<LumatoneEditorColourPalette>& colourPalettesIn)
    // : lookAndFeel(TerpstraSysExApplication::getApp().getLookAndFeel()),
      : colourPalettes(colourPalettesIn)
{
    // setLookAndFeel(&lookAndFeel);
    
    setName("ColourPaletteWindow");

    colourSelectorGroup.reset(new ColourSelectionGroup());
    
    palettePanel.reset(new ColourPalettesPanel(colourPalettes, colourSelectorGroup.get()));
    palettePanel->addListener(this);

    palettePanelViewport.reset(new juce::Viewport("PalettePanelViewport"));
    palettePanelViewport->setViewedComponent(palettePanel.get(), false);
    palettePanelViewport->setScrollBarsShown(true, false, true, false);
    palettePanelViewport->getVerticalScrollBar().setColour(juce::ScrollBar::ColourIds::thumbColourId, juce::Colour(0xff2d3135));

    customPickerPanel.reset(new CustomPickerPanel());
    colourSelectorGroup->addSelector(customPickerPanel.get());
    colourSelectorGroup->addColourSelectionListener(customPickerPanel.get());

    colourToolTabs.reset(new juce::TabbedComponent(juce::TabbedButtonBar::Orientation::TabsAtTop));
    colourToolTabs->setName("ColourSelectionToolTabs");
    colourToolTabs->addTab(juce::translate("ColourPalettes"), juce::Colour(), palettePanelViewport.get(), false);
    colourToolTabs->addTab(juce::translate("CustomPicker"), juce::Colour(), customPickerPanel.get(), false);
    colourToolTabs->setColour(juce::TabbedComponent::ColourIds::outlineColourId, juce::Colour());
    // colourToolTabs->getTabbedButtonBar().getProperties().set(LumatoneEditorStyleIDs::fontHeightScalar, 0.9f);
    addAndMakeVisible(*colourToolTabs);
    
    const int firstTabIndex = 0;//; TerpstraSysExApplication::getApp().getPropertiesFile()->getIntValue("LastColourPopupTabIndex");
    colourToolTabs->setCurrentTabIndex(firstTabIndex);
    colourToolTabs->getTabbedButtonBar().addChangeListener(this);
}

ColourPaletteWindow::~ColourPaletteWindow()
{ 
    paletteEditPanel        = nullptr;
    colourToolTabs          = nullptr;
    
    colourSelectorGroup->removeSelector(customPickerPanel.get());
    customPickerPanel       = nullptr;
    
    palettePanelViewport    = nullptr;
    palettePanel            = nullptr;
    colourSelectorGroup     = nullptr;
    
    setLookAndFeel(nullptr);
}

void ColourPaletteWindow::resized()
{
    palettePanelViewport->setScrollBarThickness(proportionOfWidth(viewportScrollbarWidthScalar));
    colourToolTabs->setBounds(getLocalBounds());

    if (paletteEditPanel.get())
        paletteEditPanel->setBounds(getLocalBounds());

    palettePanel->rebuildPanel(colourPalettes, palettePanelViewport->getMaximumVisibleWidth());
}

void ColourPaletteWindow::startEditingPalette(int paletteIndexIn, int selectedSwatchIndex)
{
    paletteIndexEditing = paletteIndexIn;
    paletteEditPanel.reset(new PaletteEditPanel(colourPalettes[paletteIndexIn]));
    paletteEditPanel->setBounds(getLocalBounds());
    paletteEditPanel->setLookAndFeel(&getLookAndFeel());
    addAndMakeVisible(*paletteEditPanel);
    paletteEditPanel->addChangeListener(this);

    // Retain selected swatch
    if (selectedSwatchIndex >= 0)
        paletteEditPanel->setSelectedSwatch(selectedSwatchIndex);
}

void ColourPaletteWindow::duplicatePalette(int paletteIndexIn)
{
    auto copiedPalette = colourPalettes[paletteIndexIn].clone();
    colourPalettes.insert(paletteIndexIn + 1, copiedPalette);
    // TerpstraSysExApplication::getApp().saveColourPalette(copiedPalette);
    palettePanel->rebuildPanel(colourPalettes);
}

void ColourPaletteWindow::removePalette(int paletteIndexToRemove)
{
    // Remove loaded colour palette
    juce::String deletedPalette = colourPalettes[paletteIndexToRemove].getPathToFile();
    colourPalettes.remove(paletteIndexToRemove);

    // TerpstraSysExApplication::getApp().deletePaletteFile(deletedPalette);

    palettePanel->rebuildPanel(colourPalettes);
}

void ColourPaletteWindow::editPaletteRequested(int paletteIndex, int selectedSwatchIndex)
{
    if (paletteIndex >= 0 && paletteIndex < colourPalettes.size())
    {
        startEditingPalette(paletteIndex, selectedSwatchIndex);
    }
    else
        jassert(true); // Something bad happened!   
}

void ColourPaletteWindow::clonePaletteRequested(int paletteIndex)
{
    if (paletteIndex >= 0 && paletteIndex < colourPalettes.size())
    {
        duplicatePalette(paletteIndex);
    }
    else
        jassert(true); // Something bad happened!
}

void ColourPaletteWindow::deletePaletteRequested(int paletteIndex)
{
    if (paletteIndex >= 0 && paletteIndex < colourPalettes.size())
    {
        removePalette(paletteIndex);
    }
    else
        jassert(true); // Something bad happened!
}

void ColourPaletteWindow::newPaletteRequested()
{
    paletteEditingIsNew = true;
    colourPalettes.insert(0, LumatoneEditorColourPalette());
    startEditingPalette(0, 0);
}

void ColourPaletteWindow::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    // Palette editing finished
    if (source == paletteEditPanel.get())
    {
        if (paletteEditPanel->wasSaveRequested())
        {
            if (paletteIndexEditing >= 0 && paletteIndexEditing < colourPalettes.size())
            {
                auto palette = colourPalettes.getReference(paletteIndexEditing);
                palette.setColours(paletteEditPanel->getCurrentPalette());

                juce::String newName = paletteEditPanel->getPaletteName();
                palette.setName(newName);

                // TerpstraSysExApplication::getApp().saveColourPalette(palette);
            }
            else
                jassert(true); // Something bad happened!


            palettePanel->rebuildPanel(colourPalettes);
        }
        else if (paletteEditingIsNew)
            removePalette(paletteIndexEditing);

        paletteIndexEditing = -1;
        paletteEditingIsNew = false;
        paletteEditPanel = nullptr;
    }
    
    else if (source == &colourToolTabs->getTabbedButtonBar())
    {
        const int newTab = colourToolTabs->getCurrentTabIndex();
        // TerpstraSysExApplication::getApp().getPropertiesFile()->setValue("LastColourPopupTabIndex", newTab);
    }
}
