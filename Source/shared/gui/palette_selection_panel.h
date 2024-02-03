/*
  ==============================================================================

    ColourSelectionPanels.h
    Created: 21 Dec 2020 9:48:08pm
    Author:  Vincenzo

  ==============================================================================
*/

#ifndef LUMATONE_PALETTE_SELECTION_PANEL_H
#define LUMATONE_PALETTE_SELECTION_PANEL_H

#include "./colour_palette_component.h"

//==============================================================================
/*
*   Panel that displays the palettes and allows users to select a swatch
*/
class ColourPalettesPanel : public juce::Component
{
public:

    ColourPalettesPanel(juce::Array<LumatoneEditorColourPalette> palettesIn, ColourSelectionGroup* colourSelectionGroup = nullptr)
        : selectionGroup(colourSelectionGroup)
    {
        addMouseListener(this, true);
        setRepaintsOnMouseActivity(true);

        newPalette.reset(new ColourPaletteComponent("EmptyPalette"));
        newPaletteBtn.reset(new juce::TextButton("NewButton", juce::translate("NewPaletteTip")));
        newPaletteBtn->setButtonText(juce::translate("NewPalette"));
        // newPaletteBtn->getProperties().set(LumatoneEditorStyleIDs::textButtonHyperlinkFlag, 1);
        newPaletteBtn->onClick = [&] { listeners.call(&ColourPalettesPanel::Listener::newPaletteRequested); };

    };

    ~ColourPalettesPanel()
    {
        for (auto palette : allPalettes)
            selectionGroup->removeSelector(palette);

        paletteLabels.clear();
        controlGroups.clear();
        newPaletteBtn = nullptr;
        newPalette = nullptr;
    }

    // Used to make this component resize itself depending on how many swatches there are
    //void setViewUnits(int widthIn, int heightIn)
    //{
    //    viewableWidth = widthIn;
    //    viewableHeight = heightIn;

    //    setSize(viewableWidth, getHeightFromNumRows(numRows));
    //}

    int getHeightFromNumRows(int widthIn, int numRowsIn)
    {
        float rowHeight = widthIn * (itemHeightScalar + topMarginScalar + bottomMarginScalar);
        auto height = juce::roundToInt(numRowsIn * rowHeight);
        return height;
    }

    void paint(juce::Graphics& g) override
    {
        //Draws rectangles around items and margins
        //for (auto item : dbgItems)
        //{
        //    g.setColour(Colours::red);
        //    g.drawRect(item.currentBounds);

        //    g.setColour(Colours::green);
        //    g.drawRect(item.currentBounds.getX(), item.currentBounds.getBottom(), item.width, item.margin.bottom, 1.0f);

        //    g.setColour(Colours::yellow);
        //    g.drawRect(item.currentBounds.getX() - item.margin.left - 1, item.currentBounds.getY(), item.margin.left - 1, item.currentBounds.getHeight(), 1.0f);
        //    g.drawRect(item.currentBounds.getRight() + 1, item.currentBounds.getY(), item.margin.right - 1, item.currentBounds.getHeight(), 1.0f);

        //    g.setColour(Colours::violet);
        //    g.drawRect(item.currentBounds.getX(), item.currentBounds.getY() - item.margin.top, item.width, item.margin.top);
        //}
    };

