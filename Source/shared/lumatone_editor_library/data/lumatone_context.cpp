#include "lumatone_context.h"

LumatoneContext::LumatoneContext(const LumatoneLayout &layout)
    : LumatoneLayout(layout)
{

}

LumatoneContext::LumatoneContext(const LumatoneContext &context)
    : LumatoneContext(context, (const LumatoneLayout&)context)
{

}

LumatoneContext::LumatoneContext(const LumatoneContext &context, const LumatoneLayout &layout)
    : LumatoneLayout(layout)
    , midiChannelsMapped(context.midiChannelsMapped)
    , mappedMidiChannels(context.mappedMidiChannels)
    , midiNotesMapped(context.midiNotesMapped)
    , mappedMidiNotes(context.mappedMidiNotes)
{

}

LumatoneContext::~LumatoneContext()
{
}

bool LumatoneContext::setMappedMidiChannels(juce::Array<int> midiChannelMap)
{
    const int expectedSize = getNumBoards() * getOctaveBoardSize(); // TODO cache this

    if (midiChannelMap.size() != expectedSize)
        return false;

    juce::ScopedLock l(mappedMidiChannels.getLock());
    for (int i = 0; i < expectedSize; i++)
    {
        mappedMidiChannels.set(i, midiChannelMap[i]);
    }

    midiChannelsMapped = true;

    return true;
}

void LumatoneContext::clearMappedMidiChannels()
{
    juce::ScopedLock l(mappedMidiChannels.getLock());

    mappedMidiChannels.clear();
    midiChannelsMapped = false;
}

bool LumatoneContext::setMappedMidiNotes(juce::Array<int> midiNoteMap)
{
    const int expectedSize = getNumBoards() * getOctaveBoardSize(); // TODO cache this

    if (midiNoteMap.size() != expectedSize)
        return false;

    juce::ScopedLock l(mappedMidiNotes.getLock());
    for (int i = 0; i < expectedSize; i++)
    {
        mappedMidiNotes.set(i, midiNoteMap[i]);
    }

    midiNotesMapped = true;

    return true;
}

void LumatoneContext::clearMappedMidiNotes()
{
    juce::ScopedLock l(mappedMidiNotes.getLock());

    mappedMidiNotes.clear();
    midiNotesMapped = false;
}

LumatoneKeyContext LumatoneContext::getKeyContext(int boardIndex, int keyIndex) const
{
    auto coord = LumatoneKeyCoord(boardIndex, keyIndex);
    if (!isKeyCoordValid(coord))
        return LumatoneKeyContext();

    auto keyNum = keyCoordToKeyNum(coord);
    MappedLumatoneKey mappedKey = MappedLumatoneKey(getKey(boardIndex, keyIndex), coord);
    LumatoneKeyContext keyContext = LumatoneKeyContext(mappedKey);

    // TODO make a cache of this

    if (midiChannelsMapped)
        keyContext.setChannelNumber(mappedMidiChannels[keyNum]);

    if (midiNotesMapped)
        keyContext.setNoteOrCC(mappedMidiNotes[keyNum]);

    return keyContext;
}

LumatoneKeyContext LumatoneContext::getKeyContext(int keyNum) const
{
    auto coord = keyNumToKeyCoord(keyNum);
    return getKeyContext(coord.boardIndex, coord.keyIndex);
}
