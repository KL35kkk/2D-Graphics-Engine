#ifndef Blend_DEFINED
#define Blend_DEFINED

#include "GColor.h"
#include "GPaint.h"
#include "GPixel.h"
#include "GShader.h"
#include "GBlendMode.h"

static inline int convertFloatToPixel(float value) {
    assert(0 <= value && value <= 1);
    int n = static_cast<int>(floor(value * 255 + 0.5));

    return n;
}

static inline GPixel convertColorToPixel(GColor color) {
    color = color.pinToUnit();

    int alpha = convertFloatToPixel(color.fA);
    int red   = convertFloatToPixel(color.fR * color.fA);
    int green = convertFloatToPixel(color.fG * color.fA);
    int blue  = convertFloatToPixel(color.fB * color.fA);

    return GPixel_PackARGB(alpha, red, green, blue);
}


static inline int div255(int x, int y) {
    int result = x * y;


    return ((result + 128) * 257) >> 16;
}

static inline GPixel GMultPixel(const GPixel& p1, const GPixel& p2) {
    int a1 = GPixel_GetA(p1), r1 = GPixel_GetR(p1), g1 = GPixel_GetG(p1), b1 = GPixel_GetB(p1);
    int a2 = GPixel_GetA(p2), r2 = GPixel_GetR(p2), g2 = GPixel_GetG(p2), b2 = GPixel_GetB(p2);
    
    return GPixel_PackARGB(div255(a1, a2), div255(r1, r2), div255(g1, g2), div255(b1, b2));
}

// Helper functions for calculating the result pixel for each of the 12 blend modes. 
static GPixel Clear(const GPixel source, const GPixel dest) {
    return GPixel_PackARGB(0, 0, 0, 0);
}

static GPixel Src(const GPixel source, const GPixel dest) {
    return source;
}

static GPixel Dst(const GPixel source, const GPixel dest) {
    return dest;
}


static GPixel SrcOver(const GPixel source, const GPixel dest) {
    //!< [Sa + Da * (1 - Sa), Sc + Dc * (1 - Sa)]
    int sA = GPixel_GetA(source);
    int sR = GPixel_GetR(source);
    int sG = GPixel_GetG(source);
    int sB = GPixel_GetB(source);
    
    int dA = GPixel_GetA(dest);
    int dR = GPixel_GetR(dest);
    int dG = GPixel_GetG(dest);
    int dB = GPixel_GetB(dest);
    
    int a = sA + div255(255 - sA, dA);
    int r = sR + div255(255 - sA, dR);
    int g = sG + div255(255 - sA, dG);
    int b = sB + div255(255 - sA, dB);   
    return GPixel_PackARGB(a, r, g, b);
}

static GPixel DstOver(const GPixel source, const GPixel dest) {
    //!< [Da + Sa * (1 - Da), Dc + Sc * (1 - Da)]
    int sA = GPixel_GetA(source);
    int sR = GPixel_GetR(source);
    int sG = GPixel_GetG(source);
    int sB = GPixel_GetB(source);
    
    int dA = GPixel_GetA(dest);
    int dR = GPixel_GetR(dest);
    int dG = GPixel_GetG(dest);
    int dB = GPixel_GetB(dest);
    
    int a = dA + div255(255 - dA, sA);
    int r = dR + div255(255 - dA, sR);
    int g = dG + div255(255 - dA, sG);
    int b = dB + div255(255 - dA, sB); 
    return GPixel_PackARGB(a, r, g, b);
}

static GPixel SrcIn(const GPixel source, const GPixel dest) {
    //!< [Sa * Da, Sc * Da]
    int sA = GPixel_GetA(source);
    int sR = GPixel_GetR(source);
    int sG = GPixel_GetG(source);
    int sB = GPixel_GetB(source);
    
    int dA = GPixel_GetA(dest);

    int a = div255(sA, dA);
    int r = div255(sR, dA);
    int g = div255(sG, dA);
    int b = div255(sB, dA);
    return GPixel_PackARGB(a, r, g, b);
}

static GPixel DstIn(const GPixel source, const GPixel dest) {
    //!< [Da * Sa, Dc * Sa]  
    int sA = GPixel_GetA(source);
    
    int dA = GPixel_GetA(dest);
    int dR = GPixel_GetR(dest);
    int dG = GPixel_GetG(dest);
    int dB = GPixel_GetB(dest);

    int a = div255(sA, dA);
    int r = div255(sA, dR);
    int g = div255(sA, dG);
    int b = div255(sA, dB);
    return GPixel_PackARGB(a, r, g, b);
}

