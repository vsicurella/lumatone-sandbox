#pragma once

#include "./colour_palette_file.h"

class LumatonePaletteLibrary
{
public:

    LumatonePaletteLibrary()
    { 
        juce::Array<juce::Colour> test;
        test.add(juce::Colours::red);
        test.add(juce::Colours::blue);
        test.add(juce::Colours::green);
        test.add(juce::Colours::yellow);
        test.add(juce::Colours::cyan);
        test.add(juce::Colours::magenta);
        test.add(juce::Colours::white);

        palettes.clear();
        palettes.add(LumatoneEditorColourPalette(test));
    }
    ~LumatonePaletteLibrary() {}

    juce::Array<LumatoneEditorColourPalette> getPalettes() const { return palettes; }
    juce::Array<LumatoneEditorColourPalette>& getPalettesReference() { return palettes; }

private:

    juce::Array<LumatoneEditorColourPalette> palettes;

};
