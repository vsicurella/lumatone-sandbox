/*
  ==============================================================================

    NotationDefinition.cpp
    Created: 10 May 2022 9:35:34pm
    Author:  Vincenzo

  ==============================================================================
*/

#include "NotationDefinition.h"

NotationDefinition::NotationDefinition(
    int size,
    juce::Array<juce::String> nominalsIn,
    juce::Array<int> degreeToNominals,
    juce::Array<juce::String> accidentalsIn,
    juce::Array<int> degreeToAccidentals,
    juce::Font accidentalFontIn
) : scaleSize(size), accidentalFont(accidentalFontIn)
{
    initializeLabels(nominalsIn, degreeToNominals, accidentals, degreeToAccidentals);
}

NotationDefinition::NotationDefinition(
    int size,
    juce::Array<int> degreeToNominalsIn,
    juce::Array<juce::String> accidentals,
    juce::Array<int> degreeToAccidentalsIn,
    juce::Font accidentalFontIn
) : scaleSize(size), accidentalFont(accidentalFontIn)
{
    initializeLabels(degreeToNominalsIn, accidentals, degreeToAccidentalsIn);
}

bool NotationDefinition::validateNominalMapping(const juce::Array<juce::String>& nominalsIn, const juce::Array<int>& degreeNominalMap)
{
    if (nominalsIn.size() == 0)
    {
        error = "Nominals list is empty.";
        return false;
    }

    // Check for invalid nominals mappings
    for (int i = 0; i < degreeNominalMap.size(); i++)
    {
        int index = degreeNominalMap[i];
        if (index >= nominalsIn.size())
        {
            error = juce::String("Degree") + juce::String(i) + " exceeds nominal list with index " + juce::String(index);
            return false;
        }
        else if (index < 0)
        {
            error = juce::String("Degree") + juce::String(i) + " has invalid nominal index " + juce::String(index);
            return false;
        }

        if (i >= scaleSize)
        {
            warning = juce::String("Degree nominal map is larger than scale size");
            break;
        }
    }

    return true;
}

bool NotationDefinition::validateAccidentalMapping(const juce::Array<juce::String>& accidentalsIn, const juce::Array<int>& degreeAccidentalMap)
{
    // Check for invalid accidental mappings
    for (int i = 0; i < degreeAccidentalMap.size(); i++)
    {
        int index = degreeAccidentalMap[i];
        if (index >= accidentals.size())
        {
            error = juce::String("Degree") + juce::String(i) + " exceeds accidental list with index " + juce::String(index);
            return false;
        }
        else if (index < 0)
        {
            error = juce::String("Degree") + juce::String(i) + " has invalid accidental index " + juce::String(index);
            return false;
        }

        if (i >= scaleSize)
        {
            warning = juce::String("Degree accidental map is larger than scale size");
            break;
        }
    }

    return true;
}

void NotationDefinition::initializeLabels(
    const juce::Array<juce::String>& nominalsIn,
    const juce::Array<int>& nominalMapIn,
    const juce::Array<juce::String>& accidentalsIn,
    const juce::Array<int> accidentalsMapIn
)
{
    if (!validateNominalMapping(nominalsIn, nominalMapIn))
        return;

    nominals = nominalsIn;
    degreeNominalMap = nominalMapIn;

    if (!validateAccidentalMapping(accidentalsIn, accidentalsMapIn))
        return;

    accidentals = accidentalsIn;
    degreeAccidentalMap = accidentalsMapIn;
}

void NotationDefinition::initializeLabels(
    const juce::Array<int>& nominalMapIn,
    const juce::Array<juce::String>& accidentalsIn,
    const juce::Array<int> accidentalsMapIn
)
{
    // For now just make a list of the 26 latin letters
    auto alphabet = juce::Array<juce::String>();
    for (int i = 0; i < 26; i++)
    {
        char ch = i + 26;
        alphabet.add(juce::String::fromUTF8(&ch, 1));
    }

    initializeLabels(alphabet, nominalMapIn, accidentalsIn, accidentalsMapIn);
}

NotationTable NotationDefinition::generateTable() const
{
    auto nominalList = juce::Array<juce::String>();
    for (auto nominalIndex : degreeNominalMap)
        nominalList.add(nominals[nominalIndex]);

    auto accidentalList = juce::Array<juce::String>();
    for (auto accidentalIndex : degreeAccidentalMap)
        accidentalList.add(accidentals[accidentalIndex]);

    return NotationTable(nominalList, accidentalList, accidentalFont);
}
