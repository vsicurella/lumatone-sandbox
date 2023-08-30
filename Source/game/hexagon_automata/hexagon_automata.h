#pragma once

#include "../game_base.h"
#include "../../lumatone_editor_library/hex/lumatone_hex_map.h"

namespace HexagonAutomata
{
    struct HexState
    {
       float health;
       int age;

        HexState()
            : health(0.0f), age(0) {}
        HexState(float healthIn, int ageIn=0)
            : health(healthIn), age(ageIn) {}

        void applyFactor(float lifeFactor) { health *= lifeFactor; }
        
        bool isAlive() const { return health > 0.0f; }

        bool isDead() const { return health <= 0.0f && age > 0; }
        
        bool isEmpty() const { return health <= 0.0f && age == 0; }

        void setBorn() { health = 1.0f; }
    };


    struct MappedHexState   : public HexState
                            , public MappedLumatoneKey
                            , public Hex::Point
    {
        MappedHexState()
            : HexState()
            , MappedLumatoneKey(LumatoneKey(), LumatoneKeyCoord())
            , Hex::Point(0, 0)
        {}
        
        MappedHexState(const HexState& stateIn, const MappedLumatoneKey& key, Hex::Point hexCoord)
            : HexState(stateIn)
            , MappedLumatoneKey(key)
            , Hex::Point(hexCoord) {}
    };

    struct NeighborFunction 
    {
        virtual juce::Array<Hex::Point> getNeighborsVector(int distance=1) const { return Hex::Point().neighbors(distance); }

        virtual float getLifeFactor(const MappedHexState& origin, const MappedHexState* neighbors, int numNeighbors) const = 0;
        virtual bool generateNewLife(const MappedHexState& origin, const MappedHexState* neighbors, int numNeighbors) const = 0;
    };

    struct DefaultNeighborFunction : public NeighborFunction
    {
        virtual float getLifeFactor(const MappedHexState& origin, const MappedHexState* neighbors, int numNeighbors) const override
        {
            if (numNeighbors < 2 || numNeighbors > 3)
                return 0.0f;
            return 1.0f;
        }

        virtual bool generateNewLife(const MappedHexState& origin, const MappedHexState* neighbors, int numNeighbors) const override
        {
            if (numNeighbors == 3)
                return true;
            return false;
        }
    };

    struct BornSurviveRule : public NeighborFunction
    {
        int numForBorn = 0;
        int numLowSurvive = 0;
        int numHighSurvive = 0;

        BornSurviveRule(int numBorn, int surviveLower, int surviveUpper)
            : numForBorn(numBorn)
            , numLowSurvive(surviveLower)
            , numHighSurvive(surviveUpper) {}
        
        virtual float getLifeFactor(const MappedHexState& origin, const MappedHexState* neighbors, int numNeighbors) const override
        {
            if (numNeighbors < numLowSurvive || numNeighbors > numHighSurvive)
                return 0.0f;
            return 1.0f;
        }

        virtual bool generateNewLife(const MappedHexState& origin, const MappedHexState* neighbors, int numNeighbors) const override
        {
            if (numNeighbors == numForBorn)
                return true;
            return false;
        }
    };
    
    struct GameState
    {
        std::shared_ptr<LumatoneLayout> layout;
        LumatoneHexMap hexMap;

        int numCells = 0;

        juce::Array<HexState> cells;

        GameState(std::shared_ptr<LumatoneLayout> layoutIn)
            : layout(layoutIn)
            , hexMap(LumatoneHexMap(layoutIn)) 
        {
            resetState();
        }

        GameState(const GameState& copy)
            : layout(copy.layout)
            , hexMap(copy.layout)
            , numCells(copy.numCells)
            , cells(copy.cells) {}

        virtual void resetState()
        {
            numCells = layout->getOctaveBoardSize() * layout->getNumBoards();

            cells.resize(numCells);
            cells.fill(HexState());
        }

        virtual juce::Array<MappedHexState> getNeighbors(const MappedHexState& cell, const juce::Array<Hex::Point>& vector) const;
        virtual juce::Array<MappedHexState> getAliveNeighbors(const MappedHexState& cell, const juce::Array<Hex::Point>& vector) const;
    };

