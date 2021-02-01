#ifndef Clipper_DEFINED
#define Clipper_DEFINED

#include <vector>

#include "GPoint.h"
#include "GRect.h"

// new edge
struct Edge {
    int topY;
    int bottomY;
    float curX;
    float slope;
    short wind;

    Edge() {}
    Edge(GPoint, GPoint, int wind);

    bool operator<(const Edge& other) const;

};

void clipLine(GPoint p0, GPoint p1, GRect bounds, std::vector<Edge> &edges);


#endif
