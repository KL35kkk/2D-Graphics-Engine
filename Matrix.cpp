#include <math.h>

#include "GMatrix.h"
#include "GPoint.h"


GMatrix::GMatrix() {    
    fMat[0] = 1;    fMat[1] = 0;    fMat[2] = 0;
    fMat[3] = 0;    fMat[4] = 1;    fMat[5] = 0;
    
}


GMatrix GMatrix::Translate(float tx, float ty) {
    GMatrix matrix = GMatrix(1, 0, tx, 0, 1, ty);

    return matrix;
}


GMatrix GMatrix::Scale(float sx, float sy) {
    GMatrix matrix = GMatrix(sx, 0, 0, 0, sy, 0);

    return matrix;
}


GMatrix GMatrix::Rotate(float radians) {
    GMatrix matrix = GMatrix(
                        cos(radians), -sin(radians), 0,
                        sin(radians), cos(radians), 0);
    return matrix;
}


GMatrix GMatrix::Concat(const GMatrix& a, const GMatrix& b) {
    GMatrix matrix = GMatrix(
        b[0] * a[0] + b[3] * a[1],
        b[1] * a[0] + b[4] * a[1],
        b[2] * a[0] + b[5] * a[1] + a[2],
        b[0] * a[3] + b[3] * a[4],
        b[1] * a[3] + b[4] * a[4],
        b[2] * a[3] + b[5] * a[4] + a[5]);
    
    return matrix;
}


bool GMatrix::invert(GMatrix* inverse) const {
    float a = this->fMat[0];
    float b = this->fMat[1];
    float c = this->fMat[2];
    float d = this->fMat[3];
    float e = this->fMat[4];
    float f = this->fMat[5];

    float determinant = a * e - b * d;
    if (determinant == 0) {
        return false;
    }

    float divisor = 1 / determinant;

    *inverse = GMatrix(
        e * divisor, -b * divisor, -(c * e - b * f) * divisor,
        -d * divisor, a * divisor, (c * d - a * f) * divisor);

    return true;
}


void GMatrix::mapPoints(GPoint dst[], const GPoint src[], int count) const {
    
    for (int i = 0; i < count; i++) {
        
        GPoint temp_src = src[i];

        float new_x = fMat[0] * temp_src.x() + fMat[1] * temp_src.y() + fMat[2];
        float new_y = fMat[3] * temp_src.x() + fMat[4] * temp_src.y() + fMat[5];

        dst[i].set(new_x, new_y);

    }

}
