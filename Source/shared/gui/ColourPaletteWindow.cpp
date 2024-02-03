/*
  ==============================================================================

    colour_palette_window.cpp
    Created: 14 Dec 2020 11:32:03pm
    Author:  Vincenzo

  ==============================================================================
*/

#include "ColourPaletteWindow.h"

#include "../lumatone_editor_library/palettes/colour_selection_group.h"
#include "../lumatone_editor_library/palettes/colour_picker_panel.h"

#include "./colour_palette_component.h"
// #include "./palette_edit_panel.h"


//==============================================================================
// ColourPaletteWindow Definitions

ColourPaletteWindow::ColourPaletteWindow(const LumatoneSandboxState& stateIn)
    : LumatoneSandboxState("ColourPaletteWindow", stateIn)
    , LumatoneSandboxState::Controller(static_cast<LumatoneSandboxState&>(*this))
{
    setName("ColourPaletteWindow");

    colourSelectorGroup.reset(new ColourSelectionGroup());

    loadColourPalettesFromFile();

    palettePanel.reset(new ColourPalettesPanel(getColourPalettes(), colourSelectorGroup.get()));
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

    // const int firstTabIndex = getProperty(LumatoneEditorProperty::LastColourWindowTab, "0").getIntValue();
    // colourToolTabs->setCurrentTabIndex(firstTabIndex);
    // colourToolTabs->getTabbedButtonBar().addChangeListener(this);
}

ColourPaletteWindow::~ColourPaletteWindow()
{
    // paletteEditPanel        = nullptr;
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

    // if (paletteEditPanel.get())
    //     paletteEditPanel->setBounds(getLocalBounds());

    palettePanel->rebuildPanel(getColourPalettes(), palettePanelViewport->getMaximumVisibleWidth());
}

void ColourPaletteWindow::startEditingPalette(int paletteIndexIn, int selectedSwatchIndex)
{
    // paletteIndexEditing = paletteIndexIn;
    // paletteEditPanel.reset(new PaletteEditPanel(*this, LumatoneEditorState::getColourPalettes()[paletteIndexIn]));
    // paletteEditPanel->setBounds(getLocalBounds());
    // paletteEditPanel->setLookAndFeel(&getLookAndFeel());
    // addAndMakeVisible(*paletteEditPanel);
    // paletteEditPanel->addChangeListener(this);

    // // Retain selected swatch
    // if (selectedSwatchIndex >= 0)
    //     paletteEditPanel->setSelectedSwatch(selectedSwatchIndex);
}

void ColourPaletteWindow::duplicatePalette(int paletteIndexIn)
{
    auto colourPalettes = getColourPalettes();
    auto copiedPalette = getColourPalettes()[paletteIndexIn].clone();
    
    if (! copiedPalette.saveToFile())
        return; // TODO

    colourPalettes.insert(paletteIndexIn + 1, copiedPalette);
    setColourPalettes(colourPalettes);

    palettePanel->rebuildPanel(colourPalettes);
}

void ColourPaletteWindow::removePalette(int paletteIndexToRemove)
{
    // Remove loaded colour palette
    auto colourPalettes = getColourPalettes();
    auto deletedPalette = colourPalettes[paletteIndexToRemove];
    
    if (!deletedPalette.deleteFile())
        return; // TODO

    colourPalettes.remove(paletteIndexToRemove);

    setColourPalettes(colourPalettes);

    palettePanel->rebuildPanel(colourPalettes);
}

void ColourPaletteWindow::editPaletteRequested(int paletteIndex, int selectedSwatchIndex)
{
    if (paletteIndex >= 0 && paletteIndex < getColourPalettes().size())
    {
        startEditingPalette(paletteIndex, selectedSwatchIndex);
    }
    else
        jassert(true); // Something bad happened!
}

void ColourPaletteWindow::clonePaletteRequested(int paletteIndex)
{
    if (paletteIndex >= 0 && paletteIndex < getColourPalettes().size())
    {
        duplicatePalette(paletteIndex);
    }
    else
        jassert(true); // Something bad happened!
}

void ColourPaletteWindow::deletePaletteRequested(int paletteIndex)
{
    if (paletteIndex >= 0 && paletteIndex < getColourPalettes().size())
    {
        removePalette(paletteIndex);
    }
    else
        jassert(true); // Something bad happened!
}

void ColourPaletteWindow::newPaletteRequested()
{
    paletteEditingIsNew = true;

    auto colourPalettes = getColourPalettes();
    auto newPalette = LumatoneEditorColourPalette();
    newPalette.saveToFile();
    colourPalettes.insert(0, newPalette);
    
    setColourPalettes(colourPalettes);
    startEditingPalette(0, 0);
}

void ColourPaletteWindow::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    // Palette editing finished
    // if (source == paletteEditPanel.get())
    // {
    //     if (paletteEditPanel->wasSaveRequested())
    //     {
    //         if (paletteIndexEditing >= 0 && paletteIndexEditing < getColourPalettes().size())
    //         {
    //             auto colourPalettes = getColourPalettes();
    //             auto palette = colourPalettes.getReference(paletteIndexEditing);
    //             palette.setColours(paletteEditPanel->getCurrentPalette());

    //             juce::String newName = paletteEditPanel->getPaletteName();
    //             palette.setName(newName);

    //             palette.saveToFile();
    //             setColourPalettes(colourPalettes);
    //         }
    //         else
    //             jassert(true); // Something bad happened!


    //         palettePanel->rebuildPanel(getColourPalettes());
    //     }
    //     else if (paletteEditingIsNew)
    //         removePalette(paletteIndexEditing);

    //     paletteIndexEditing = -1;
    //     paletteEditingIsNew = false;
    //     paletteEditPanel = nullptr;
    // }

    //  if (source == &colourToolTabs->getTabbedButtonBar())
    // {
    //     const int newTab = colourToolTabs->getCurrentTabIndex();
    //     getPropertiesFile()->setValue(LumatoneEditorProperty::LastColourWindowTab, juce::String(newTab));
    // }
}
