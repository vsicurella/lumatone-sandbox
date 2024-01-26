/*
==============================================================================

LumatoneDataStructures.h
Created: XXX
Author:  hsstraub

==============================================================================
*/

#ifndef LUMATONE_LAYOUT_STATE_H
#define LUMATONE_LAYOUT_STATE_H

#include "./lumatone_board.h"

namespace LumatoneConfigProperty
{
	static const juce::Identifier State = juce::Identifier("LumatoneConfigState");
	static const juce::Identifier LayoutState = juce::Identifier("LayoutState");

	static const juce::Identifier AftertouchEnabled = juce::Identifier("AftertouchEnabled");
	static const juce::Identifier LightOnKeyStrokes = juce::Identifier("LightOnKeyStrokes");
	static const juce::Identifier InvertExpression = juce::Identifier("InvertExpression");
	static const juce::Identifier InvertSustain = juce::Identifier("InvertSustain");
	static const juce::Identifier ExpressionSensitivity = juce::Identifier("ExpressionSensitivity");

	static const juce::Identifier VelocityIntervalTable = juce::Identifier("VelocityIntervalTable");
	static const juce::Identifier NoteVelocityTable = juce::Identifier("NoteVelocityTable");
	static const juce::Identifier AftertouchTable = juce::Identifier("AftertouchTable");
	static const juce::Identifier LumatouchTable = juce::Identifier("LumatouchTable");
}

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

	juce::String toConfigString() const;

	void setValues(const int* values);

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
	LumatoneLayout(juce::ValueTree stateIn);
	LumatoneLayout(juce::StringArray stringArray);

	void clearVelocityIntervalTable(LumatoneConfigTable* table);
	void clearAll(bool initializeWithNoteKeyType=false);
    bool isEmpty() const;

	void operator=(const LumatoneLayout& toCopy);

	void fromStringArray(const juce::StringArray& stringArray);
	juce::StringArray toStringArray() const;

	juce::ValueTree getState() const { return state; }

	int getNumBoards() const { return numBoards; }
	int getOctaveBoardSize() const { return octaveBoardSize; }

	bool getAftertouchOn() const { return afterTouchActive; }
	bool getLightOnKeyStrokes() const { return lightOnKeyStrokes; }
	bool getInvertExpression() const { return invertExpression; }
	bool getInvertSustain() const { return invertSustain; }
	int getExpressionSensitivity() const { return expressionSensitivity; }

	const LumatoneConfigTable* getConfigTable(LumatoneConfigTable::TableType tableType) const;
	void setConfigTable(LumatoneConfigTable::TableType tableType, const int* table);

	juce::Array<juce::Colour> getLayoutColours() const;
    juce::Array<LumatoneKeyCoord> getKeysWithColour(const juce::Colour& c) const;

	// Key configuration
	const LumatoneBoard& getBoard(int index) const;
	virtual void setBoard(const LumatoneBoard& boardIn, int boardIndex);

	const LumatoneKey& getKey(int boardIndex, int keyIndex) const;
	MappedLumatoneKey getMappedKey(int boardIndex, int keyIndex) const;

	virtual void setKey(const LumatoneKey& keyIn, int boardIndex, int keyIndex);
	virtual void setKey(MappedLumatoneKey keyIn);
	virtual void setKeyConfig(const LumatoneKey& keyIn, int boardIndex, int keyIndex);
	virtual void setKeyColour(juce::Colour colour, int boardIndex, int keyIndex);

	// Config
	void setOctaveBoardSize(int size);

	virtual void setAftertouchEnabled(bool enabled);
	virtual void setLightOnKeyStrokes(bool enabled);
	virtual void setInvertExpression(bool invert);
	virtual void setInvertSustain(bool invert);
	virtual void setExpressionSensitivity(juce::uint8 value);

	// Mutators
	virtual void transform(std::function<void(int,int,LumatoneKey&)> transformFnc);
	virtual LumatoneLayout withKeyTransform(std::function<void(int,int,LumatoneKey&)> transformFnc) const;

public:
	juce::Array<LumatoneKeyCoord> getAllKeyCoords() const;

	bool isKeyCoordValid(const LumatoneKeyCoord& coord) const;
	bool isKeyCoordValid(int keyIndex, int boardIndex) const;

	int keyCoordToKeyNum(const LumatoneKeyCoord& coord) const;
	int keyCoordToKeyNum(int keyIndex, int boardIndex) const;

	LumatoneKeyCoord keyNumToKeyCoord(int keyNum) const;

public:
	static LumatoneLayout IdentityMapping(int numBoards=MAXNUMBOARDS, int octaveBoardSize=56, LumatoneKeyType keyType=LumatoneKeyType::noteOnNoteOff);

private:

	void updateState();
	void refreshFromState();

private:
	int numBoards = 0;
	int octaveBoardSize = 0;

	// General options
	bool afterTouchActive;
	bool lightOnKeyStrokes;
	bool invertExpression;
	bool invertSustain;
	int expressionSensitivity;

	// Velocity curves
	int table[VELOCITYINTERVALTABLESIZE];

	LumatoneConfigTable	velocityTable;
	LumatoneConfigTable faderTable;
	LumatoneConfigTable afterTouchTable;
	LumatoneConfigTable lumaTouchTable;

private:
	juce::ValueTree state;
};

#endif LUMATONE_LAYOUT_STATE_H
