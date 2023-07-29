/*
==============================================================================

LumatoneDataStructures.h
Created: XXX
Author:  hsstraub

==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "lumatone_midi_driver/firmware_definitions.h"

// Mapping of one key
class LumatoneKey
{
public:
	typedef juce::Colour COLOURTYPE;

public:
    LumatoneKey(LumatoneKeyType newKeyType = LumatoneKeyType::disabled) { noteNumber = 0; channelNumber = 1; colour = juce::Colour(); keyType = newKeyType; ccFaderDefault = true; };
	LumatoneKey(LumatoneKeyType newKeyType, int newChannelNumber, int newNoteNumber, COLOURTYPE newColour, bool invertCCFader = false)
	{
        keyType = newKeyType; channelNumber = newChannelNumber; noteNumber = newNoteNumber; colour = newColour; ccFaderDefault = invertCCFader;
	}
	bool isEmpty() const { return channelNumber == 0; }

    bool operator!=(const LumatoneKey& second) const { return noteNumber != second.noteNumber || channelNumber != second.channelNumber || colour != second.colour || keyType != second.keyType || ccFaderDefault != second.ccFaderDefault; }
    
    // Chainable setters
    LumatoneKey withKeyType(LumatoneKeyType type) const { return LumatoneKey(type, channelNumber, noteNumber, colour, ccFaderDefault); }
    LumatoneKey withColour(juce::Colour newColour) const { return LumatoneKey(keyType, channelNumber, noteNumber, newColour, ccFaderDefault); }
    LumatoneKey withNoteOrCC(int noteOrCC) const { return LumatoneKey(keyType, channelNumber, noteOrCC, colour, ccFaderDefault); }
    LumatoneKey withChannelNumber(int channel) const { return LumatoneKey(keyType, channel, noteNumber, colour, ccFaderDefault); }
    LumatoneKey withInvertCCFader(bool invertCCFader) const { return LumatoneKey(keyType, channelNumber, noteNumber, colour, invertCCFader); }

public:
	int			noteNumber;
	int			channelNumber;
	COLOURTYPE	colour;
    bool        ccFaderDefault;
	LumatoneKeyType		keyType;
};

// Subset of 56 (or 55) keys
struct LumatoneBoard {
	LumatoneKey		theKeys[56];
	int				board_idx;
	int				key_idx;

	LumatoneBoard(LumatoneKeyType newKeyType = LumatoneKeyType::disabled);

	bool isEmpty() const;
};

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
#define NUMBEROFBOARDS 5

// Number of rows by which each octave board is offset
#define BOARDROWOFFSET 2

// Number of entries in the velocity interval table
#define VELOCITYINTERVALTABLESIZE 127

class LumatoneLayout
{
private:
	LumatoneBoard boards[NUMBEROFBOARDS];

public:
	LumatoneLayout();

	virtual void clearVelocityIntervalTable();
	virtual void clearAll(bool initializeWithNoteKeyType=false);
    virtual bool isEmpty() const;

	virtual void fromStringArray(const juce::StringArray& stringArray);
	virtual juce::StringArray toStringArray();

	// The colours that are used
	//SortedSet<TerpstraKey::COLOURTYPE> getUsedColours();

	LumatoneConfigTable* getConfigTable(LumatoneConfigTable::TableType tableType);

	// Key configuration
	const LumatoneBoard* readBoard(int index) const;
	LumatoneBoard* getBoard(int index);


public:

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