static GPixel SrcOut(const GPixel source, const GPixel dest) {
    //!< [Sa * (1 - Da), Sc * (1 - Da)]
    int sA = GPixel_GetA(source);
    int sR = GPixel_GetR(source);
    int sG = GPixel_GetG(source);
    int sB = GPixel_GetB(source);
    
    int dA = GPixel_GetA(dest);

    int a = div255(sA, 255 - dA);
    int r = div255(sR, 255 - dA);
    int g = div255(sG, 255 - dA);
    int b = div255(sB, 255 - dA);
    return GPixel_PackARGB(a, r, g, b);
}

static GPixel DstOut(const GPixel source, const GPixel dest) {
    //!< [Da * (1 - Sa), Dc * (1 - Sa)]
    int sA = GPixel_GetA(source);
    
    int dA = GPixel_GetA(dest);
    int dR = GPixel_GetR(dest);
    int dG = GPixel_GetG(dest);
    int dB = GPixel_GetB(dest);

    int a = div255(dA, 255 - sA);
    int r = div255(dR, 255 - sA);
    int g = div255(dG, 255 - sA);
    int b = div255(dB, 255 - sA);
    return GPixel_PackARGB(a, r, g, b);
}

static GPixel SrcATop(const GPixel source, const GPixel dest) {
    //!< [Da, Sc * Da + Dc * (1 - Sa)]
    int sA = GPixel_GetA(source);
    int sR = GPixel_GetR(source);
    int sG = GPixel_GetG(source);
    int sB = GPixel_GetB(source);
    
    int dA = GPixel_GetA(dest);
    int dR = GPixel_GetR(dest);
    int dG = GPixel_GetG(dest);
    int dB = GPixel_GetB(dest);

    int a = dA;
    int r = div255(sR, dA) + div255(dR, 255 - sA);
    int g = div255(sG, dA) + div255(dG, 255 - sA);
    int b = div255(sB, dA) + div255(dB, 255 - sA);
    return GPixel_PackARGB(a, r, g, b);
}

static GPixel DstATop(const GPixel source, const GPixel dest) {
    //!< [Sa, Dc * Sa + Sc * (1 - Da)]
    int sA = GPixel_GetA(source);
    int sR = GPixel_GetR(source);
    int sG = GPixel_GetG(source);
    int sB = GPixel_GetB(source);
    
    int dA = GPixel_GetA(dest);
    int dR = GPixel_GetR(dest);
    int dG = GPixel_GetG(dest);
    int dB = GPixel_GetB(dest);

    int a = sA;
    int r = div255(dR, sA) + div255(sR, 255 - dA);
    int g = div255(dG, sA) + div255(sG, 255 - dA);
    int b = div255(dB, sA) + div255(sB, 255 - dA);
    return GPixel_PackARGB(a, r, g, b);
}

static GPixel Xor(const GPixel source, const GPixel dest) {
    //!< [Sa + Da - 2 * Sa * Da, Sc * (1 - Da) + Dc * (1 - Sa)]
    int sA = GPixel_GetA(source);
    int sR = GPixel_GetR(source);
    int sG = GPixel_GetG(source);
    int sB = GPixel_GetB(source);
    
    int dA = GPixel_GetA(dest);
    int dR = GPixel_GetR(dest);
    int dG = GPixel_GetG(dest);
    int dB = GPixel_GetB(dest);

    int a = sA + dA - div255(2 * sA, dA);
    int r = div255(sR, 255 - dA) + div255(dR, 255 - sA);
    int g = div255(sG, 255 - dA) + div255(dG, 255 - sA);
    int b = div255(sB, 255 - dA) + div255(dB, 255 - sA);
    return GPixel_PackARGB(a, r, g, b);
}
typedef GPixel(*BlendProc) (GPixel, GPixel);



static BlendProc BLEND_PROCS[] = {
    Clear,
    Src,
    Dst,
    SrcOver,
    DstOver,
    SrcIn,
    DstIn,
    SrcOut,
    DstOut,
    SrcATop,
    DstATop,
    Xor
};

static inline BlendProc getBlendProc(const GBlendMode mode) {
    return BLEND_PROCS[static_cast<int>(mode)];
}

static inline GPixel div2_127(const GPixel& pixel){
    return GPixel_PackARGB((GPixel_GetA(pixel)) >> 1, 
                            (GPixel_GetR(pixel)) >> 1, 
                            (GPixel_GetG(pixel)) >> 1, 
                            (GPixel_GetB(pixel)) >> 1);
}

static inline GPixel div2_128(const GPixel& pixel){
    return GPixel_PackARGB((GPixel_GetA(pixel) + 1) >> 1, 
                            (GPixel_GetR(pixel) + 1) >> 1, 
                            (GPixel_GetG(pixel) + 1) >> 1, 
                            (GPixel_GetB(pixel) + 1) >> 1);
}

