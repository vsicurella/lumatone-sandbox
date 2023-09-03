/*
  ==============================================================================

    NotationTable.cpp
    Created: 8 May 2022 4:24:32pm
    Author:  Vincenzo

  ==============================================================================
*/

#include "NotationTable.h"

NotationTable::NotationTable(juce::Array<juce::String> degreeNominalsIn, juce::Array<juce::String> degreeAccidentalsIn, juce::Font accidentalsFontIn)
    : degreeNominals(degreeAccidentalsIn), degreeAccidentals(degreeAccidentalsIn), accidentalsFont(accidentalsFontIn) {}

juce::String NotationTable::getNoteNominal(int scaleDegree) const
{
    return degreeNominals[scaleDegree];
}

juce::String NotationTable::getNoteAccidental(int scaleDegree) const
{
    return degreeAccidentals[scaleDegree];
}

juce::String NotationTable::getNoteNameString(int scaleDegree) const
{
    // accidental alias?
    return getNoteNominal(scaleDegree) + getNoteAccidental(scaleDegree);
}

juce::String NotationTable::getPitchNameString(int scaleDegree, int periodNum) const
{
    // accidental alias?
    return getNoteNameString(scaleDegree) + juce::String(periodNum);

}

void NotationTable::drawNoteName(int scaleDegree, juce::Graphics g, juce::Rectangle<int> bounds) const
{
    juce::Font nominalFont = g.getCurrentFont();

    auto nominal = getNoteNominal(scaleDegree);
    auto nominalWidth = nominalFont.getStringWidth(nominal);

    auto accidental = getNoteAccidental(scaleDegree);
    auto accidentalWidth = accidentalsFont.getStringWidth(accidental);

    g.drawFittedText(nominal, bounds.withWidth(nominalWidth), juce::Justification::centredLeft, 1);

    g.setFont(accidentalsFont);
    g.drawFittedText(accidental, bounds.withTrimmedLeft(nominalWidth), juce::Justification::centredLeft, 1);

    g.setFont(nominalFont);
}

void NotationTable::drawPitchName(int scaleDegree, int periodNum, juce::Graphics g, juce::Rectangle<int> bounds) const
{
    juce::Font nominalFont = g.getCurrentFont();

    auto nominal = getNoteNominal(scaleDegree);
    auto nominalWidth = nominalFont.getStringWidth(nominal);

    auto accidental = getNoteAccidental(scaleDegree);
    auto accidentalWidth = accidentalsFont.getStringWidth(accidental);

    auto number = juce::String(periodNum);
    auto numberWidth = nominalFont.getStringWidth(number);

    g.drawFittedText(nominal, bounds.withWidth(nominalWidth), juce::Justification::centredLeft, 1);

    g.setFont(accidentalsFont);
    g.drawFittedText(accidental, bounds.withTrimmedLeft(nominalWidth).withWidth(accidentalWidth), juce::Justification::centredLeft, 1);

    g.setFont(nominalFont);
    g.drawFittedText(number, bounds.withTrimmedLeft(nominalWidth + accidentalWidth), juce::Justification::centredLeft, 1);
}
