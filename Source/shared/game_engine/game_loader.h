/*

    File: game_loader.h
    Author: Vito
    Date: 2023/11/08

*/

#ifndef LUMATONE_GAME_LOADER_H
#define LUMATONE_GAME_LOADER_H

#include <JuceHeader.h>

class LumatoneSandboxGameBase;
class LumatoneController;

class LumatoneSandboxGameLoader
{
public:

    static LumatoneSandboxGameBase* CreateGameInstance(juce::String gameId, juce::ValueTree gameEngineState, LumatoneController* controller);
};

#endif