static inline void Blend(const GPaint& paint, GPixel* row, int count) {
    GBlendMode mode = paint.getBlendMode();
    GColor color = paint.getColor();
    GPixel src = convertColorToPixel(color);
    float fA = color.fA;
    if(fA == 0.0){
        switch(mode){
            case GBlendMode::kClear:
                std::fill(row, row + count, 0);
                break;
            case GBlendMode::kSrc: 
                std::fill(row, row + count, 0);
                break;
            case GBlendMode::kDst:
                break;
            case GBlendMode::kSrcOver:
                break;
            case GBlendMode::kDstOver:
                break;
            case GBlendMode::kSrcIn: 
                std::fill(row, row + count, 0);
                break;
            case GBlendMode::kDstIn: 
                std::fill(row, row + count, 0);
                break;
            case GBlendMode::kSrcOut:
                std::fill(row, row + count, 0);
                break;
            case GBlendMode::kDstOut:  
                break;
            case GBlendMode::kSrcATop: 
                break;
            case GBlendMode::kDstATop: 
                std::fill(row, row + count, 0);
                break;
            case GBlendMode::kXor:{
                break;
            }
        }
    } else if(fA == 1.0){
        switch(mode){
            case GBlendMode::kClear:
                std::fill(row, row + count, 0);
                break;
            case GBlendMode::kSrc: 
                std::fill(row, row + count, src);
                break;
            case GBlendMode::kDst:
                break;
            case GBlendMode::kSrcOver:
                std::fill(row, row + count, src);
                break;
            case GBlendMode::kDstOver:
                for(int i = 0; i < count; ++i) {
                    row[i] = DstOver(src, row[i]);
                }
                break;
            case GBlendMode::kSrcIn: 
                for(int i = 0; i < count; ++i) {
                    row[i] = SrcIn(src, row[i]);
                }    
                break;
            case GBlendMode::kDstIn: 
                break;
            case GBlendMode::kSrcOut:
                for(int i = 0; i < count; ++i) {
                    row[i] = SrcOut(src, row[i]);
                }
                break;
            case GBlendMode::kDstOut:  
                std::fill(row, row + count, 0);
                break;
            case GBlendMode::kSrcATop: 
                for(int i = 0; i < count; ++i) {
                    row[i] = SrcIn(src, row[i]);
                }
                break;
            case GBlendMode::kDstATop: 
                for(int i = 0; i < count; ++i) {
                    row[i] = SrcOut(src, row[i]) + row[i];
                }
                break;
            case GBlendMode::kXor:{
                for(int i = 0; i < count; ++i) {
                    row[i] = SrcOut(src, row[i]);
                }
                break;
            }
        }
    } else if(fA == 0.5) {
        switch(mode){
            case GBlendMode::kClear:
                std::fill(row, row + count, 0);
                break;
            case GBlendMode::kSrc: 
                std::fill(row, row + count, src);
                break;
            case GBlendMode::kDst:
                break;
            case GBlendMode::kSrcOver:
                for(int i = 0; i < count; ++i) {
                    row[i] = src + div2_127(row[i]);
                }
                break;
            case GBlendMode::kDstOver:
                for(int i = 0; i < count; ++i) {
                    row[i] = DstOver(src, row[i]);
                }
                break;
            case GBlendMode::kSrcIn: 
                for(int i = 0; i < count; ++i) {
                    row[i] = SrcIn(src, row[i]);
                }
                break;
            case GBlendMode::kDstIn: 
                for(int i = 0; i < count; ++i) {
                    row[i] = div2_128(row[i]);
                }
                break;
            case GBlendMode::kSrcOut:
                for(int i = 0; i < count; ++i) {
                    row[i] = SrcOut(src, row[i]);
                }
                break;
            case GBlendMode::kDstOut:  
                for(int i = 0; i < count; ++i) {
                    row[i] = div2_127(row[i]);
                }
                break;
            case GBlendMode::kSrcATop: 
                for(int i = 0; i < count; ++i) {
                    row[i] = SrcIn(src, row[i]) + div2_127(row[i]);
                }
                break;
            case GBlendMode::kDstATop: 
                for(int i = 0; i < count; ++i) {
                    row[i] = SrcOut(src, row[i]) + div2_128(row[i]);
                }
                break;
            case GBlendMode::kXor:{
                for(int i = 0; i < count; ++i)
                    row[i] = Xor(src, row[i]);
                break;
            }
        }
    } else {
        switch(mode){
            case GBlendMode::kClear:
                std::fill(row, row + count, 0);
                break;
            case GBlendMode::kSrc: 
                std::fill(row, row + count, src);
                break;
            case GBlendMode::kDst:
                break;
            case GBlendMode::kSrcOver:
                for(int i = 0; i < count; ++i) {
                    row[i] = SrcOver(src, row[i]);
                }
                break;
            case GBlendMode::kDstOver:
                for(int i = 0; i < count; ++i) {
                    row[i] = DstOver(src, row[i]);
                }
                break;
            case GBlendMode::kSrcIn: 
                for(int i = 0; i < count; ++i) {
                    row[i] = SrcIn(src, row[i]);
                }
                break;
            case GBlendMode::kDstIn: 
                for(int i = 0; i < count; ++i) {
                    row[i] = DstIn(src, row[i]);
                }
                break;
            case GBlendMode::kSrcOut:
                for(int i = 0; i < count; ++i) {
                    row[i] = SrcOut(src, row[i]);
                }
                break;
            case GBlendMode::kDstOut:  
                for(int i = 0; i < count; ++i) {
                    row[i] = DstOut(src, row[i]);
                }
                break;
            case GBlendMode::kSrcATop: 
                for(int i = 0; i < count; ++i) {
                    row[i] = SrcATop(src, row[i]);
                }
                break;
            case GBlendMode::kDstATop: 
                for(int i = 0; i < count; ++i) {
                    row[i] = DstATop(src, row[i]);
                }
                break;
            case GBlendMode::kXor:{
                for(int i = 0; i < count; ++i) {
                    row[i] = Xor(src, row[i]);
                }
                break;
            }
        }
    }
}

    
static inline void Shade(const GPaint& paint, GPixel* row, int x, int y, int count) {
    if (paint.getShader() == nullptr) {
        Blend(paint, row, count);
    } else {
        GShader* shader = paint.getShader();
        GPixel src[count];
        shader->shadeRow(x, y, count, src);
        GBlendMode mode = paint.getBlendMode();
        if (shader -> isOpaque()) {
            switch(mode) {
                case GBlendMode::kClear:
                    std::fill(row, row + count, 0);
                    break;
                case GBlendMode::kSrc: 
                    for(int i = 0; i < count; ++i)
                    row[i] = src[i];
                    break;
                case GBlendMode::kDst:
                    break;
                case GBlendMode::kSrcOver:
                    for(int i = 0; i < count; ++i)
                    row[i] = src[i];
                    break;
                case GBlendMode::kDstOver:
                    for(int i = 0; i < count; ++i)
                    row[i] = DstOver(src[i], row[i]);
                    break;
                case GBlendMode::kSrcIn: 
                    for(int i = 0; i < count; ++i)
                    row[i] = SrcIn(src[i], row[i]);
                    break;
                case GBlendMode::kDstIn:
                    break;
                case GBlendMode::kSrcOut:
                    for(int i = 0; i < count; ++i)
                    row[i] = SrcOut(src[i], row[i]);
                    break;
                case GBlendMode::kDstOut:  
                    std::fill(row, row + count, 0);
                    break;
                case GBlendMode::kSrcATop: 
                    for(int i = 0; i < count; ++i)
                    row[i] = SrcIn(src[i], row[i]);
                    break;
                case GBlendMode::kDstATop: 
                    for(int i = 0; i < count; ++i)
                    row[i] = row[i] + SrcOut(src[i], row[i]);
                    break;
                case GBlendMode::kXor:{
                    for(int i = 0; i < count; ++i)
                    row[i] = SrcOut(src[i], row[i]);
                    break;
                }
            }
        } else {
            switch(mode){
                case GBlendMode::kClear:
                    std::fill(row, row + count, 0);
                    break;
                case GBlendMode::kSrc: 
                    for(int i = 0; i < count; ++i)
                    row[i] = Src(src[i], row[i]);
                    break;
                case GBlendMode::kDst:
                    break;
                case GBlendMode::kSrcOver:
                    for(int i = 0; i < count; ++i)
                    row[i] = SrcOver(src[i], row[i]);
                    break;
                case GBlendMode::kDstOver:
                    for(int i = 0; i < count; ++i)
                    row[i] = DstOver(src[i], row[i]);
                    break;
                case GBlendMode::kSrcIn: 
                    for(int i = 0; i < count; ++i)
                    row[i] = SrcIn(src[i], row[i]);
                    break;
                case GBlendMode::kDstIn: 
                    for(int i = 0; i < count; ++i)
                    row[i] = DstIn(src[i], row[i]);
                    break;
                case GBlendMode::kSrcOut:
                    for(int i = 0; i < count; ++i)
                    row[i] = SrcOut(src[i], row[i]);
                    break;
                case GBlendMode::kDstOut:  
                    for(int i = 0; i < count; ++i)
                    row[i] = DstOut(src[i], row[i]);
                    break;
                case GBlendMode::kSrcATop: 
                    for(int i = 0; i < count; ++i)
                    row[i] = SrcATop(src[i], row[i]);
                    break;
                case GBlendMode::kDstATop: 
                    for(int i = 0; i < count; ++i)
                    row[i] = DstATop(src[i], row[i]);
                    break;
                case GBlendMode::kXor:{
                    for(int i = 0; i < count; ++i)
                    row[i] = Xor(src[i], row[i]);
                    break;
                }
            }
        }
    }
}

#endif
