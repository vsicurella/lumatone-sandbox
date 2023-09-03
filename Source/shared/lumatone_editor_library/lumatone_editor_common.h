#pragma once
#include <JuceHeader.h>

namespace LumatoneEditor
{
    using ColourHash = unsigned int;

    static ColourHash getColourHash(juce::Colour colour)
    {
        return (colour.getRed() << 16) | (colour.getGreen() << 8) | colour.getBlue();
    }

    static juce::uint8 roundToUint8(float value)
	{
		if (value >= 255)
			return (juce::uint8)255;

		if (value <= 0)
			return (juce::uint8)(0);

		juce::uint8 rounded = (juce::uint8)value;
		float delta = value - (float)rounded;

		if (delta >= 0.5f)
			rounded += 1;

		return rounded;
	}
}

struct LumatoneKeyCoord
{
	LumatoneKeyCoord(int boardIndexIn = -1, int keyIndexIn = -1)
		: boardIndex(boardIndexIn)
		, keyIndex(keyIndexIn) {}

	int boardIndex = -1;
	int keyIndex = -1;

	bool isInitialized() const { return boardIndex >= 0 && keyIndex >= 0; }

	bool operator==(const LumatoneKeyCoord& coord) const { return keyIndex == coord.keyIndex && boardIndex == coord.boardIndex; }
	bool operator!=(const LumatoneKeyCoord& coord) const { return keyIndex != coord.keyIndex || boardIndex != coord.boardIndex; }

	static juce::String toString(int board, int key) { return juce::String(board) + "," + juce::String(key); }
	juce::String toString() { return LumatoneKeyCoord::toString(boardIndex, keyIndex); }
};

