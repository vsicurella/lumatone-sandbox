/*

    File: game_base_state.h
    Author: Vito
    Date: 2023/11/07

*/

#include "game_base_state.h"

juce::Array<juce::Identifier> LumatoneGameBaseState::GetLumatoneGameBaseProperties()
{
    juce::Array<juce::Identifier> properties;
    properties.add(LumatoneGameBaseState::ID::Ticks);
    properties.add(LumatoneGameBaseState::ID::MaxUpdatesPerFrame);
    properties.add(LumatoneGameBaseState::ID::LogLevel);
    return properties;
}

LumatoneGameBaseState::LumatoneGameBaseState(LumatoneSandbox::GameName nameIn, juce::Identifier gameIdIn, LumatoneGameEngineState& gameEngineStateIn)
    : LumatoneGameEngineState(LumatoneSandbox::GameNameToString(nameIn), gameEngineStateIn)
    , gameId(gameIdIn)
    , layoutBeforeStart(*gameEngineStateIn.getMappingData())
{
    state = juce::ValueTree(LumatoneGameEngineState::ID::GameStateId);
    setStateProperty(LumatoneGameEngineState::ID::GameName, name);
    
    // engineState.addStateListener(this);
    addStateListener(this);
}

void LumatoneGameBaseState::handleStatePropertyChange(juce::ValueTree stateIn, const juce::Identifier &property)
{
    // if (engineState == stateIn)
    // {
        LumatoneGameEngineState::handleStatePropertyChange(stateIn, property);
    // }
}


LumatoneLayout LumatoneGameBaseState::getIdentityLayout(bool resetColors, juce::Colour boardColour)
{
    LumatoneLayout identity = LumatoneLayout::IdentityMapping(getNumBoards(), getOctaveBoardSize());

    for (int b = 0; b < layoutBeforeStart.getNumBoards(); b++)
    {
        for (int k = 0; k < layoutBeforeStart.getOctaveBoardSize(); k++)
        {
            if (resetColors)
            {
                identity.setKeyColour(boardColour, b, k);
            }
            else
            {
                auto layoutKey = layoutBeforeStart.getKey(b, k);
                identity.setKeyColour(layoutKey.getColour(), b, k);
            }
        }
    }

    return identity;
}

LumatoneContext LumatoneGameBaseState::getIdentityWithLayoutContext(bool resetColors)
{
    LumatoneLayout identityLayout = getIdentityLayout(resetColors);
    LumatoneContext layoutContext = LumatoneContext(layoutBeforeStart);

    juce::Array<int> midiChannelContextMap;
    juce::Array<int> midiNoteContextMap;

    for (int b = 0; b < layoutBeforeStart.getNumBoards(); b++)
    {
        for (int k = 0; k < layoutBeforeStart.getOctaveBoardSize(); k++)
        {
            auto layoutKey = layoutBeforeStart.getKey(b, k);
            midiChannelContextMap.add(layoutKey.getMidiChannel());
            midiNoteContextMap.add(layoutKey.getMidiNumber());
        }
    }

    LumatoneContext context = LumatoneContext(identityLayout);
    context.setMappedMidiChannels(midiChannelContextMap);
    context.setMappedMidiNotes(midiNoteContextMap);

    return context;
}

void LumatoneGameBaseState::updateSavedLayout()
{
    layoutBeforeStart = *getMappingData();
}

LumatoneKeyContext LumatoneGameBaseState::getKeyAt(int boardIndex, int keyIndex) const
{
    return getKeyContext(boardIndex, keyIndex);
}

