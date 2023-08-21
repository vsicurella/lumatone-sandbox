/*
  ==============================================================================

    hex_geometry.h
    Created: 4 Jun 2023 4:59:13pm
    Author:  Vincenzo

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

namespace Hex
{
    struct Cube
    {
        float q = 0;
        float r = 0;
        float s = 0;

        Hex::Cube operator-(const Cube& toSubtract) const;
        Hex::Cube operator+(const Cube& toAdd) const;
        
        float distanceTo(const Cube& toPoint) const;

        void round();
    };

    class Point
    {
    public:
        float q = 0;
        float r = 0;

    private:

        static float line(int a, int b, float t);

    public:

        static float distance(Hex::Point a, Hex::Point b);
        
        static Cube axialToCube(int q, int r);
        static Cube axialToCube(float q, float r);

        static Cube cubeLinearInterpolation(Cube a, Cube b, float t);
        static Cube cubeLineDraw(Cube a, Cube b, int i, int N = 0);

        static Hex::Point rounded(Hex::Point point);

    public:

        Point() {}
        Point(int hex_q, int hex_r);
        Point(float hex_q, float hex_r);
        Point(Hex::Cube cube);

        juce::String toString() const { return juce::String(q) + "," + juce::String(r); }

        bool operator==(const Hex::Point& compare) const;

        Hex::Point operator+(const Hex::Point& toAdd) const;
        void operator+=(const Hex::Point& toAdd);

        Hex::Point operator-(const Hex::Point& toSubtract) const;
        void operator-=(const Hex::Point& toSubtract);

        Cube toCube() const;
        void round();

        int distanceTo(Hex::Point toPoint) const;

        void transpose(int delta_q, int delta_r);
        void transposeQ(int delta_q);
        void transposeR(int delta_r);

        Hex::Point stepUpQ(int steps = 1);
        Hex::Point stepDownQ(int steps = 1);
        
        Hex::Point stepUpR(int steps = 1);
        Hex::Point stepDownR(int steps = 1);
        
        Hex::Point stepUpQUpR(int steps_q = 1, int steps_r = 1);
        Hex::Point stepUpQDownR(int steps_q = 1, int steps_r = 1);
        
        Hex::Point stepDownQUpR(int steps_q = 1, int steps_r = 1);
        Hex::Point stepDownQDownR(int steps_q = 1, int steps_r = 1);

        juce::Array<Hex::Point> ring(int distance = 1) const;
        juce::Array<Hex::Point> neighbors(int maxDistance = 1) const;

        juce::Array<Hex::Point> lineTo(Hex::Point endPoint);
    };
}