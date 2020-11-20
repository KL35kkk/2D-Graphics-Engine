#include "GMatrix.h"
#include "GPath.h"
#include "GPoint.h"
#include "GRect.h"

#include "shader_utils.h"

GPath& GPath::addPolygon(const GPoint pts[], int count) {
    if (count <= 1) { return *this; }

    this->moveTo(pts[0]);
    for (int i = 1; i < count; ++i) {
        this->lineTo(pts[i]);
    }

    return *this;
}


GPath& GPath::addRect(const GRect& rect, Direction dir) {
    this->moveTo(GPoint::Make(rect.left(), rect.top()));

    if (dir == Direction::kCW_Direction) {
        this->lineTo(GPoint::Make(rect.right(), rect.top()));
        this->lineTo(GPoint::Make(rect.right(), rect.bottom()));
        this->lineTo(GPoint::Make(rect.left(), rect.bottom()));
    } else {
        this->lineTo(GPoint::Make(rect.left(), rect.bottom()));
        this->lineTo(GPoint::Make(rect.right(), rect.bottom()));
        this->lineTo(GPoint::Make(rect.right(), rect.top()));
    }

    return *this;
}


GRect GPath::bounds() const {
    int count = this->fPts.size();

    if (count == 0) {
        return GRect::MakeWH(0, 0);
    }

    float xVals[count], yVals[count];
    for (int i = 0; i < count; ++i) {
        xVals[i] = fPts[i].fX;
        yVals[i] = fPts[i].fY;
    }

    return GRect::MakeLTRB(
                    manyMin(xVals, count),
                    manyMin(yVals, count),
                    manyMax(xVals, count),
                    manyMax(yVals, count));
}


void GPath::transform(const GMatrix& matrix) {
    matrix.mapPoints(this->fPts.data(), this->fPts.data(), this->fPts.size());
}


GPath& GPath::addCircle(GPoint center, float radius, GPath::Direction dir) {
    float t8 = tan(M_PI / 8);
    float sqrt2_2 = sqrt(2) / 2;
    
    GPoint pts[] = {
            {1, 0},
            {1, t8},
            {sqrt2_2, sqrt2_2},
            {t8, 1},
            {0, 1},
            {-t8, 1},
            {-sqrt2_2, sqrt2_2},
            {-1, t8},
            {-1, 0},
            {-1, -t8},
            {-sqrt2_2, -sqrt2_2},
            {-t8, -1},
            {0, -1},
            {t8, -1},
            {sqrt2_2, -sqrt2_2},
            {1, -t8}
    };

    for (int i = 0; i < 16; i ++) {
        pts[i] = center + radius * pts[i];
    }

    moveTo(pts[0]);
    if (dir == 0) {
        // CW
        for (int i = 1; i <= 15; i += 2) {
            quadTo(pts[i], pts[(i + 1) % 16]);
        }
    } else {
        // CCW
        for (int i = 15; i >= 1; i -= 2) {
            quadTo(pts[i], pts[i - 1]);
        }
    }
    return *this;
}


void GPath::ChopQuadAt(const GPoint *src, GPoint *dst, float t) {
    GPoint p1 = src[0];
    GPoint p2 = src[1];
    GPoint p3 = src[2];

    auto z = t;

    dst[0] = p1;
    dst[1] = z * p2 - (z - 1) * p1;
    dst[2] = static_cast<GPoint>(static_cast<GPoint>(pow(z, 2) * p3) - static_cast<GPoint>(2 * z * (z - 1) * p2)) + static_cast<GPoint>(pow((z - 1), 2) * p1);

    dst[3] = z * p3 - (z - 1) * p2;
    dst[4] = p3;
}


void GPath::ChopCubicAt(const GPoint *src, GPoint *dst, float t) {
    auto p1 = src[0];
    auto p2 = src[1];
    auto p3 = src[2];
    auto p4 = src[3];
    auto z = t;

    dst[0] = p1;
    dst[1] = z * p2 - (z - 1) * p1;
    dst[2] = static_cast<GPoint>(pow(z, 2) * p3 - 2 * z * (z - 1) * p2) + pow((z - 1), 2) * p1;
    dst[3] = static_cast<GPoint>(pow(z, 3) * p4 - 3 * pow(z, 2) * (z - 1) * p3) + static_cast<GPoint>(3 * z * pow((z - 1), 2) * p2) - pow((z - 1), 3) * p1;
    dst[4] = static_cast<GPoint>(static_cast<GPoint>(pow(z, 2) * p4) - 2 * z * (z - 1) * p3) + pow((z - 1), 2) * p2;
    dst[5] = z * p4 - (z - 1) * p3;
    dst[6] = p4;
}
