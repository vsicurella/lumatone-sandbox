/*
  ==============================================================================

    hex_geometry.cpp
    Created: 4 Jun 2023 4:59:13pm
    Author:  Vincenzo

  ==============================================================================
*/

#include "hex_field.h"

Hex::Cube Hex::Cube::operator-(const Cube& toSubtract) const
{
    return { q - toSubtract.q, r - toSubtract.r, s - toSubtract.s };
}

Hex::Cube Hex::Cube::operator+(const Cube& toAdd) const
{
    return { q + toAdd.q, r + toAdd.q, s + toAdd.s };
}

float Hex::Cube::distanceTo(const Cube& toPoint) const
{
    auto dif = *this - toPoint;
    return (abs(dif.q) + abs(dif.r) + abs(dif.s)) / 2.0f;
}

void Hex::Cube::round()
{
    int rQ = juce::roundToInt(q);
    int rR = juce::roundToInt(r);
    int rS = juce::roundToInt(s);

    float dQ = abs(rQ - q);
    float dR = abs(rR - r);
    float dS = abs(rS = s);

    if (dQ < dR && dQ > dS)
        rQ = -rR - rS;
    else if (dR > dS)
        rR = -rQ - rS;
    else
        rS = -rQ - rR;

    q = rQ;
    r = rR;
    s = rS;
}

Hex::Point::Point(int hex_q, int hex_r)
    : q(hex_q), r(hex_r) { }

Hex::Point::Point(float hex_q, float hex_r)
    : q(hex_q), r(hex_r) { }

Hex::Point::Point(Hex::Cube cube) : q(cube.q), r(cube.r) { }

float Hex::Point::line(int a, int b, float t)
{
    return a + (b - a) * t;
}

float Hex::Point::distance(Hex::Point a, Hex::Point b)
{
    return ( abs(a.q - b.q) 
           + abs(a.q + a.r - b.q - b.r)
           + abs(a.r - b.r)
           ) / 2.0f;
}

Hex::Cube Hex::Point::axialToCube(int q, int r)
{
    return { (float)q, (float)r, (float)(-q - r) 
};

}Hex::Cube Hex::Point::axialToCube(float q, float r)
{
    return { q, r, -q - r };
}

Hex::Cube Hex::Point::cubeLinearInterpolation(Cube a, Cube b, float t)
{
    return {
        line(a.q, b.q, t),
        line(a.r, b.r, t),
        line(a.s, b.s, t)
    };
}

Hex::Cube Hex::Point::cubeLineDraw(Hex::Cube a, Hex::Cube b, int i, int N)
{
    if (N == 0)
        N = (int)a.distanceTo(b);

    if (N == 0)
        return a;

    float t = i / N;
    return cubeLinearInterpolation(a, b, t);
}

Hex::Point Hex::Point::rounded(Hex::Point point)
{
    auto cubed = point.toCube();
    cubed.round();
    return Hex::Point(cubed.q, cubed.r);
}

Hex::Cube Hex::Point::toCube() const
{
    return axialToCube(q, r);
}

void Hex::Point::round()
{
    auto cubed = toCube();
    cubed.round();
    
    q = cubed.q;
    r = cubed.r;
}

bool Hex::Point::operator==(const Hex::Point& compare) const
{
    return q == compare.q && r == compare.r;
}

Hex::Point Hex::Point::operator+(const Hex::Point& toAdd) const
{
    return Hex::Point(q + toAdd.q, r + toAdd.r);
}

void Hex::Point::operator+=(const Hex::Point & toAdd)
{
    q += toAdd.q;
    r += toAdd.r;
}

Hex::Point Hex::Point::operator-(const Hex::Point & toSubtract) const
{
    return Hex::Point(q - toSubtract.q, r - toSubtract.r);
}

void Hex::Point::operator-=(const Hex::Point & toSubtract)
{
    q -= toSubtract.q;
    r -= toSubtract.r;
}

int Hex::Point::distanceTo(Hex::Point toPoint) const
{
    return Hex::Point::distance(*this, toPoint);
}

void Hex::Point::transpose(int delta_q, int delta_r)
{
    q += delta_q;
    r += delta_r;
}

void Hex::Point::transposeQ(int delta_q)
{
    q += delta_q;
}

void Hex::Point::transposeR(int delta_r)
{
    r += delta_r;
}

Hex::Point Hex::Point::stepUpQ(int steps)
{
    return Hex::Point(q + steps, r);
}

Hex::Point Hex::Point::stepDownQ(int steps)
{
    return Hex::Point(q - steps, r);
}

Hex::Point Hex::Point::stepUpR(int steps)
{
    return Hex::Point(q, r + steps);
}

Hex::Point Hex::Point::stepDownR(int steps)
{
    return Hex::Point(q, r - steps);
}

Hex::Point Hex::Point::stepUpQUpR(int steps_q, int steps_r)
{
    return Hex::Point(q + steps_q, r + steps_r);
}

Hex::Point Hex::Point::stepUpQDownR(int steps_q, int steps_r)
{
    return Hex::Point(q + steps_q, r - steps_r);
}

Hex::Point Hex::Point::stepDownQUpR(int steps_q, int steps_r)
{
    return Hex::Point(q - steps_q, r + steps_r);
}

Hex::Point Hex::Point::stepDownQDownR(int steps_q, int steps_r)
{
    return Hex::Point(q - steps_q, r - steps_r);
}

juce::Array<Hex::Point> Hex::Point::ring(int distance) const
{
    juce::Array<Hex::Point> points;

    if (distance > 0) for (int d = 0; d <= distance; d++)
    {
        points.add(Hex::Point(q + distance, r - distance + d));
        points.add(Hex::Point(q - d, r + distance));
        points.add(Hex::Point(q - distance + d, r - d));
    }

    return points;
}

juce::Array<Hex::Point> Hex::Point::neighbors(int maxDistance) const
{
    juce::Array<Hex::Point> points;

    for (int d = 1; d <= maxDistance; d++)
    {
        points.addArray(ring(d));
    }

    return points;
}

juce::Array<Hex::Point> Hex::Point::lineTo(Hex::Point endPoint)
{
    juce::Array<Hex::Point> points;

    auto cubeA = toCube();
    auto cubeB = endPoint.toCube();

    int N = (int)cubeA.distanceTo(cubeB);
    for (int i = 0; i <= N; N++)
    {
        auto cube = cubeLineDraw(cubeA, cubeB, i, N);
        points.add(Hex::Point(cube));
    }

    return points;
}

