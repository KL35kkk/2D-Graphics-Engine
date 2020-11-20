#ifndef CompositeShader_DEFINED
#define CompositeShader_DEFINED


#include "include/GShader.h"
#include "GBlendModes.h"
#include "include/GPixel.h"
#include "include/GPoint.h"


class CompositeShader: public GShader{

public:
    CompositeShader(GShader* shader1, GShader* shader2){
        this -> shader1 = shader1;
        this -> shader2 = shader2;

    }
    bool isOpaque() override { 
        return shader1 -> isOpaque() && shader2 -> isOpaque(); 
    }

    bool setContext(const GMatrix& ctm) override {
        return shader1 -> setContext(ctm) && shader2 -> setContext(ctm);    
    }

    void shadeRow(int x, int y, int count, GPixel row[]) override{
        GPixel scratch[count];
        shader1 -> shadeRow(x, y, count, scratch);
        shader2 -> shadeRow(x, y, count, row);
        for(int i = 0; i < count; i++){
            row[i] = GMultPixel(row[i], scratch[i]);
        }
    }
    
private:
    GShader* shader1;
    GShader* shader2;
};

#endif