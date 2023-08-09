/*
  ==============================================================================

    colour_model.cpp
    Created: 9 Aug 2023 12:34:31am
    Author:  Vincenzo

  ==============================================================================
*/

#include "colour_model.h"

LumatoneColourModel::LumatoneColourModel()
{
    parseTable();
}

LumatoneColourModel::~LumatoneColourModel()
{

}

void LumatoneColourModel::readTable(const juce::var& tableVar, ColourTable& table)
{
    auto matrix1 = tableVar.getArray();
    for (int i = 0; i < increment; i++)
    {
        auto matrix2 = matrix1->getUnchecked(i).getArray();
        for (int j = 0; j < increment; j++)
        {
            auto matrix3 = matrix2->getUnchecked(j).getArray();
            for (int k = 0; k < increment; k++)
            {
                auto vec = matrix3->getUnchecked(k).getArray();
                for (int c = 0; c < 3; c++)
                {
                    raw[i][j][k][c] = (float)vec->getUnchecked(c);
                }
            }
        }
    }
}

void LumatoneColourModel::parseTable()
{
    juce::var dataVar;
    auto result = juce::JSON::parse(BinaryData::model_json, dataVar);
    auto table = dataVar.getDynamicObject();

    auto names = table->getProperties();

    if (names.contains("increment"))
        increment = (int)table->getProperty("increment");

    if (names.contains("raw"))
        readTable(table->getProperty("raw"), raw);

    if (names.contains("adjusted"))
        readTable(table->getProperty("adjusted"), adjusted);

    return;
}
