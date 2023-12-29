// Supply pitch/note, tuning, or remapped configs for a LumatoneLayout

#ifndef LUMATONE_LAYOUT_CONTEXT_H
#define LUMATONE_LAYOUT_CONTEXT_H

#include "./lumatone_layout.h"

struct LumatoneKeyContext : public MappedLumatoneKey
{
    LumatoneKeyContext() : MappedLumatoneKey() {}

    LumatoneKeyContext(const LumatoneKey& keyData, int boardIndex, int keyIndex) 
        : MappedLumatoneKey(keyData, boardIndex, keyIndex) {}

    LumatoneKeyContext(const MappedLumatoneKey& key) : MappedLumatoneKey(key) {}

    // TODO add more properties
};

class LumatoneContext : public LumatoneLayout
{
public:

    LumatoneContext(const LumatoneLayout& layout);
    LumatoneContext(const LumatoneContext& context);
    LumatoneContext(const LumatoneContext& context, const LumatoneLayout& layout);
    
    ~LumatoneContext();

    bool setMappedMidiChannels(juce::Array<int> midiChannelMap);
    void clearMappedMidiChannels();

    bool setMappedMidiNotes(juce::Array<int> midiNoteMap);
    void clearMappedMidiNotes();

    LumatoneKeyContext getKeyContext(int boardIndex, int keyIndex) const;
    LumatoneKeyContext getKeyContext(int keyNum) const;

private:

    bool midiChannelsMapped = false;
    juce::Array<int, juce::CriticalSection> mappedMidiChannels;

    bool midiNotesMapped = false;
    juce::Array<int, juce::CriticalSection> mappedMidiNotes;
};

#endif LUMATONE_LAYOUT_CONTEXT_H
