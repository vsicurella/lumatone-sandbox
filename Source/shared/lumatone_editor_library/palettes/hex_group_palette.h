/*
  ==============================================================================

    polygon_palette.h
    Created: 13 Nov 2020 7:24:40pm
    Author:  Vincenzo

  ==============================================================================
*/

#pragma once

#include "palette_base.h"

#include "../mapping/lumatone_tiling.h"

//==============================================================================
/*
*/
class HexagonPalette: public Palette
{
public:
    HexagonPalette(juce::Array<juce::Point<int>> hexCoordsIn, double rotationAngle = 0.0)
        : Palette(hexCoordsIn.size())
        , hexCoordinates(hexCoordsIn)
        , angleOffset(rotationAngle)
    {
        for (auto point : hexCoordinates)
        {
            if (point.x > numColumns)
                numColumns = point.x;

            if (point.y > numRows)
                numRows = point.y;
        }

        numColumns += 1;
        numRows += 1;

        createSwatches();
    }

    ~HexagonPalette() override
    {
    }

    void paint(juce::Graphics& g) override
    {
        for (int i = 0; i < getNumberOfSwatches(); i++)
        {
            juce::Path& swatch = swatchPaths.getReference(i);

            g.setColour(getSwatchColour(i));
            g.fillPath(swatch);

//#if JUCE_DEBUG
//            g.setColour(getSwatchColour(i).contrasting());
//            g.drawFittedText(String(i), swatch.getBounds().toNearestInt(), Justification::centred, 1);
//#endif
        }

        if (selectedSwatch >= 0)
        {
            g.setColour(juce::Colours::white);
            g.strokePath(swatchPaths[selectedSwatch], juce::PathStrokeType(2.0f));
        }
    }

    void resized() override
    {
        float scalar = (width > height)
            ? getWidth() / width
            : getHeight() / height;

        scalar *= (1 - 0.05f); // kludge margin to fix it getting cutoff sometimes

        float newWidth = width * scalar;
        float newHeight = height * scalar;

        float xOffset = (getWidth() - newWidth) * 0.5f - getWidth() * 0.125f; // Last bit is a correction needed in HexagonTiling
        float yOffset = (getHeight() - newHeight) * 0.5f;

        // Get scaling and centering transform
        juce::AffineTransform transform = juce::AffineTransform()
            .followedBy(juce::AffineTransform::scale(scalar))
            .followedBy(juce::AffineTransform::translation(xOffset, 0));

        for (int i = 0; i < getNumberOfSwatches(); i++)
        {
            juce::Path p = swatchPaths[i];
            p.applyTransform(transform);
            swatchPaths.set(i, p);
        }
    }

private:

    void createSwatches()
    {
        LumatoneTiling tiling;
        tiling.fitTilingTo(juce::Rectangle<float>(0.0, 0.0, width, height),
            numColumns, numRows, margin, 0, true
        );

        juce::Array<juce::Point<float>> centres = tiling.transformPointsFromOrigin(hexCoordinates);

        juce::AffineTransform transform;

        // TODO get rotation pivot
        if (angleOffset != 0.0)
            transform = juce::AffineTransform::rotation(angleOffset);

        // For finding actual width and height
        juce::Path tilePath;

        swatchPaths.clear();
        for (auto c : centres)
        {
            c.applyTransform(transform);

            juce::Path hex;
            hex.addPolygon(c, 6, tiling.getRadius());
            swatchPaths.add(hex);

            tilePath.addPath(hex);
        }

        juce::Rectangle<float> tileBounds = tilePath.getBounds(); // does this clip (juce::translate) for optimal size, or does it retain original origin?
        width = tileBounds.getWidth();
        height = tileBounds.getHeight();
    }

private:

    const juce::Array<juce::Point<int>> hexCoordinates;
    const float angleOffset;

    const float margin = 0.024f;
    float width = 1, height = 1;
    
    int numColumns = 0;
    int numRows = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HexagonPalette)
};

class TenHexagonPalette : public HexagonPalette
{
    juce::Array<juce::Point<int>> createCoordinates()
    {
        juce::Array<juce::Point<int>> coords = { 
			juce::Point<int>(1, 0), 
			juce::Point<int>(2, 0), 
			juce::Point<int>(3, 0),
			juce::Point<int>(0, 1), 
			juce::Point<int>(1, 1), 
			juce::Point<int>(2, 1), 
			juce::Point<int>(3, 1),
			juce::Point<int>(0, 2), 
			juce::Point<int>(1, 2), 
			juce::Point<int>(2, 2)
        };

        return coords;
    }

public:

    TenHexagonPalette(double rotationAngle = 0.0)
        : HexagonPalette(createCoordinates(), rotationAngle)
    {}
};