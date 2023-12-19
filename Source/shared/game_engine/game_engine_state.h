/*

    File: game_engine_state.h
    Author: Vito
    Date: 2023/11/07

*/

#ifndef LUMATONE_GAME_ENGINE_STATE_H
#define LUMATONE_GAME_ENGINE_STATE_H

#include "../games/games_index.h"
#include "../lumatone_editor_library/data/lumatone_state.h"

class LumatoneGameBaseState;
class LumatoneGameEngineState : protected LumatoneStateBase
{
public:
    struct ID
    {
        inline static const juce::Identifier GameEngineStateId = juce::Identifier("GameEngineState");
        inline static const juce::Identifier DefaultFps = juce::Identifier("DefaultFps");
        inline static const juce::Identifier RequestedFps = juce::Identifier("RequestedFps");
        
        inline static const juce::Identifier GameStateId = juce::Identifier("GameStateId");
        inline static const juce::Identifier GameStatus = juce::Identifier("GameStatus");
        inline static const juce::Identifier GameName = juce::Identifier("GameName");
        inline static const juce::Identifier LogLevel = juce::Identifier("GameEngineLogLevel");
    };

    enum class GameStatus
    {
        NoGame,
        // NotLoaded,
        Loaded,
        Running,
        Paused,
        Stopped
    };
    
    static juce::String GameStatusToString(LumatoneGameEngineState::GameStatus status)
    {
        switch (status)
        {
        // case LumatoneGameEngineState::GameStatus::NotLoaded:
        //     return juce::String("NotLoaded");
        case LumatoneGameEngineState::GameStatus::Loaded:
            return juce::String("Loaded");
        case LumatoneGameEngineState::GameStatus::Running:
            return juce::String("Running");
        case LumatoneGameEngineState::GameStatus::Paused:
            return juce::String("Paused");
        case LumatoneGameEngineState::GameStatus::Stopped:
            return juce::String("Stopped");

        default:
            return juce::String("NoGame");
        }
    }

    static LumatoneGameEngineState::GameStatus GameStatusFromString(juce::String status)
    {
        // if (status == "NotLoaded")
        //     return LumatoneGameEngineState::GameStatus::NotLoaded;
        if (status == "Loaded")
            return LumatoneGameEngineState::GameStatus::Loaded;
        if (status == "Running")
            return LumatoneGameEngineState::GameStatus::Running;
        if (status == "Paused")
            return LumatoneGameEngineState::GameStatus::Paused;
        if (status == "Stopped")
            return LumatoneGameEngineState::GameStatus::Stopped;
        
        return LumatoneGameEngineState::GameStatus::NoGame;
    }

public:
    static juce::Array<juce::Identifier> GetGameEngineProperties();

public:
    LumatoneGameEngineState(juce::String nameIn, juce::ValueTree stateIn, juce::UndoManager* undoManager);
    LumatoneGameEngineState(juce::String nameIn, const LumatoneGameEngineState& stateIn);

    virtual ~LumatoneGameEngineState() {}

    float getTimeIntervalMs() const;
    double getFps() const;

    bool operator==(const juce::ValueTree& compare) const;
    bool operator!=(const juce::ValueTree& compare) const;

    void addStateListener(LumatoneStateBase* stateIn);

public:
    void setDefaultFps(double fps);
private:
    void setDefaultFps(double fps, bool writeToState);

public:
    virtual void forceFps(double fps);
private:
    virtual void forceFps(double fps, bool writeToState);

public:
    // juce::ValueTree getGameStateTree();

    bool isGameLoaded() const { return gameStatus > GameStatus::NoGame; }
    bool isGameRunning() const { return gameStatus == GameStatus::Running; }
    bool isGamePaused() const { return gameStatus == GameStatus::Paused; }


public:
    static float bpmToMsec(float bpmValue) { return 6.0e4f / bpmValue; }
    static float msecToBpm(float msValue) { return 6.0e4f / msValue; }

    float msecToTicks(float msec) const { return msec / static_cast<float>(getTimeIntervalMs()); }
    float ticksToMsec(float ticks) const { return getTimeIntervalMs() * ticks; }

    float bpmToGameTicks(float bpmValue) const { return msecToTicks(bpmToMsec(bpmValue)); }
    float ticksToBpm(float ticksValue) const { return msecToBpm(ticksToMsec(ticksValue)); }

protected:
    void setGameState(LumatoneGameBaseState* gameStateIn);

protected:
    void updateTimeIntervalMs();

    virtual void setGameStatus(LumatoneGameEngineState::GameStatus newState, bool writeToState);
    void setGameName(LumatoneSandbox::GameName gameNameIn, bool writeToState);

protected:    
    virtual juce::ValueTree loadStateProperties(juce::ValueTree stateIn) override;
    virtual void handleStatePropertyChange(juce::ValueTree stateIn, const juce::Identifier& property) override;

protected:
    double defaultFps = 30;
    double runGameFps = 30;
    float timeIntervalMs = 0;

    LumatoneSandbox::GameName               gameName    = LumatoneSandbox::GameName::NoGame;
    LumatoneGameEngineState::GameStatus     gameStatus  = LumatoneGameEngineState::GameStatus::NoGame;
    
    bool sentFirstGameMessage = false;

    friend class LumatoneGameBaseState; // preferably shouldn't have to do this?
};

#endif // LUMATONE_GAME_ENGINE_STATE_H
