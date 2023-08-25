/*
  ==============================================================================

    lumatone_geometry.h
    Created: 9 May 2016 9:04:35pm
    Author:  hsstraub

  ==============================================================================
*/

#ifndef BOARDGEOMETRY_H_INCLUDED
#define BOARDGEOMETRY_H_INCLUDED

#include <JuceHeader.h>

// Number of rows by which each octave board is offset
#define BOARDROWOFFSET 2

enum class LumatoneBoardSize
{
    PROTOTYPE,
    LM56
};

static int GetSizeOfBoard(LumatoneBoardSize boardSize)
{
    switch (boardSize)
    {
    case LumatoneBoardSize::PROTOTYPE:
        return 55;
    default:
        break;
    }

    return 56;
}

static LumatoneBoardSize GetLumatoneBoardSize(int octaveBoardSize)
{
    switch (octaveBoardSize)
    {
        case 55:
            return LumatoneBoardSize::PROTOTYPE;

        default:
        case 56:
            return LumatoneBoardSize::LM56;
    }
}


//==============================================================================
// Manages the geometry of the board - used to create isomorphic key mappings
class LumatoneGeometry
{
    // Types
public:
    typedef juce::Array<int> StraightLine;
    typedef juce::Array<StraightLine> StraightLineSet;

    // Construction
public:
    LumatoneGeometry(LumatoneBoardSize boardSize=LumatoneBoardSize::LM56);

    // Methods
public:
    StraightLine horizontalLineOfField(int fieldIndex) const { return getLineOfField(fieldIndex, this->horizontalLines); };
    StraightLine rightUpwardLineOfField(int fieldIndex) const { return getLineOfField(fieldIndex, this->rightUpwardLines); };

    StraightLine horizontalLine(int rowIndex) const { return horizontalLines[rowIndex]; }
    StraightLine rightUpwardLine(int colIndex) const { return rightUpwardLines[colIndex]; }

    int horizontalLineCount() const { return this->horizontalLines.size(); }
    int rightUpwardLineCount() const { return this->rightUpwardLines.size(); }

    int horizontalLineSize(int rowIndex) const { return (rowIndex >= 0 && rowIndex < horizontalLineCount()) ? this->horizontalLines[rowIndex].size() : 0; }
    int firstColumnOffset(int rowIndex) const { return (rowIndex >= 0 && rowIndex < firstColumnOffsets.size()) ? this->firstColumnOffsets[rowIndex] : 0; }

    StraightLineSet globalHorizontalLineOfField(int setSelection, int fieldIndex) const;
    StraightLineSet globalRightUpwardLineOfField(int setSelection, int fieldIndex) const;

    StraightLineSet getHorizontalLinesWithContinuation(int octaveBoardOffset) const;

    StraightLine getLineOfField(int fieldIndex, StraightLineSet lineSet) const;
    StraightLine continuationOfHorizontalLine(StraightLine line, int octaveBoardOffset) const;
    StraightLine continuationOfRightUpwardLine(StraightLine line, int octaveBoardOffset) const;

    int getMaxHorizontalLineSize() const;

    juce::Array<juce::Point<int>> getOctaveCoordinates(int boardIndex) const;

    int getLastIndexForRow(int rowIndex) const;
    int getVerticalOriginLineIndexForRow(int rowIndex) const;

    juce::Point<int> coordinatesForKey (int boardIndex, int keyIndex) const;

    // Attributes
private:
    int octaveBoardSize = 0;

    StraightLineSet    horizontalLines;
    StraightLineSet    rightUpwardLines;
    
    juce::Array<int> firstColumnOffsets;
    juce::Array<int> rowOffsets; // Horizonal offset of each horizontal line on the board, relative to the lower left key.
    
    juce::Array<int> rowVerticalOriginLineIndex;    // Key index for the key in a vertical line with the board's 0th key, for a given row

    int boardXOffset; // Offset along shallow diagonals between analogous keys from one board to the next (typically 7)
    int boardYOffset; // Offset along up/right diagonals between analogous keys from one board to the next (typically -2)
    int maxHorizontalLineSize;
};

#endif  // BOARDGEOMETRY_H_INCLUDED
