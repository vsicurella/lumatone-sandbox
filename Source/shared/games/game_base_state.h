/*

    File: game_base_state.h
    Author: Vito
    Date: 2023/11/07

*/

#ifndef LUMATONE_GAME_BASE_STATE_H
#define LUMATONE_GAME_BASE_STATE_H

#include "../game_engine/game_engine_state.h"

class LumatoneGameBaseState : protected LumatoneGameEngineState
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
    LumatoneGameBaseState(LumatoneSandbox::GameName name, juce::Identifier gameId, LumatoneGameEngineState& gameEngineStateIn);

    // juce::ValueTree getNode() const { return state; }

    int getNumTicks() const { return ticks; }

    LumatoneKeyContext getKeyAt(int boardIndex, int keyIndex) const;

    const LumatoneLayout& getLayoutBeforeStart() const { return layoutBeforeStart; }

protected:
    void updateSavedLayout();

public:
    LumatoneLayout getIdentityLayout(bool resetColors=true, juce::Colour boardColour={});
    LumatoneContext getIdentityWithLayoutContext(bool resetColors);

protected:
    virtual juce::ValueTree loadStateProperties(juce::ValueTree stateIn) override = 0;
    virtual void handleStatePropertyChange(juce::ValueTree stateIn, const juce::Identifier& property) override;

protected:
    juce::Identifier gameId;
    
    int ticks = 0;
    int maxUpdatesPerFrame = 30;

    LumatoneLayout layoutBeforeStart;
};

#endif
