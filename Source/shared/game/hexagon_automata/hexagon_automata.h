#pragma once

#include "../game_base.h"

#include "../../lumatone_editor_library/hex/lumatone_hex_map.h"
#include "../../lumatone_editor_library/color/adjust_layout_colour.h"


namespace HexagonAutomata
{
    struct HexState
    {
       float health;
       int age;

       juce::Colour colour;

        HexState()
            : health(0.0f), age(0), colour(juce::Colours::white) {}
        HexState(float healthIn, int ageIn=0, juce::Colour colourIn=juce::Colours::white)
            : health(healthIn), age(ageIn), colour(colourIn) {}

        void applyFactor(float lifeFactor) { health *= lifeFactor; }
        
        bool isAlive() const { return health > 0.0f; }

        bool isDead() const { return health <= 0.0f && age > 0; }
        
        bool isEmpty() const { return health <= 0.0f && age == 0; }

        void setBorn() { health = 1.0f; age = 0; }

        void setDead() { health = 0.0f; }

        void setEmpty() { health = 0.0f; age = 0; }
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
        juce::Array<int> numsBorn;
        juce::Array<int> numsSurvive;
        
        BornSurviveRule(int numBorn, int surviveLower, int surviveUpper)
        {
            numsBorn.add(numBorn);
            numsSurvive.add(surviveLower);
            numsSurvive.add(surviveUpper);
        }

        BornSurviveRule(juce::Array<int> bornNums, juce::Array<int> surviveNums)
            : numsBorn(bornNums)
            , numsSurvive(surviveNums) {}

        BornSurviveRule(juce::String bornString, juce::String surviveString)
        {
            numsBorn = ParseListArgument(bornString);
            numsSurvive = ParseListArgument(surviveString);
        }

        static juce::Array<int> ParseListArgument(juce::String numberList)
        {
            juce::Array<int> list;
            juce::String dbgStr;

            auto tks = juce::StringArray::fromTokens(numberList, juce::String(","), juce::String());
            for (auto tk : tks)
            { 
                if (tk.trim().isNotEmpty())
                {
                    auto num = tk.getIntValue();
                    if (num > 0 && num < 280)
                    {
                        list.addIfNotAlreadyThere(num);
                        dbgStr += juce::String(num) + ",";
                    }
                }
            }

            DBG("BornSurviveRule::ParseListArgument: " + dbgStr);

            return list;
        }

        virtual float getLifeFactor(const MappedHexState& origin, const MappedHexState* neighbors, int numNeighbors) const override
        {
            if (numsSurvive.contains(numNeighbors))
                return 1.0f;
            return 0.0f;
        }

        virtual bool generateNewLife(const MappedHexState& origin, const MappedHexState* neighbors, int numNeighbors) const override
        {
            if (numsBorn.contains(numNeighbors))
                return true;
            return false;
        }
    };

    enum class GameMode
    {
        Classic = 0,
        Sequencer
    };

    struct GameState
    {
        std::shared_ptr<LumatoneLayout> layout;
        LumatoneHexMap hexMap;

        int numCells = 0;

        juce::Array<HexState> cells;

        GameState(std::shared_ptr<LumatoneLayout> layoutIn)
            : layout(layoutIn)
            , hexMap(layoutIn) 
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

        MappedHexState getMappedCell(int cellNum);

        virtual juce::Array<MappedHexState> getNeighbors(Hex::Point cellCoord, const juce::Array<Hex::Point>& vector) const;
        virtual juce::Array<MappedHexState> getAliveNeighbors(Hex::Point cellCoord, const juce::Array<Hex::Point>& vector) const;
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

        const float aliveScalar = 1.0f;
        const float deadScalar = 0.5f;
        const float emptyScalar = 0.25f;

        Renderer(juce::Colour aliveColourIn=juce::Colours::white, juce::Colour deadColourIn=juce::Colours::grey)
        {
            setColour(aliveColourIn, deadColourIn);
        }

        void setColour(juce::Colour aliveColourIn, juce::Colour deadColourIn=juce::Colours::transparentBlack)
        {
            aliveColour = aliveColourIn;
            oldColour = aliveColour.contrasting(1.0f);

            if (deadColourIn != juce::Colours::transparentBlack)
                deadColour = deadColourIn;

            ageGradient = juce::ColourGradient(aliveColour, 0.0f, 0.0f,
                oldColour, 1.0f, 1.0f, false);
        }

