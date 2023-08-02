/*
  ==============================================================================

    NotationTable.h
    Created: 8 May 2022 4:24:32pm
    Author:  Vincenzo

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class NotationTable
{
    Font accidentalsFont;
    
    juce::Array<juce::String> degreeNominals;
    juce::Array<juce::String> degreeAccidentals;

public:

    NotationTable(juce::Array<juce::String> degreeNominalsIn, juce::Array<juce::String> degreeAccidentalsIn, Font accidentalsFontIn = Font());

    Font getAccidentalsFont() const { return accidentalsFont; };

    juce::String getNoteNominal(int scaleDegree) const;

    juce::String getNoteAccidental(int scaleDegree) const; 

    juce::String getNoteNameString(int scaleDegree) const;
    
    juce::String getPitchNameString(int scaleDegree, int periodNum) const;

    void drawNoteName(int scaleDegree, juce::Graphics g, juce::Rectangle<int> bounds) const;

    void drawPitchName(int scaleDegree, int periodNum, juce::Graphics g, juce::Rectangle<int> bounds) const;
};