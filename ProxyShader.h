#ifndef ProxyShader_DEFINED
#define ProxyShader_DEFINED

#include "include/GShader.h"
#include "include/GBitmap.h"
#include "include/GMatrix.h"
#include "include/GPixel.h"
#include "include/GPoint.h"

class ProxyShader: public GShader{
public:
    ProxyShader(GShader* shader, const GPoint pts[3], const GPoint texs[3]){
        if(texs){
            GMatrix T, P;
            T = GMatrix(texs[1].fX - texs[0].fX, texs[2].fX - texs[0].fX, texs[0].fX,
                    texs[1].fY - texs[0].fY, texs[2].fY - texs[0].fY, texs[0].fY);
            P = GMatrix(pts[1].fX - pts[0].fX, pts[2].fX - pts[0].fX, pts[0].fX,
                    pts[1].fY - pts[0].fY, pts[2].fY - pts[0].fY, pts[0].fY);
            T.invert(&T);
            extra = extra.Concat(P, T);
        }
        this -> realShader = shader;
    }

    bool isOpaque() override { 
        return realShader -> isOpaque(); 
    }

    bool setContext(const GMatrix& ctm) override {
        GMatrix mat;
        mat = mat.Concat(ctm, extra);
        return realShader -> setContext(mat);
    }
    
    void shadeRow(int x, int y, int count, GPixel row[]) override {
        realShader ->shadeRow(x, y, count, row);
    }

private:
    GMatrix extra;
    GShader* realShader;
};


#endif