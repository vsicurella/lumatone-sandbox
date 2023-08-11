#pragma once

namespace Interpolate
{
    struct TrilinearParams
    {
        float n000;
        float n001;
        float n010;
        float n011;
        float n100;
        float n101;
        float n110;
        float n111;

        float tx;
        float ty;
        float tz;

        TrilinearParams(float _n000, float _n001, float _n010, float _n011, 
                        float _n100, float _n101, float _n110, float _n111,
                        float _tx, float _ty, float _tz)
                    :   n000(_n000), n001(_n001), n010(_n010), n011(_n011), 
                        n100(_n100), n101(_n101), n110(_n110), n111(_n111),
                        tx(_tx), ty(_ty), tz(_tz) { }
    };


    static float nearest(float x0, float x1, float t)
    {
        float delta = x1 - x0;
        if (delta != 0)
        {
            float high, low;
            if (delta > 0)
            {
                high = x1;
                low = x0;
            }
            else
            {
                high = x0;
                low = x1;
            }

            if (t >= 0.5)
                return high;
            
            return low;
        }

        return x0;
    }

    static float linear(float x0, float x1, float t)
    {
        float delta = x1 - x0;
        if (delta == 0)
            return x0;

        float result = x0 * (1 - t) + x1 * t;
        return result;
    }

    static float bilinear(float x0, float x1, float y0, float y1, float tx, float ty)
    {
        float c0 = linear(x0, x1, tx);
        float c1 = linear(x0, x1, tx);
        return linear(c0, c1, ty);
    }

    static float trilinear(float x00, float x01, float x10, float x11,
                           float y00, float y01, float y10, float y11,
                           float tx, float ty, float tz)
    {
        float c0 = bilinear(x00, x01, x10, x11, tx, ty);
        float c1 = bilinear(y00, y01, y10, y11, tx, ty);
        return linear(c0, c1, tz);
    }

    static float trilinear(TrilinearParams& params)
    {
        return trilinear(params.n000, params.n001, params.n010, params.n011,
                         params.n100, params.n101, params.n110, params.n111,
                         params.tx, params.ty, params.tz);
    }
}