        juce::Colour getAliveColour() const { return aliveColour; }

        virtual juce::Colour renderAliveColour(const MappedHexState& state)
        {
            if (state.HexState::isEmpty())
                return emptyColour;
            if (state.isAlive())
                return state.HexState::colour;
            return deadColour;
        }

        virtual juce::Colour renderGradientColour(const MappedHexState& state)
        {
            if (state.HexState::isEmpty())
                return emptyColour;

            if (state.isDead())
                return deadColour;

            auto ageFactor = (double)state.age / (double)maxAge;
            auto colour = state.HexState::colour;

            // if (ageFactor <= 1.0f)
            //     colour = ageGradient.getColourAtPosition(ageFactor);



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

        virtual MappedLumatoneKey renderSequencerKey(const MappedHexState& cell, const LumatoneLayout& noteLayout)
        {
            auto key = (MappedLumatoneKey)cell;

            auto noteKey = noteLayout.readKey(cell.boardIndex, cell.keyIndex);
            key.colour = noteKey->colour;

            if (cell.isAlive())
                AdjustLayoutColour::multiplyBrightness(aliveScalar, key);
            // else if (cell.isDead())
            //     AdjustLayoutColour::multiplyBrightness(deadScalar, key);
            else
                AdjustLayoutColour::multiplyBrightness(emptyScalar, key);

            return key;
        }

        virtual juce::Colour renderNewbornColour(const juce::Array<MappedHexState>& parents)
        {
            if (parents.size() == 0)
                return juce::Colours::white;
                
            float hue = 0;
            float saturation = 0;
            float value = 0;

            float _hue;

            for (auto cell : parents)
            {
                auto colour = cell.HexState::colour;

                hue += colour.getHue();
                saturation += colour.getSaturation();
                value += colour.getBrightness();
            }

            return juce::Colour(hue - (int)hue, saturation / parents.size(), value / parents.size(), (juce::uint8)0xff);
            // return parents[0].HexState::colour;
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

        LumatoneAction* renderFrame() override;

        void rerenderState();

        double getLockedFps() const { return 0; }

public:
        void addSeed(Hex::Point coord, bool triggerMidi=true);
        void addSeeds(juce::Array<Hex::Point> seedCoords, bool triggerMidi=true);
        void addSeeds(int numSeeds, bool triggerMidi=true);
        
        void updateNextGeneration();
        void updateGenerationAge(); 

        void setAliveColour(juce::Colour newColour);
        juce::Colour getAliveColour() const;

        void setDeadColour(juce::Colour newColour);
        juce::Colour getDeadColour() const;

        void setBornSurviveRules(juce::Array<int> bornNums, juce::Array<int> surviveNums);
        void setBornSurviveRules(juce::String bornInput, juce::String surviveInput);
        
        void setNeighborDistance(int distance);
        

    private:

        void handleAnyNoteOn(int midiChannel, int midiNote, juce::uint8 velocity) override;

        void completeMappingLoaded(LumatoneLayout layout);

    private:

        // Apply cell updates to game state
        // Returns whether or not cell is still populated
        bool applyUpdatedCell(const MappedHexState& cellUpdate);

        // Produce midi note from cell and send immediately
        // Returns whether or not cell can be triggered
        bool triggerCellMidi(const MappedHexState& cell);

    private:

        void resetState() override;

        void initialize();
        void redoCensus();

        void initializeLayoutContext();

    private:

        juce::CriticalSection lock;

        std::unique_ptr<NeighborFunction> rules;

        juce::Array<Hex::Point> neighborsVector;

        GameMode mode;

        int ticks = 0;

        int ticksPerGeneration = 10;
        int ticksToNextGeneration = 0;

        int verbose = 0;
        
        // int ticksPerAge = 0;
        // int ticksToNextAge = 0;

        juce::Array<MappedHexState> populatedCells;
        // juce::Array<MappedHexState, juce::CriticalSection> updatedCells;

        juce::Array<MappedHexState> newCells;
        juce::Array<MappedHexState> bornCells;
        juce::Array<MappedHexState> diedCells;
        juce::Array<MappedHexState> agingCells;

        std::unique_ptr<Renderer> render;

        juce::Random random;
    };
}
