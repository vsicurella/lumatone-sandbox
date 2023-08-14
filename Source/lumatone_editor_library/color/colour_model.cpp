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
    cache.reset(new juce::HashMap<LumatoneEditor::ColourHash, juce::Colour>(280));
    parseTable();
}

LumatoneColourModel::~LumatoneColourModel()
{
    cache = nullptr;
}

juce::Colour LumatoneColourModel::getModelColour(juce::Colour colour)
{
    if (colour.isTransparent())
        return juce::Colour();

    LumatoneEditor::ColourHash hash = LumatoneEditor::getColourHash(colour);
    auto cached = (*cache)[hash];
    if (cached.isOpaque())
        return cached;

    auto modelColour = calculateModelColour(LumatoneColourModel::Type::ADJUSTED, colour);
    cache->set(hash, modelColour);
    return modelColour;
}

// LumatoneColour LumatoneColourModel::getLumatoneColour(juce::Colour colour)
// {
//     auto modelColour = getModelColour(colour);
//     return LumatoneColour(colour, modelColour);
// }

juce::Colour LumatoneColourModel::calculateModelColour(LumatoneColourModel::Type type, const juce::Colour& colour)
{
    float shadowMakeupLightness = 1.12f;

    // return colour.withMultipliedLightness(shadowMakeupLightness);

    // auto params = getInterpolationParams(type, colour);
    auto params = getInterpolationParams(type, colour);
    
    float r = Interpolate::trilinear(params.red) / 255.0f;
    float g = Interpolate::trilinear(params.green) / 255.0f;
    float b = Interpolate::trilinear(params.blue) / 255.0f;

    float L = 1.0f;
    // float L =  0.2126f*r + 0.7152f*g + 0.0722f*b;
    // float L = colour.getPerceivedBrightness();
    // float L =  sqrtf(0.299*powf(r,2) + 0.587f*powf(g, 2) + 0.114f*powf(b,2));

    // float fr = 1.0f / r;
    // float fg = 1.0f / g;
    // float fb = 1.0f / b;

    // float f = fr;
    // if (f > fg)
    //     f = fg;
    // if (f > fb)
    //     f = fb;

    // // f *= 1.12f;

    // r *= f;
    // b *= f;
    // g *= f;

    // auto hsvc = juce::Colour(
    //     roundToUint8(r * 255.0f),
    //     roundToUint8(g * 255.0f),
    //     roundToUint8(b * 255.0f));

    // auto sat = hsvc.getSaturation();

    // return hsvc.withSaturation(1.0f).withAlpha(sat);

    auto result = juce::Colour(
        roundToUint8(r * 255.0f),
        roundToUint8(g * 255.0f),
        roundToUint8(b * 255.0f),
        L * 0.88f
    );

    // if (result.getPerceivedBrightness() < 0.8f)
        return result.withMultipliedLightness(shadowMakeupLightness);//.withSaturationHSL(1.0f);
    // return result;
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
                    table[i][j][k][c] = (int)vec->getUnchecked(c);
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

LumatoneColourModel::TrilinearInterpolationParams LumatoneColourModel::getInterpolationParams(LumatoneColourModel::Type type, const juce::Colour& colour)
{
    LumatoneColourModel::ColourTable* table;
    switch (type)
    {
    case LumatoneColourModel::Type::RAW:
        table = &raw;
        break;
    case LumatoneColourModel::Type::ADJUSTED:
    default:
        table = &adjusted;
        break;
    }

    float ri = colour.getFloatRed() * (increment - 1);
    int r0 = ri;
    int r1 = r0 + 1;

    float gi = colour.getFloatGreen() * (increment - 1);
    int g0 = gi;
    int g1 = g0 + 1;

    float bi = colour.getFloatBlue() * (increment - 1);
    int b0 = bi;
    int b1 = b0 + 1;

    uint8* n000 = (*table)[r0][g0][b0];
    uint8* n100 = (*table)[r1][g0][b0];
    uint8* n010 = (*table)[r0][g1][b0];
    uint8* n110 = (*table)[r1][g1][b0];
    uint8* n001 = (*table)[r0][g0][b1];
    uint8* n101 = (*table)[r1][g0][b1];
    uint8* n011 = (*table)[r0][g1][b1];
    uint8* n111 = (*table)[r1][g1][b1];

    float rt = ri - r0;
    float gt = gi - g0;
    float bt = bi - b0;

    auto redParams = Interpolate::TrilinearParams(
        n000[0], n100[0], n010[0], n110[0], 
        n001[0], n101[0], n011[0], n111[0],
        rt, gt, bt
    );
    auto greenParams = Interpolate::TrilinearParams(
        n000[1], n100[1], n010[1], n110[1], 
        n001[1], n101[1], n011[1], n111[1],
       rt, gt, bt
    );
    auto blueParams = Interpolate::TrilinearParams(
        n000[2], n100[2], n010[2], n110[2], 
        n001[2], n101[2], n011[2], n111[2],
        rt, gt, bt
    );

    return LumatoneColourModel::TrilinearInterpolationParams(redParams, greenParams, blueParams);
}

LumatoneColourModel::uint8 LumatoneColourModel::roundToUint8(float value)
{
    if (value >= 255)
        return (uint8)255;

    if (value <= 0)
        return (uint8)(0);

    uint8 rounded = (uint8)value;
    float delta = value - (float)rounded;

    if (delta >= 0.5f)
        rounded += 1;

    return rounded;
}
