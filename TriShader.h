#ifndef TriShader_DEFINED
#define TriShader_DEFINED

#include "include/GShader.h"
#include "include/GBitmap.h"
#include "GBlendModes.h"
#include "include/GMatrix.h"
#include "include/GPixel.h"
#include "include/GPoint.h"

class TriShader: public GShader{

public:
    TriShader(const GPoint pts[3], const GColor colors[3]){
        if(colors){
            this -> c10 = GColor::MakeARGB(colors[1].fA - colors[0].fA, colors[1].fR - colors[0].fR, 
                                        colors[1].fG - colors[0].fG, colors[1].fB - colors[0].fB);
            this -> c20 = GColor::MakeARGB(colors[2].fA - colors[0].fA, colors[2].fR - colors[0].fR, 
                                        colors[2].fG - colors[0].fG, colors[2].fB - colors[0].fB);

            this -> c0 = colors[0];
            float ux = pts[1].fX - pts[0].fX, uy = pts[1].fY - pts[0].fY, vx = pts[2].fX - pts[0].fX, vy = pts[2].fY - pts[0].fY;
            lmat = GMatrix(ux, vx, pts[0].fX, uy, vy, pts[0].fY);
            lmat.invert(&fInverse);
        }
    }
    
    bool isOpaque() override{
        return false;
    }

    bool setContext(const GMatrix& ctm) override{
        fInverse = fInverse.Concat(ctm, lmat);
        return fInverse.invert(&fInverse);    
    }

    void shadeRow(int x, int y, int count, GPixel row[]) override{
        GPoint pts[1]{ x + 0.5f, y + 0.5f };
        fInverse.mapPoints(pts, pts, 1);
        GPoint start = pts[0];

        float a = fInverse[0], d = fInverse[3], sx = start.fX, sy = start.fY;
        float c10a = c10.fA, c10r = c10.fR, c10g = c10.fG, c10b = c10.fB;
        float c20a = c20.fA, c20r = c20.fR, c20g = c20.fG, c20b = c20.fB;
        GColor color = GColor::MakeARGB(c10a * sx + c20a * sy + c0.fA,
                                        c10r * sx + c20r * sy + c0.fR,
                                        c10g * sx + c20g * sy + c0.fG,
                                        c10b * sx + c20b * sy + c0.fB);
        float da = a * c10a + d * c20a, dr = a * c10r + d * c20r, dg = a * c10g + d * c20g, db = a * c10b + d * c20b;
        for(int i = 0; i < count; i++){
            row[i] = convertColorToPixel(color);
            color.fA += da;
            color.fR += dr;
            color.fG += dg;
            color.fB += db;
        }
        
    }
private:
    GColor c10, c20, c0;
    GMatrix fInverse;
    GMatrix lmat;
};
#endif