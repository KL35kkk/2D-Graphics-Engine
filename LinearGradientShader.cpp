#include "GShader.h"
#include "GBitmap.h"
#include "GBlendModes.h"
#include "GMatrix.h"
#include "GPixel.h"
#include "GPoint.h"

class MyLinearGradientShader: public GShader {

public:
    MyLinearGradientShader(GPoint p0, GPoint p1, const GColor colors[], int count, TileMode tile) {
        this->fColors = (GColor*) malloc(count * sizeof(GColor));
        memcpy(this->fColors, colors, count * sizeof(GColor));
        this->colorCount = count;
        this->fTile = tile;

        if(p0.fX > p1.fX) {
            std::swap(p0, p1);
        }

        float dx = p1.fX - p0.fX, dy = p1.fY - p0.fY;
        fUnitMatrix = GMatrix(
                        dx, -dy, p0.fX,
                        dy, dx, p0.fY);        
    }

    bool isOpaque() override {
        return false;
    }


    bool setContext(const GMatrix& ctm) override {
        fInverse = fInverse.Concat(ctm, fUnitMatrix);

        return fInverse.invert(&fInverse);    
    }

    void shadeRow(int x, int y, int count, GPixel row[]) override {
        GPoint pts[1]{ x + 0.5f, y + 0.5f };
        fInverse.mapPoints(pts, pts, 1);
        GPoint start = pts[0];

        float a = fInverse[0];
        float ft;
        for (int i = 0; i < count; i++) {
            ft = start.fX;
            if (fTile == TileMode::kRepeat) {
                ft = ft - GFloorToInt(ft);

            } else if (fTile == TileMode::kMirror) {
                ft *= 0.5;
                ft = ft - GFloorToInt(ft);
                //
                ft *= 2;


            } else {
                ft = std::max(0.0f, std::min(1.0f, ft));

            }

            ft = ft * (colorCount - 1);
            int idx = GFloorToInt(ft);
            float loc = ft - idx;
            GColor c1 = fColors[idx].pinToUnit(), c2 = fColors[idx + 1].pinToUnit();
            row[i] = convertColorToPixel(GColor::MakeARGB(c1.fA * (1 - loc) + c2.fA * loc,
                                                            c1.fR * (1 - loc) + c2.fR * loc,
                                                            c1.fG * (1 - loc) + c2.fG * loc,
                                                            c1.fB * (1 - loc) + c2.fB * loc));
           
            start.fX += a;
        }
        
    }
private:
    GColor* fColors;
    GMatrix fInverse;
    GMatrix fUnitMatrix;
    int colorCount;
    TileMode fTile;

};

std::unique_ptr<GShader> GCreateLinearGradient(GPoint p0, GPoint p1, const GColor colors[], int count, GShader::TileMode tile) {
    if(count < 1) {
        return nullptr;
    }

    return std::unique_ptr<GShader>(new MyLinearGradientShader(p0, p1, colors, count, tile));
}