    void resized() override
    {
        juce::Rectangle<int> viewportBounds(getWidth(), viewableHeight);
        juce::FlexBox flexBox(juce::FlexBox::Direction::row, juce::FlexBox::Wrap::wrap, juce::FlexBox::AlignContent::flexStart, juce::FlexBox::AlignItems::center, juce::FlexBox::JustifyContent::flexStart);

        float itemWidth         = viewportBounds.proportionOfWidth(itemWidthScalar);
        float itemHeight        = viewportBounds.proportionOfWidth(itemHeightScalar);
        float topMargin         = viewportBounds.proportionOfWidth(topMarginScalar);
        float bottomMargin      = viewportBounds.proportionOfWidth(bottomMarginScalar);
        float horizontalMargin  = (viewportBounds.getWidth() - (3 * itemWidth)) * 0.143f;

        for (auto palette : allPalettes)
        {
            juce::FlexItem item(itemWidth, itemHeight, *palette);
            item.margin = juce::FlexItem::Margin(topMargin, horizontalMargin, bottomMargin, horizontalMargin);
            flexBox.items.add(item);
        }

        flexBox.performLayout(viewportBounds);

        float bottomMarginControlHeight = juce::roundToInt(viewportBounds.proportionOfWidth(btmMarginCtrlScalar));
        float bottomMarginControlSpace  = (bottomMargin - bottomMarginControlHeight) * 0.5f;

        float labelYItemOffset = itemHeight * 0.8f;
        for (int i = 0; i < controlGroups.size(); i++)
        {
            juce::FlexItem& item = flexBox.items.getReference(i + 1);
            juce::Rectangle<float> bottomMarginBounds(item.currentBounds.getX(), item.currentBounds.getBottom(), itemWidth, bottomMargin);
            int fourthWidthItem = bottomMarginBounds.proportionOfWidth(0.25f);

            juce::Rectangle<float> labelBounds = item.currentBounds.withTrimmedTop(labelYItemOffset);
            juce::Point<float> controlsPosition;
            if (paletteLabels[i]->getText().isNotEmpty())
            {
                auto label = paletteLabels[i];
                label->setBounds(labelBounds.toNearestInt());
                controlsPosition = bottomMarginBounds.getPosition().translated(0, bottomMarginControlSpace);
            }
            else
            {
                controlsPosition = labelBounds.getPosition();
            }

            auto group = controlGroups.getUnchecked(i);
            group->getEditButton()->setSize(fourthWidthItem, bottomMarginControlHeight);
            group->getEditButton()->setTopLeftPosition(controlsPosition.roundToInt().translated(bottomMarginBounds.proportionOfWidth(0.125f), 0));
            group->getCloneButton()->setBounds(group->getEditButton()->getBounds().translated(fourthWidthItem, 0));
            group->getTrashButton()->setBounds(group->getEditButton()->getBounds().translated(fourthWidthItem * 2.0f, 0));

            auto hitBox = juce::Rectangle<float>(item.currentBounds.getTopLeft().translated(-horizontalMargin, -topMargin), bottomMarginBounds.getBottomRight()).toNearestInt();
            controlGroupHitBoxes.set(i, hitBox);
        }

        dbgItems = flexBox.items;

        newPaletteBtn->setSize(itemWidth, bottomMarginControlHeight);
        newPaletteBtn->setTopLeftPosition(newPalette->getX(), newPalette->getY() + labelYItemOffset);
        newPaletteBtn->toFront(false);

        needsResize = false;
    }

    void mouseMove(const juce::MouseEvent& mouse) override
    {
        int newIndex = -1;
        if (mouse.eventComponent->getProperties().contains("index"))
        {
            newIndex = mouse.eventComponent->getProperties()["index"];
        }
        else
        {
            for (int i = 0; i < controlGroups.size(); i++)
            {
                if (controlGroupHitBoxes[i].contains(mouse.getEventRelativeTo(this).position.roundToInt()))
                {
                    newIndex = i;
                    break;
                }
            }
        }

        if (newIndex != lastPaletteMouseOver && lastPaletteMouseOver >= 0 && lastPaletteMouseOver < controlGroups.size())
            setControlsVisibleForPalette(lastPaletteMouseOver, false);

        if (newIndex >= 0 && newIndex < controlGroups.size())
        {
            setControlsVisibleForPalette(newIndex, true);
            lastPaletteMouseOver = newIndex;
        }
    }

