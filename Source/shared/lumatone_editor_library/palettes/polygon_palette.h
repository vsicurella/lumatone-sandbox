/*
  ==============================================================================

    polygon_palette.h
    Created: 13 Nov 2020 7:24:40pm
    Author:  Vincenzo

  ==============================================================================
*/

#pragma once

#include "palette_base.h"


//==============================================================================
/*
*/
class PolygonPalette  : public Palette
{
public:
    PolygonPalette(int numSidesIn = 6, float angleOffsetIn = juce::MathConstants<float>::pi / 12.0f)
        : Palette(numSidesIn), angleOffset(angleOffsetIn)
    {
        createSwatches();
    }

    ~PolygonPalette() override
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
//            g.setColour(palette[i].contrasting());
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
        // Original bounds
        juce::Rectangle<float> originalBounds = { 0.0f, 0.0f, 1.0f, 1.0f };
        
        float size = juce::jmin(getWidth(), getHeight());

        // Crop to square
        juce::Rectangle<float> scaled = { 0, 0, size, size };

        // Get scaling and centering transform
        juce::AffineTransform transform = juce::RectanglePlacement().getTransformToFit(originalBounds, scaled.withCentre({ (float)getLocalBounds().getCentreX(), size * 0.5f }));

        for (int i = 0; i < getNumberOfSwatches(); i++)
        {
            juce::Path p = triangles[i];
            p.applyTransform(transform);
            swatchPaths.set(i, p);
        }
    }

private:

    void createSwatches()
    {
        float angInc = 2 * juce::MathConstants<float>::pi / getNumberOfSwatches();
        float angMargin = angInc * margin * 0.5f;

        for (int i = 0; i < getNumberOfSwatches(); i++)
        {
            juce::Path swatchPath;
            
            float angFrom  =  angInc *  i      + angleOffset + angMargin;
            float angTo    =  angInc * (i + 1) + angleOffset - angMargin;
            float bisector = (angFrom + angTo) * 0.5f;

            juce::Point<float> pointFrom = {
                center.x + cosf(angFrom)  * outerRadius + cosf(angFrom) * innerRadius,
                center.y + sinf(angFrom)  * outerRadius + sinf(angFrom) * innerRadius
            };

            juce::Point<float> pointTo = {
                center.x + cosf(angTo)    * outerRadius + cosf(angTo)   * innerRadius,
                center.y + sinf(angTo)    * outerRadius + sinf(angTo)   * innerRadius
            };
            
            juce::Point<float> pointCenter = {
                center.x + cosf(bisector) * innerRadius,
                center.y + sinf(bisector) * innerRadius
            };

            swatchPath.addTriangle(pointFrom, pointTo, pointCenter);
            
            triangles.add(swatchPath);
            swatchPaths.add(swatchPath);
        }

        triangles.minimiseStorageOverheads();
        swatchPaths.minimiseStorageOverheads();
    }

private:

    const float angleOffset;

    const juce::Point<float> center = { 0.5f, 0.5f };

    const float margin = 0.025f;
    const float radiusBounds = 0.5f;

    const float outerRadius = radiusBounds * (1 - margin);
    const float innerRadius = radiusBounds * margin;

    float width = 1, height = 1;

    juce::Array<juce::Path> triangles;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PolygonPalette)
};
