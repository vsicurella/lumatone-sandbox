/*
  ==============================================================================

    NotationDefinition.h
    Created: 10 May 2022 9:35:34pm
    Author:  Vincenzo

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h> 
#include "./NotationTable.h"

class NotationDefinition
{
    juce::Font accidentalFont;

    int scaleSize = 0;

    // Lists of unique note names
    juce::Array<juce::String> nominals;
    juce::Array<juce::String> accidentals;

    juce::Array<int> degreeNominalMap;
    juce::Array<int> degreeAccidentalMap;

    juce::String error;
    juce::String warning;

private:

    bool validateNominalMapping(const juce::Array<juce::String>& nominals, const juce::Array<int>& degreeNominalMap);

    bool validateAccidentalMapping(const juce::Array<juce::String>& accidentals, const juce::Array<int>& degreeAccidentalMap);

    void initializeLabels(
        const juce::Array<juce::String>& nominalsIn,
        const juce::Array<int>& nominalMapIn,
        const juce::Array<juce::String>& accidentalsIn,
        const juce::Array<int> accidentalsMapIn
    );

    void initializeLabels(
        const juce::Array<int>& nominalMapIn,
        const juce::Array<juce::String>& accidentalsIn,
        const juce::Array<int> accidentalsMapIn
    );

public:

    NotationDefinition(
        int size,
        juce::Array<juce::String> nominalsIn,
        juce::Array<int> degreeToNominals,
        juce::Array<juce::String> accidentalsIn,
        juce::Array<int> degreeToAccidentals,
        juce::Font accidentalFontIn = juce::Font()
    );

    // Use generic nominals starting on A
    NotationDefinition(
        int size,
        juce::Array<int> degreeToNominalsIn,
        juce::Array<juce::String> accidentals,
        juce::Array<int> degreeToAccidentalsIn,
        juce::Font accidentalFontIn = juce::Font()
    );

    bool hasError() const { return error.isNotEmpty(); }
    juce::String getError() const { return error; }

    bool hasWarning() const { return warning.isNotEmpty(); }
    juce::String getWarning() const { return warning; }

    NotationTable generateTable() const;
};