    class Renderer
    {
    public:
        juce::Colour aliveColour;
        juce::Colour deadColour;
        juce::Colour emptyColour = juce::Colours::black;

        juce::Colour oldColour;
        juce::Colour ageColour;
        int maxAge = 20;

    private:
        juce::ColourGradient healthGradient;
        juce::ColourGradient ageGradient;

    public:

        Renderer(juce::Colour aliveColourIn=juce::Colours::white, juce::Colour deadColourIn=juce::Colours::grey)
        {
            setColour(aliveColourIn, deadColourIn);
        }

        void setColour(juce::Colour aliveColourIn, juce::Colour emptyColourIn=juce::Colours::transparentBlack)
        {
            aliveColour = aliveColourIn;
            oldColour = aliveColour.contrasting(1.0f);

            if (emptyColourIn != juce::Colours::transparentBlack)
                deadColour = emptyColourIn;

            ageGradient = juce::ColourGradient(aliveColour, 0.0f, 0.0f,
                oldColour, 1.0f, 1.0f, false);
        }

        virtual juce::Colour renderAliveColour(const MappedHexState& state)
        {
            if (state.HexState::isEmpty())
                return emptyColour;
            if (state.isAlive())
                return aliveColour;
            return deadColour;
        }

        virtual juce::Colour renderGradientColour(const MappedHexState& state)
        {
            if (state.HexState::isEmpty())
                return emptyColour;

            if (state.isDead())
                return deadColour;

            auto ageFactor = (double)state.age / (double)maxAge;
            auto colour = oldColour;

            if (ageFactor <= 1.0f)
                colour = ageGradient.getColourAtPosition(ageFactor);

            healthGradient = juce::ColourGradient(deadColour, 0.0f, 0.0f,
                                                  colour, 1.0f, 1.0f, false);
            return healthGradient.getColourAtPosition(state.health);
        }

        virtual juce::Colour renderCellColour(const MappedHexState& state)
        {
            return renderAliveColour(state);
        }

        virtual MappedLumatoneKey renderCellKey(const MappedHexState& state)
        {
            auto key = (MappedLumatoneKey)state;
            key.colour = renderCellColour(state);
            return key;
        }
    };
    

    class Game : public LumatoneSandboxGameBase
               , private HexagonAutomata::GameState
    {
    public:

        Game(LumatoneController* controller);
        Game(LumatoneController* controller, const HexagonAutomata::GameState& stateIn);
        
        void reset(bool clearQueue) override;
        
        void nextTick() override;
        void pauseTick() override;

        void setTicksPerGeneration(int ticks);

        juce::UndoableAction* renderFrame() override;

        double getLockedFps() const { return 0; }


        void addSeed(Hex::Point coord);
        void addSeeds(juce::Array<Hex::Point> seedCoords);
        void addSeeds(int numSeeds);
        
        void updateNextGeneration();
        void updateGenerationAge(); 

        void setAliveColour(juce::Colour newColour);

        void rerenderState();

    private:

        void handleNoteOn(LumatoneMidiState* midiState, int midiChannel, int midiNote, juce::uint8 velocity) override;

        // Apply cell updates to game state
        // Returns whether or not cell is still populated
        bool applyUpdatedCell(const MappedHexState& cellUpdate);

    private:

        void resetState() override;

        void initialize();
        void redoCensus();

    private:

        juce::CriticalSection lock;

        std::unique_ptr<NeighborFunction> rules;

        juce::Array<Hex::Point> neighborsVector;

        int ticks = 0;

        int ticksPerGeneration = 10;
        int ticksToNextGeneration = 0;

        int verbose = 0;
        
        // int ticksPerAge = 0;
        // int ticksToNextAge = 0;

        juce::Array<MappedHexState> populatedCells;
        // juce::Array<MappedHexState, juce::CriticalSection> updatedCells;

        juce::Array<MappedHexState> bornCells;
        juce::Array<MappedHexState> diedCells;
        juce::Array<MappedHexState> agingCells;

        std::unique_ptr<Renderer> render;

        juce::Random random;
    };
}

