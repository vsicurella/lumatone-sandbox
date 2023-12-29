#ifndef LUMATONE_GRAPHICS_RENDER_H
#define LUMATONE_GRAPHICS_RENDER_H

#include "./lumatone_assets.h"

#include "../data/application_state.h"
#include "../mapping/lumatone_tiling.h"

#include "../graphics/resampler/image_resampler.h"

class LumatoneRender
{
public:
    LumatoneRender(LumatoneApplicationState& stateIn);
    ~LumatoneRender();

    LumatoneGeometry& getLumatoneGeometry() { return lumatoneGeometry; }
    LumatoneTiling& getLumatoneTiling() { return tilingGeometry; }

    void resetOctaveSize();

    juce::Array<juce::Point<float>> getKeyCentres();

    void render(LumatoneAssets::LumatoneGraphicRenderSize maxRenderSize=LumatoneAssets::LumatoneGraphicRenderSize::_4x);
    
    juce::Image getResizedRender(int targetWidth, int targetHeight);
    juce::Image getResizedAsset(LumatoneAssets::ID assetId, int targetWidth, int targetHeight, bool useJuceResize=false);

private:

    LumatoneApplicationState state;

    LumatoneGeometry    lumatoneGeometry;
    LumatoneTiling      tilingGeometry;

    juce::Array<juce::Point<float>> keyCentres;

    std::unique_ptr<ImageProcessor> imageProcessor;

    juce::Image lumatoneGraphic;
    juce::Image keyShapeGraphic;
    juce::Image keyShadowGraphic;

    juce::Array<juce::Image> renders;

    // In reference to lumatoneBounds
    const float keybedX = 0.06908748f;

    const float oct1Key1X = 0.0839425f;
    const float oct1Key1Y = 0.335887f;

    const float oct1Key56X = 0.2731f;
    const float oct1Key56Y = 0.83135f;

    const float oct5Key7X = 0.879f;
    const float oct5Key7Y = 0.355f;

    const float keyW = 0.027352f;
    const float keyH = 0.07307f;

    //===============================================================================

    juce::Point<float>  oct1Key1;
    juce::Point<float> oct1Key56;
    juce::Point<float>  oct5Key7;
};

#endif LUMATONE_GRAPHICS_RENDER_H
