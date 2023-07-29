/*
  ==============================================================================

    ViewComponents.h
    Created: 16 Jun 2014 7:20:34pm
    Author:  hsstraub

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

#include "LumatoneDataStructures.h"
#include "ViewConstants.h"


class LumatoneKeyEditListener
{
public:
	virtual ~LumatoneKeyEditListener() {}

	virtual void keyClickedCallback(const juce::MouseEvent& e, int boardIndex, int keyIndex, LumatoneKey keyValue) = 0;
};

/*
==============================================================================
Display of the data of one key
==============================================================================
*/
class LumatoneKeyEdit : public juce::Component, public juce::SettableTooltipClient
{
public:
	LumatoneKeyEdit(int boardIndex, int keyIndex);
	~LumatoneKeyEdit();

	int getBoardIndex() const;
	int getKeyIndex() const;

	LumatoneKey getValue() const;
	void setValue(const LumatoneKey& newValue);

	void setIsSelected(bool newValue);

	void paint(juce::Graphics& g);
	void resized();

	void setKeySize(float keySizeIn);

	enum ColourIds
	{
		backgroundColourId = 0x2000100,
		outlineColourId = 0x2000101,
		selectedKeyOutlineId = 0x2000102
	};

	void mouseDown(const juce::MouseEvent& e) override;

	juce::String toString() const;

public:

	void addKeyEditListener(LumatoneKeyEditListener* listenerIn);
	void removeKeyEditListener(LumatoneKeyEditListener* listenerIn);

protected:

	juce::ListenerList<LumatoneKeyEditListener> listeners;

private:
	bool isSelected;

	int board_idx = -1;
	int key_idx = -1;

	juce::Path    hexPath;
	juce::Label*	midiNoteLabel;
	juce::Label*  midiChannelLabel;

	juce::Colour			keyColour;
	LumatoneKeyType			keyType;
    bool                    ccFaderDefault;
	
	float					keySize;

	const float				channelLabelRadiusScalar = 0.4347826f;
};


struct LumatoneKeyPtr
{
	int board_idx = -1;
	int key_idx = -1;
	LumatoneKey* key = nullptr;

	LumatoneKeyPtr() {}
	LumatoneKeyPtr(int boardIndex, int keyIndex, LumatoneKey* keyPtr = nullptr);

	bool isValid() const;

	bool isNull() const;
};

using LumatoneKeySelection = juce::Array<LumatoneKeyPtr>;

struct LumatoneBoardSet
{
	int board_idx = -1;
	std::unique_ptr<LumatoneKeyEdit> keys[56];

	LumatoneBoardSet(int boardIndex);
};

