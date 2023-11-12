/*

    File: game_base_state.h
    Author: Vito
    Date: 2023/11/07

*/

#ifndef LUMATONE_GAME_BASE_STATE_H
#define LUMATONE_GAME_BASE_STATE_H

#include "../game_engine/game_engine_state.h"

class LumatoneGameEngineState;

class LumatoneGameBaseState : protected LumatoneStateBase
{
public:
    struct ID
    {
        inline static const juce::Identifier Ticks = juce::Identifier("NumGameTicks");
        inline static const juce::Identifier MaxUpdatesPerFrame = juce::Identifier("MaxUpdatesPerFrame");
        inline static const juce::Identifier LogLevel = juce::Identifier("GameLogLevel");
    };

    static juce::Array<juce::Identifier> GetLumatoneGameBaseProperties();

public:
    LumatoneGameBaseState(LumatoneSandbox::GameName name, juce::Identifier gameId, juce::ValueTree engineStateIn);

protected:
    virtual void valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property) override;    
    
    virtual juce::ValueTree loadStateProperties(juce::ValueTree stateIn) override;
    virtual void handleStatePropertyChange(juce::ValueTree stateIn, const juce::Identifier& property) override;

protected:
    juce::Identifier gameId;
    
    juce::ValueTree engineStateTree;
    LumatoneGameEngineState engineState;

    int ticks = 0;
    int maxUpdatesPerFrame = 10;
};

#endif
