/*
  ==============================================================================

    colour_model.h
    Created: 9 Aug 2023 12:34:31am
    Author:  Vincenzo

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

#define MAX_INCREMENT 16

class LumatoneColourModel
{
    typedef float ColourTable[MAX_INCREMENT][MAX_INCREMENT][MAX_INCREMENT][3];

public:

    LumatoneColourModel();
    ~LumatoneColourModel();


private:

    void readTable(const juce::var& tableVar, ColourTable& table);

    void parseTable();


private:


    int increment = MAX_INCREMENT;

    ColourTable raw;
    ColourTable adjusted;
};