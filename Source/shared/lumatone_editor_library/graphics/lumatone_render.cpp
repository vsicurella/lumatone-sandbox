#include "lumatone_render.h"
#include "../color/colour_model.h"

LumatoneRender::LumatoneRender(const LumatoneApplicationState& stateIn)
    : LumatoneApplicationState("LumatoneRender", stateIn)
{
    imageProcessor.reset(new ImageProcessor());
}

LumatoneRender::~LumatoneRender()
{
    imageProcessor = nullptr;
}

void LumatoneRender::resetOctaveSize()
{
    lumatoneGeometry = LumatoneGeometry(GetLumatoneBoardSize(getOctaveBoardSize()));

    // tilingGeometry.setColumnAngle(LUMATONEGRAPHICCOLUMNANGLE);
    // tilingGeometry.setRowAngle(LUMATONEGRAPHICROWANGLE);

    oct1Key1 = juce::Point<float>(oct1Key1X, oct1Key1Y);
    oct1Key56 = juce::Point<float>(oct1Key56X, oct1Key56Y);
    oct5Key7 = juce::Point<float>(oct5Key7X, oct5Key7Y);

    tilingGeometry.fitSkewedTiling(oct1Key1, oct1Key56, 10, oct5Key7, 24, true);
    keyCentres = tilingGeometry.getHexagonCentresSkewed(lumatoneGeometry, 0, getNumBoards());
}

juce::Array<juce::Point<float>> LumatoneRender::getKeyCentres()
{
    return tilingGeometry.getHexagonCentresSkewed(lumatoneGeometry, 0, getNumBoards());
}

void LumatoneRender::render(LumatoneAssets::LumatoneGraphicRenderSize maxRenderSize)
{
    int width = LumatoneAssets::LumatoneKeyboardRenderWidth(maxRenderSize);
    int height = LumatoneAssets::LumatoneKeyboardRenderHeight(maxRenderSize);

    int keyWidth = juce::roundToInt(width * keyW);
    int keyHeight = juce::roundToInt(height * keyH);

    juce::Image renderImage = juce::Image(juce::Image::PixelFormat::ARGB, width, height, true);
    juce::Graphics g(renderImage);

    g.drawImageAt(LumatoneAssets::getImage(LumatoneAssets::ID::LumatoneGraphic, height, width), 0, 0);

    auto shapeLayer = LumatoneAssets::getImage(LumatoneAssets::ID::KeyShape, keyHeight, keyWidth);
    auto shadowLayer = LumatoneAssets::getImage(LumatoneAssets::ID::KeyShadow, keyHeight, keyWidth);

    LumatoneColourModel* colourModel = getColourModel();

    int keyNum = 0;
    for (int boardIndex = 0; boardIndex < getNumBoards(); boardIndex++)
    {
        for (int keyIndex = 0; keyIndex < getOctaveBoardSize(); keyIndex++)
        {
            juce::Colour keyColour = getKey(boardIndex, keyIndex).getColour();
            keyColour = colourModel->getModelColour(keyColour);

            juce::Point<int> keyPos = juce::Point<int>(
                juce::roundToInt(keyCentres[keyNum].x * width - keyWidth * 0.5f),
                juce::roundToInt(keyCentres[keyNum].y * height - keyHeight * 0.5f)
            );

            if (!keyColour.isTransparent())
            {
                g.setColour(keyColour);
                g.drawImageAt(shapeLayer, keyPos.x, keyPos.y, true);
            }

            g.drawImageAt(shadowLayer, keyPos.x, keyPos.y);

            keyNum++;
        }
    }

    // Add rescaled renders to cache

    for (int renderSize = 0; renderSize <= (int)LumatoneAssets::LumatoneGraphicRenderSize::_8x; renderSize++)
    {
        auto size = (LumatoneAssets::LumatoneGraphicRenderSize)renderSize;
        int renderWidth = LumatoneAssets::LumatoneKeyboardRenderWidth(size);
        int renderHeight = LumatoneAssets::LumatoneKeyboardRenderHeight(size);

        juce::Image resized;
        if (renderHeight == height && renderWidth == width)
        {
            resized = renderImage;
        }
        else
        {
            resized = imageProcessor->resizeImage(renderImage, renderWidth, renderHeight);
        }

        renders.set((int)size, resized);
    }
}

juce::Image LumatoneRender::getResizedAsset(LumatoneAssets::ID assetId, int targetWidth, int targetHeight, bool useJuceResize)
{
    if (targetWidth == 0 || targetHeight == 0)
        return juce::Image();

    auto cachedImage = LumatoneAssets::getImage(assetId, targetHeight, targetWidth);

    if (cachedImage.isNull())
        return cachedImage;

    if (useJuceResize)
        return cachedImage.rescaled(targetWidth, targetHeight, juce::Graphics::ResamplingQuality::highResamplingQuality);

    return imageProcessor->resizeImage(cachedImage, targetWidth, targetHeight);
}

juce::Image LumatoneRender::getResizedRender(int targetWidth, int targetHeight)
{
    if (targetWidth == 0 || targetHeight == 0)
        return juce::Image();

    LumatoneAssets::LumatoneGraphicRenderSize size = LumatoneAssets::GetLumatoneRenderSize(targetWidth, targetHeight);

    juce::Image baseRender = renders[(int)size];

    if (baseRender.isNull())
        return baseRender;

    if (baseRender.getWidth() == targetWidth && baseRender.getHeight() == targetHeight)
        return baseRender;

    return imageProcessor->resizeImage(baseRender, targetWidth, targetHeight);
}
