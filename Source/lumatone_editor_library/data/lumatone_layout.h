/*
==============================================================================

LumatoneDataStructures.h
Created: XXX
Author:  hsstraub

==============================================================================
*/

#pragma once
#include "./lumatone_board.h"

// Velocity curve config
class LumatoneConfigTable
{
public:

	typedef enum
	{
		none = -1,
		freeDrawing = 0,
		linearSegments = 1,
		quadraticCurves = 2
	} DrawMode;

	typedef enum
	{
		velocityInterval = 1,
		fader = 2,
		afterTouch = 3,
		lumaTouch = 4
		// ToDo modulation wheel
	} TableType;

public:
	LumatoneConfigTable();
	// Default value for the given curve type
	LumatoneConfigTable(TableType tableType);
	LumatoneConfigTable(const juce::String& configString);

	juce::String toConfigString();

public:
	DrawMode editStrategy;

	// Velocity curve values. Meaning varies depending on editStrategy
	int velocityValues[128];
};

/*
==============================================================================
A complete key mapping
==============================================================================
*/
#define MAXNUMBOARDS 5

// Number of entries in the velocity interval table
#define VELOCITYINTERVALTABLESIZE 127

class LumatoneLayout
{
private:
	LumatoneBoard boards[MAXNUMBOARDS];

public:
	LumatoneLayout(int numBoards=5, int octaveBoardSize=56, bool initializeWithNotes=false);

	void clearVelocityIntervalTable();
	void clearAll(bool initializeWithNoteKeyType=false);
    bool isEmpty() const;

	void fromStringArray(const juce::StringArray& stringArray);
	juce::StringArray toStringArray();

	int getNumBoards() const { return numBoards; }
	int getOctaveBoardSize() const { return octaveBoardSize; }

	LumatoneConfigTable* getConfigTable(LumatoneConfigTable::TableType tableType);

	juce::Array<juce::Colour> getLayoutColours() const;
    juce::Array<LumatoneKeyCoord> getKeysWithColour(const juce::Colour& c) const;

	// Key configuration
	const LumatoneBoard* readBoard(int index) const;
	LumatoneBoard* getBoard(int index);

	const LumatoneKey* readKey(int boardIndex, int keyIndex) const;

public:

	int numBoards = 0;
	int octaveBoardSize = 0;

	// General options
	bool afterTouchActive;
	bool lightOnKeyStrokes;
	bool invertExpression;
	bool invertSustain;
	int expressionControllerSensivity;

	// Velocity curves
	int table[VELOCITYINTERVALTABLESIZE];

	LumatoneConfigTable	velocityTable;
	LumatoneConfigTable faderTable;
	LumatoneConfigTable afterTouchTable;
	LumatoneConfigTable lumaTouchTable;
};