    // Setup panels from scratch
    void rebuildPanel(juce::Array<LumatoneEditorColourPalette> palettesIn, int width = 0, bool resize = true)
    {
        for (auto group : controlGroups)
            selectionGroup->removeSelector(group->getPaletteComponent());

        removeAllChildren();
        addAndMakeVisible(newPaletteBtn.get());
        addAndMakeVisible(newPalette.get());

        controlGroups.clear();
        paletteLabels.clear();

        allPalettes = juce::Array<ColourPaletteComponent*>(newPalette.get());

        // Palettes with colour
        for (int i = 0; i < palettesIn.size(); i++)
        {
            auto group = controlGroups.add(new PaletteControlGroup(palettesIn.getReference(i)));

            auto paletteComponent = group->getPaletteComponent();
            paletteComponent->getProperties().set("index", i);
            addAndMakeVisible(paletteComponent);

            allPalettes.add(paletteComponent);

            if (selectionGroup)
                selectionGroup->addSelector(paletteComponent);

            group->getEditButton()->getProperties().set("index", i);
            group->getEditButton()->onClick = [&, i, paletteComponent] { listeners.call(&ColourPalettesPanel::Listener::editPaletteRequested, i, paletteComponent->getSelectedSwatchNumber()); };
            addChildComponent(group->getEditButton());

            group->getCloneButton()->getProperties().set("index", i);
            group->getCloneButton()->onClick = [&, i, paletteComponent] { listeners.call(&ColourPalettesPanel::Listener::clonePaletteRequested, i); };
            addChildComponent(group->getCloneButton());

            group->getTrashButton()->getProperties().set("index", i);
            group->getTrashButton()->onClick = [&, group, i] { listeners.call(&ColourPalettesPanel::Listener::deletePaletteRequested, i); };
            addChildComponent(group->getTrashButton());

            juce::String name = palettesIn[i].getName();
            auto label = paletteLabels.add(new juce::Label("Label_" + name, name));
            label->setJustificationType(juce::Justification::centred);
            label->getProperties().set("index", i);
            // label->getProperties().set(LumatoneEditorStyleIDs::labelMaximumLineCount, 2);
            addAndMakeVisible(*label);

            controlGroupHitBoxes.set(i, juce::Rectangle<int>());
        }

        int rows = ceil((palettesIn.size() + 1) * 0.333333f);

        int w = getWidth();

        // Set height depending on how many rows
        width = (width < 1) ? w : width;
        viewableHeight = getHeightFromNumRows(width, rows);

        if (resize)
        {
            needsResize = true;
            setSize(width, viewableHeight);

            // Force resize
            if (needsResize)
                resized();
        }

        numRows = rows;

    }

private:

    void setControlsVisibleForPalette(int paletteIndex, bool areVisible)
    {
        auto group = controlGroups.getUnchecked(paletteIndex);
        group->getEditButton()->setVisible(areVisible);
        group->getCloneButton()->setVisible(areVisible);
        group->getTrashButton()->setVisible(areVisible);
    }

private:

    ColourSelectionGroup* selectionGroup;

    std::unique_ptr<ColourPaletteComponent> newPalette;
    std::unique_ptr<juce::TextButton> newPaletteBtn;
    juce::OwnedArray<PaletteControlGroup> controlGroups;
    juce::Array<ColourPaletteComponent*> allPalettes;
    juce::OwnedArray<juce::Label> paletteLabels;

    juce::Array<juce::FlexItem> dbgItems;

    int numRows = 1;
    int viewableHeight = 0;
    bool needsResize = false;

    juce::Array<juce::Rectangle<int>> controlGroupHitBoxes;
    int lastPaletteMouseOver = -1;

    const float itemWidthScalar         = 0.265f;
    const float itemHeightScalar        = 0.24f;

    const float topMarginScalar         = 0.04f;
    const float horizontalMarginScalar  = 0.0367f;
    const float bottomMarginScalar      = 0.06f;
    const float btmMarginCtrlScalar     = 0.04f;

    const float buttonWidthScalar       = 0.333333f;
    const float buttonHeightScalar      = 0.166667f;

    const float panelLeftMarginWidth    = 0.020833f;

    //==============================================================================

public:

    class Listener
    {
    public:
        virtual ~Listener() {}

        virtual void editPaletteRequested(int paletteIndex, int selectedSwatchIndex) = 0;
        virtual void clonePaletteRequested(int paletteIndex) = 0;
        virtual void deletePaletteRequested(int paletteIndex) = 0;
        virtual void newPaletteRequested() = 0;
    };

    void addListener(Listener* listenerIn) { listeners.add(listenerIn); }
    void removeListener(Listener* listenerIn) { listeners.remove(listenerIn); }

protected:

    juce::ListenerList<ColourPalettesPanel::Listener> listeners;
};

#endif // LUMATONE_PALETTE_SELECTION_PANEL_H
