/*
  ==============================================================================

    ColourSelectionPanels.h
    Created: 21 Dec 2020 9:48:08pm
    Author:  Vincenzo

  ==============================================================================
*/

#ifndef LUMATONE_PALETTE_EDIT_PANEL_H
#define LUMATONE_PALETTE_EDIT_PANEL_H

#include "./colour_palette_component.h"

//==============================================================================
/*
*   Colour palette edtior panel
*/
class PaletteEditPanel : public juce::Component
                       , public juce::Button::Listener
                       , public juce::Label::Listener
                       , public juce::ChangeBroadcaster
{
public:

    PaletteEditPanel(const LumatoneEditorColourPalette& paletteIn)
        : colourPalette(paletteIn)
    {
        colourPicker.reset(new juce::ColourSelector(
            juce::ColourSelector::ColourSelectorOptions::editableColour
            + juce::ColourSelector::ColourSelectorOptions::showColourAtTop
            + juce::ColourSelector::ColourSelectorOptions::showColourspace
        ));
        addAndMakeVisible(*colourPicker);

        paletteControl.reset(new TenHexagonPalette());
        paletteControl->setColourPalette(*colourPalette.getColours());
        paletteControl->attachColourSelector(colourPicker.get());
        addAndMakeVisible(*paletteControl);

        editPaletteLabel.reset(new juce::Label("EditPaletteLabel", juce::translate("EditPalette")));
        editPaletteLabel->setJustificationType(juce::Justification::centred);
        addAndMakeVisible(*editPaletteLabel);

        paletteNameEditor.reset(new juce::Label("PaletteNameEditor"));
        paletteNameEditor->setJustificationType(juce::Justification::centred);
        paletteNameEditor->setEditable(true);
        paletteNameEditor->addListener(this);
        paletteNameEditor->setColour(juce::Label::ColourIds::backgroundColourId, juce::Colour());
        paletteNameEditor->setText(colourPalette.getName(), juce::dontSendNotification);
        // paletteNameEditor->getProperties().set(LumatoneEditorStyleIDs::labelMaximumLineCount, 3);
        addAndMakeVisible(*paletteNameEditor);
        labelTextChanged(paletteNameEditor.get()); // force update


        saveButton.reset(new juce::TextButton("SaveButton", juce::translate("SavePaletteTip")));
        saveButton->setButtonText("Save");
        saveButton->addListener(this);
        addAndMakeVisible(*saveButton);

        cancelButton.reset(new juce::TextButton("CancelButton", juce::translate("CancelPaletteTip")));
        cancelButton->setButtonText("Cancel");
        cancelButton->addListener(this);
        addAndMakeVisible(*cancelButton);
    }

    ~PaletteEditPanel()
    {
        paletteControl = nullptr;
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::black);
    }

    void resized() override
    {
        float leftWidth = getWidth() * leftColumnWidth;
        float leftCenter = leftWidth * 0.5f;

        auto labelText = editPaletteLabel->getText() + " ";
        int labelWidth = editPaletteLabel->getFont().getStringWidth(labelText);
        // resizeLabelWithHeight(editPaletteLabel.get(), proportionOfHeight(editPaletteHeight));
        editPaletteLabel->setSize(labelWidth, proportionOfHeight(editPaletteHeight));
        editPaletteLabel->setCentrePosition(leftCenter, juce::roundToInt(editPaletteLabel->getHeight() * 0.5f + proportionOfHeight(editPaletteLabelY)));

        float paletteWidth = proportionOfWidth(paletteWidthScalar);
        float paletteHeight = proportionOfHeight(paletteHeightScalar);
        paletteControl->setSize(paletteWidth, paletteHeight);
        paletteControl->setCentrePosition(leftCenter, juce::roundToInt(paletteHeight * 0.5f + proportionOfHeight(paletteY)));

        saveButton->setSize(proportionOfWidth(buttonWidth), proportionOfHeight(buttonHeight));
        saveButton->setCentrePosition(leftCenter, juce::roundToInt(saveButton->getHeight() * 0.5f + proportionOfHeight(buttonY)));
        cancelButton->setBounds(saveButton->getBounds().translated(0, saveButton->getHeight() * 1.125f));

        colourPicker->setSize(proportionOfWidth(pickerWidth), proportionOfHeight(pickerHeight));
        colourPicker->setTopLeftPosition(leftWidth, juce::roundToInt((getHeight() - colourPicker->getHeight()) * 0.5f));

        float leftMargin = colourPicker->getRight() * 0.03f * 0.5f;
        paletteNameEditor->setBounds(juce::Rectangle<int>(
            juce::Point<int>(juce::roundToInt(leftMargin), paletteControl->getBottom()),
            juce::Point<int>(colourPicker->getX() - leftMargin, saveButton->getY())
            ));
        // paletteNameEditor->getProperties().set(LumatoneEditorStyleIDs::fontHeightScalar, editPaletteLabel->getHeight() / (float)paletteNameEditor->getHeight());

        //if (lookAndFeel)
        //{
        //    Font labelFont = (paletteUnnamed)
        //        ? lookAndFeel->getAppFont(LumatoneEditorFont::UniviaProBold)
        //        : lookAndFeel->getAppFont(LumatoneEditorFont::GothamNarrowMedium);

        //    paletteNameEditor->setFont(labelFont);
        //}

    }

    // void lookAndFeelChanged() override
    // {
    //     lookAndFeel = dynamic_cast<LumatoneEditorLookAndFeel*>(&getLookAndFeel());
    //     if (lookAndFeel)
    //     {
    //     }
    // }

    //==============================================================================

    void buttonClicked(juce::Button* btn) override
    {
        if (btn == saveButton.get())
        {
            saveRequested = true;
        }

        sendChangeMessage();
    }

    //==============================================================================

    void labelTextChanged(juce::Label* label) override
    {
        // Italics and font override here is a bit of a hack
        if (label == paletteNameEditor.get())
        {
            bool nameIsEmpty = paletteNameEditor->getText() == "";

            if (!nameIsEmpty)
            {
                paletteUnnamed = false;
                colourPalette.setName(paletteNameEditor->getText());
                // paletteNameEditor->getProperties().remove(LumatoneEditorStyleIDs::fontOverrideTypefaceStyle);
                // paletteNameEditor->getProperties().set(LumatoneEditorStyleIDs::fontOverride, LumatoneEditorFont::GothamNarrowMedium);
            }

            else if (nameIsEmpty && !paletteUnnamed)
            {
                paletteUnnamed = true;
            }

            if (paletteUnnamed)
            {
                paletteNameEditor->setText("unnamed", juce::NotificationType::dontSendNotification);
                // paletteNameEditor->getProperties().set(LumatoneEditorStyleIDs::fontOverrideTypefaceStyle, "Italic");
                // paletteNameEditor->getProperties().remove(LumatoneEditorStyleIDs::fontOverride);
            }

            //resized();
        }
    }

    void editorShown(juce::Label* label, juce::TextEditor& editor) override
    {
        editor.setColour(juce::TextEditor::ColourIds::backgroundColourId, juce::Colours::black);
        editor.setJustification(juce::Justification::centred);
        editor.setMultiLine(true);
    }

    //==============================================================================

    juce::Array<juce::Colour> getCurrentPalette() const
    {
        return paletteControl->getColourPalette();
    }

    void setSelectedSwatch(int selectedSwatchNumber)
    {
        if (selectedSwatchNumber >= 0 && selectedSwatchNumber < paletteControl->getNumberOfSwatches())
            paletteControl->setSelectedSwatchNumber(selectedSwatchNumber);
    }

    juce::String getPaletteName()
    {
        return colourPalette.getName();
    }

    bool wasSaveRequested() const
    {
        return saveRequested;
    }

private:

    LumatoneEditorColourPalette     colourPalette;

    std::unique_ptr<Palette>              paletteControl;
    std::unique_ptr<juce::ColourSelector> colourPicker;

    std::unique_ptr<juce::Label>          editPaletteLabel;
    std::unique_ptr<juce::Label>          paletteNameEditor;
    std::unique_ptr<juce::TextButton>     saveButton;
    std::unique_ptr<juce::TextButton>     cancelButton;

    // LumatoneEditorLookAndFeel*      lookAndFeel = nullptr;

    bool saveRequested = false;

    bool paletteUnnamed = false;

    // Drawing constants

    const float leftColumnWidth     = 0.3677f;

    const float editPaletteLabelY   = 0.1212f;
    const float editPaletteHeight   = 0.0606f;

    const float paletteY            = 0.26f;
    const float paletteWidthScalar  = 0.25f;
    const float paletteHeightScalar = 0.25f;

    const float buttonY             = 0.6739f;
    const float buttonWidth         = 0.2208f;
    const float buttonHeight        = 0.0889f;

    const float pickerWidth         = 0.6f;
    const float pickerHeight        = 0.9f;
};

#endif // LUMATONE_PALETTE_EDIT_PANEL_H
