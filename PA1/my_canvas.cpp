#include <iostream>

#include "GBitmap.h"
#include "GCanvas.h"
#include "GColor.h"
#include "GPixel.h"
#include "GRect.h"
#include "my_utils.h"


class EmptyCanvas : public GCanvas {
public:
    EmptyCanvas(const GBitmap& device) : fDevice(device) {}
    /**
     *  Fill the entire canvas with the specified color, using SRC porter-duff mode.
     */
    void clear(const GColor& givenColor) override {
        GPixel pixel = convertColorToPixel(givenColor);
        // std::cout << pixel << std::endl;

        for (int x = 0; x < fDevice.height(); x++) {
            for (int y = 0; y < fDevice.width(); y++) {
                GPixel* address = fDevice.getAddr(y, x);

                *address = pixel;
            }
        }
    };
    
    /**
     *  Fill the rectangle with the color, using SRC_OVER porter-duff mode.
     *
     *  The affected pixels are those whose centers are "contained" inside the rectangle:
     *      e.g. contained == center > min_edge && center <= max_edge
     *
     *  Any area in the rectangle that is outside of the bounds of the canvas is ignored.
     */
    void fillRect(const GRect& rect, const GColor& color) override {
        GIRect rounded = rect.round();

        // Limit the rectangle's area to the boundaries of the canvas.
        rounded.fLeft = std::max(rounded.fLeft, 0);
        rounded.fTop = std::max(rounded.fTop, 0);
        rounded.fRight = std::min(rounded.fRight, fDevice.width());
        rounded.fBottom = std::min(rounded.fBottom, fDevice.height());

        GPixel source = convertColorToPixel(color);

        for (int y = rounded.fTop; y < rounded.fBottom; ++y) {
            for (int x = rounded.fLeft; x < rounded.fRight; ++x) {
                GPixel* addr = fDevice.getAddr(x, y);
                GPixel dest = *addr;

                *addr = cover(source, dest);
            }
        }
    };

private:
    const GBitmap fDevice;
};

std::unique_ptr<GCanvas> GCreateCanvas(const GBitmap& device) {
    if (!device.pixels()) {
        return nullptr;
    }

    return std::unique_ptr<GCanvas>(new EmptyCanvas(device));
}

std::string GDrawSomething(GCanvas* canvas) {
    // GRect rect = GRect::MakeXYWH(0, 0, 256, 256/7);
    GColor color = GColor::MakeARGB(1, 0.5, 0.5, 0.5);
    canvas->clear(color);
    for (int i = 0; i < 8; i++) {
        for (int n = 0; n < 8; n++) {
            GRect rect = GRect::MakeXYWH(n * (256/8), i * (256/8), (n+1) * (256/8), (i+1) * (256/8));
            GColor color = GColor::MakeARGB(0.1 + (0.1*i), (0.1*n), 1 - (0.1*n), (0.1*i));
            canvas->fillRect(rect, color);
        }
    }


    return std::string("Kevin's canvas");
}