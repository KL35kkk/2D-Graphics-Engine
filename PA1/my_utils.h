
#include "GColor.h"
#include "GPixel.h"

static int convertFloatToPixel(float value) {
    assert(0 <= value && value <= 1);

    int n = static_cast<int>(floor(value * 255 + 0.5));

    return n;
}

static GPixel convertColorToPixel(GColor c) {
    // normalize to make all components 'in range'
    c = c.pinToUnit();

    int a = convertFloatToPixel(c.fA);
    int r = convertFloatToPixel(c.fR * c.fA);
    int g = convertFloatToPixel(c.fG * c.fA);
    int b = convertFloatToPixel(c.fB * c.fA);

    return GPixel_PackARGB(a, r, g, b);
}

static int div255(int x, int y) {
    // we need this function since a extra 255 will appear after multiplying (255 - Sa) by D
    int result = x * y;

    // similar to floor(x + 0.5), 255/2 => 127
    // return (result + 127) / 255;
    return ((result + 128) * 257) >> 16;
}

static GPixel cover(const GPixel& src, const GPixel& dest) {
    int sAlpha = GPixel_GetA(src);
    int sRed = GPixel_GetR(src);
    int sGreen = GPixel_GetG(src);
    int sBlue = GPixel_GetB(src);

    int dAlpha = GPixel_GetA(dest);
    int dRed = GPixel_GetR(dest);
    int dGreen = GPixel_GetG(dest);
    int dBlue = GPixel_GetB(dest);

    int a = sAlpha + div255(255 - sAlpha, dAlpha);
    int r = sRed + div255(255 - sAlpha, dRed);
    int g = sGreen + div255(255 - sAlpha, dGreen);
    int b = sBlue + div255(255 - sAlpha, dBlue);

    return GPixel_PackARGB(a, r, g, b);
}