#include <iostream>
#include <vector>
#include <stack>
#include <algorithm>

#include "GBlendModes.h"
#include "Clip.h"

#include "GFinal.h"
#include "GBitmap.h"
#include "GCanvas.h"
#include "GMatrix.h"
#include "GColor.h"
#include "GMath.h"
#include "GPixel.h"
#include "GPoint.h"
#include "GPaint.h"
#include "GRect.h"
#include "GShader.h"
#include "GPath.h"
#include "ProxyShader.h"
#include "TriShader.h"
#include "ComposeShader.h"


class MyFinal : public GFinal {
public:
    MyFinal() {
        GMatrix identity;
        identity = GMatrix();
        myStack.push(identity);
    }

    void save() {
        GMatrix current = myStack.top();
        GMatrix copy(
            current[0], current[1], current[2],
            current[3], current[4], current[5]);

        myStack.push(copy);
    }

    void restore() {
        myStack.pop();
    }

    void concat(const GMatrix& matrix) {
        myStack.top().preConcat(matrix);
    }

    void drawPath(const GPath& path, const GPaint& paint) {
        GPath::Edger edger = GPath::Edger(path);
        GPath::Verb verb;
        std::vector<Edge> edges;

        GRect bound = GRect::MakeWH(fDevice.width(), fDevice.height());
        GPoint points[4];
        float d, dt;
        int N;

        while(1) {
            verb = edger.next(points);
            if (verb == GPath::Verb::kLine) {
                myStack.top().mapPoints(points, points, 2);
                clipLine(points[0], points[1], bound, edges);

            } else if (verb == GPath::Verb::kQuad) {
                myStack.top().mapPoints(points, points, 3);
                d = ((points[0] - points[1]) + (points[2] - points[1])).length();
                N = GCeilToInt(sqrt(d));
                dt = 1.0 / N;

                for (int i = 0; i < N; i++) {
                    // std::vector<Edge> * ptr = &edges;
                    clipLine(pointAtQuad(points[0], points[1], points[2], dt * i), 
                        pointAtQuad(points[0], points[1], points[2], dt * (i + 1)), bound, edges);
                    // std::cout << "!" << std::endl;
                }

            } else if (verb == GPath::Verb::kCubic) {
                myStack.top().mapPoints(points, points, 4);
                d = std::max(((points[0] - points[1]) + (points[2] - points[1])).length(),
                            ((points[1] - points[2]) + (points[3] - points[2])).length());
                N = GCeilToInt(sqrt(d * 3.0));
                dt = 1.0 / N;
                for (int i = 0; i < N; i++) {
                    clipLine(pointAtCubic(points[0], points[1], points[2], points[3], dt * i), 
                        pointAtCubic(points[0], points[1], points[2], points[3], dt * (i + 1)), bound, edges);
                    // std::cout << "@@" << std::endl;
                }
            } else if (verb == GPath::Verb::kDone) {
                // std::cout << "jump out?" << std::endl;
                break;
            }
            
        }

        GScanPath(edges, paint);    

    }

    GPoint pointAtQuad(const GPoint& p1, const GPoint& p2, const GPoint& p3, float t) {
        float tc = 1 - t;
        GPoint pa = GPoint::Make(tc * p1.fX + t * p2.fX, tc * p1.fY + t * p2.fY);
        GPoint pb = GPoint::Make(tc * p2.fX + t * p3.fX, tc * p2.fY + t * p3.fY);
        return GPoint::Make(tc * pa.fX + t * pb.fX, tc * pa.fY + t * pb.fY);
    }

    GPoint pointAtCubic(const GPoint& p1, const GPoint& p2, const GPoint& p3, const GPoint& p4, float t) {
        float tc = 1 - t;
        GPoint pa = GPoint::Make(tc * p1.fX + t * p2.fX, tc * p1.fY + t * p2.fY);
        GPoint pb = GPoint::Make(tc * p2.fX + t * p3.fX, tc * p2.fY + t * p3.fY);
        GPoint pc = GPoint::Make(tc * p3.fX + t * p4.fX, tc * p3.fY + t * p4.fY);
        GPoint pd = GPoint::Make(tc * pa.fX + t * pb.fX, tc * pa.fY + t * pb.fY);
        GPoint pe = GPoint::Make(tc * pb.fX + t * pc.fX, tc * pb.fY + t * pc.fY);
        return GPoint::Make(tc * pd.fX + t * pe.fX, tc * pd.fY + t * pe.fY);
    }    

    void GBlitRow (int y, int left, int right, const GPaint& paint) {
        left = std::max(0, left);
        right = std::min(fDevice.width(), right);

        GPixel* row = fDevice.getAddr(0, y);
        GShader* shader = paint.getShader();
        int count = right - left;

        if(shader != nullptr)
            if(!shader-> setContext(myStack.top())) return;
        Shade(paint, row + left, left, y, count);
    }


    static bool compareX(Edge e1, Edge e2) {
        return e1.curX < e2.curX;
    }

    void GScanPath(std::vector<Edge> edges, const GPaint& paint) {
        int count = edges.size();
        if (count < 2) {
            return;
        }

        // std::cout << "here?" << std::endl;
        std::sort(edges.begin(), edges.end());
        // std::cout << "come?" << std::endl;

        int y = edges[0].topY;
        int end = 0;
        while (count > 0) {
            
            while(edges[end].topY <= y && end < count) end++;

            std::sort(edges.begin(), edges.begin() + end, compareX);

            int wind = 0, x0 = 0, x1 = 0, previous = 0;
            for (int idx = 0; idx <= end; idx++) {
                previous = wind;
                wind += edges[idx].wind;
                
                if (previous == 0 && wind != 0) {
                    x0 = idx;
                } else if (previous != 0 && wind == 0) {
                    x1 = idx;
                    int l = GRoundToInt(edges[x0].curX), r = GRoundToInt(edges[x1].curX);
                    if(r > l) {
                        GBlitRow(y, l, r, paint);
                    }
                }
            }

            
            y++;
            int i = 0;
            while (i < end) {
                if(y >= edges[i].bottomY) {
                    edges.erase(edges.begin() + i);
                    end--;
                    count--;
                } else {
                    edges[i].curX += edges[i].slope;
                    i++;
                }
            }

        }
    }

    /**
     * Fill the entire canvas with a particular paint.
     */
    void drawPaint(const GPaint& paint) {
        GRect bounds = GRect::MakeWH(fDevice.width(), fDevice.height());
        drawRect(bounds, paint);    
    }

    /**
     * Draw a rectangular area by filling it with the provided paint.
     */    
    void drawRect(const GRect& rect, const GPaint& paint) {
        GPoint points[4] = {
            GPoint::Make(rect.left(), rect.top()),
            GPoint::Make(rect.right(), rect.top()),
            GPoint::Make(rect.right(), rect.bottom()),
            GPoint::Make(rect.left(), rect.bottom())
        };

        drawConvexPolygon(points, 4, paint);
    }

    /**
     * Draw a convex polygon to the canvas. The polygon is constructed by
     * forming edges between the provided points. The paint determines how the
     * new polygon is drawn with respect to the pixels already on the screen.
     */  
    void drawConvexPolygon(const GPoint srcPoints[], int count, const GPaint& paint) {
        GPoint points[count];
        myStack.top().mapPoints(points, srcPoints, count);

        GRect bounds = GRect::MakeWH(fDevice.width(), fDevice.height());
        std::vector<Edge> edges;

        for (int i = 0; i < count; ++i) {
            GPoint p0 = points[i];
            GPoint p1 = points[(i + 1) % count];

            clipLine(p0, p1, bounds, edges);
        }

        if (edges.size() == 0) {
            return;
        }

        assert(edges.size() >= 2);

        std::sort(edges.begin(), edges.end());

        int lastY = edges[edges.size() - 1].bottomY;

        Edge left = edges[0];
        Edge right = edges[1];

        int next = 2;

        float curY = left.topY;

        float leftX = left.curX;
        float rightX = right.curX;

        while (curY < lastY) {
            drawRow(curY, GRoundToInt(leftX), GRoundToInt(rightX), paint);
            curY++;

            if (curY > left.bottomY) {
                left = edges[next];
                next++;

                leftX = left.curX;
            } else {
                leftX += left.slope;
            }

            if (curY > right.bottomY) {
                right = edges[next];
                next++;

                rightX = right.curX;
            } else {
                rightX += right.slope;
            }
        }
    }

    void drawMesh(const GPoint verts[], const GColor colors[], const GPoint texs[],
                          int count, const int indices[], const GPaint& paint) {
        int n = 0;
        GPoint p0, p1, p2;

        for(int i = 0; i < count; i++) {

            p0 = verts[indices[n]];
            p1 = verts[indices[n + 1]];
            p2 = verts[indices[n + 2]];
            GPoint points[] = {p0, p1, p2};
            GColor *cols = nullptr;
            GPoint *textures = nullptr;

            if(colors) {
                cols = (GColor*) malloc(sizeof(GColor) * 3);
                cols[0] = colors[indices[n]];
                cols[1] = colors[indices[n + 1]];
                cols[2] = colors[indices[n + 2]];
            }
            if(texs) {
                textures = (GPoint*) malloc(sizeof(GPoint) * 3);
                textures[0] = texs[indices[n]];
                textures[1] = texs[indices[n + 1]];
                textures[2] = texs[indices[n + 2]];
            }
            
            drawTriangle(points, cols, textures, paint.getShader());
            n += 3;
        }
    }


    void drawQuad(const GPoint verts[4], const GColor colors[4], const GPoint texs[4],
                          int level, const GPaint& paint) {
        float factor = 1.0 / level;
        int count = 2 * level * level;
        int indices[count * 3];
        GPoint points[(level + 1) * (level + 1)];
        GPoint p03 = (verts[3] - verts[0]) * factor, p12 = (verts[2] - verts[1]) * factor;

        GPoint *textures = nullptr;
        GColor *cols = nullptr;

        GPoint left, right;
        
        left = verts[0];
        right = verts[1];

        for(int i = 0; i <= level; i++) {
            GPoint pos = (right - left) * factor;
            for(int j = 0; j <= level; j++) {
                points[i * (level + 1) + j] = left + pos * j;
            }
            left = left + p03;
            right = right + p12;
        }

        int pos = 0, i0, i1, i2, i3;
        for(int i = 0; i < level; i++) {
            for(int j = 0; j < level; j++) {
                i0 = i * (level + 1) + j;
                i1 = i0 + 1;
                i2 = i1 + level + 1;
                i3 = i2 - 1;
                indices[pos] = i0;
                indices[pos + 1] = i1;
                indices[pos + 2] = i3;
                indices[pos + 3] = i1;
                indices[pos + 4] = i3;
                indices[pos + 5] = i2;
                pos += 6;
            }
        }
        if(texs) {
            textures = (GPoint*) malloc(sizeof(GPoint) * (level + 1) * (level + 1));
            p03 = (texs[3] - texs[0]) * factor;
            p12 = (texs[2] - texs[1]) * factor;
            left = texs[0];
            right = texs[1];
            for(int i = 0; i <= level; i++) {
                GPoint pos = (right - left) * factor;
                for(int j = 0; j <= level; j++) {
                    textures[i * (level + 1) + j] = left + pos * j;
                }
                left = left + p03;
                right = right + p12;
            }
        }
        
        if(colors) {
            cols = (GColor*) malloc(sizeof(GColor) * (level + 1) * (level + 1));
            float a03 = (colors[3].fA - colors[0].fA) * factor, r03 = (colors[3].fR - colors[0].fR) * factor, 
                g03 = (colors[3].fG - colors[0].fG) * factor, b03 = (colors[3].fB - colors[0].fB) * factor;
            float a12 = (colors[2].fA - colors[1].fA) * factor, r12 = (colors[2].fR - colors[1].fR) * factor, 
                g12 = (colors[2].fG - colors[1].fG) * factor, b12 = (colors[2].fB - colors[1].fB) * factor;
            float alr, rlr, glr, blr;
            GColor cl = colors[0], cr = colors[1];

            for(int i = 0; i <= level; i++) {
                alr = (cr.fA - cl.fA) * factor;
                rlr = (cr.fR - cl.fR) * factor;
                glr = (cr.fG - cl.fG) * factor;
                blr = (cr.fB - cl.fB) * factor;
                for(int j = 0; j <= level; j++) {
                    cols[i * (level + 1) + j] = GColor::MakeARGB(cl.fA + alr * j, cl.fR + rlr * j,
                                                                cl.fG + glr * j, cl.fB + blr * j);
                }
                cl.fA += a03;
                cl.fR += r03;
                cl.fG += g03;
                cl.fB += b03;
                cr.fA += a12;
                cr.fR += r12;
                cr.fG += g12;
                cr.fB += b12;
            }
        }
        drawMesh(points, cols, textures, count, indices, paint);
      
    }

    void drawTriangle(const GPoint pts[3], const GColor colors[3], const GPoint tex[3], GShader* originalShader) {
        TriShader tri(pts, colors);
        ProxyShader proxy(originalShader, pts, tex);
        CompositeShader comp(&tri, &proxy);
        GShader *s;
        if(colors)  s= &tri;
        if(tex && !colors) s = &proxy;
        if(colors && tex) s = &comp;
        GPaint paint(s);
        drawConvexPolygon(pts, 3, s);
    }

    std::unique_ptr<GShader> createRadialGradient(GPoint center, float radius,
                                                          const GColor colors[], int count,
                                                          GShader::TileMode mode) override {
        return nullptr;
    }

    void addLine(GPath* path, GPoint p0, GPoint p1, float width, CapType) override {

    }  


private:
    const GBitmap fDevice;

    std::stack<GMatrix> myStack;

    void drawRow(int y, int xLeft, int xRight, const GPaint& paint) {
        if (xLeft >= xRight) {
            return;
        }

        xLeft = std::max(0, xLeft);
        xRight = std::min(fDevice.width(), xRight);

        BlendProc blendProc = getBlendProc(paint.getBlendMode());

        GShader* shader = paint.getShader();
        if (shader == nullptr) {
            GColor color = paint.getColor().pinToUnit();
            GPixel source = convertColorToPixel(color);

            for (int x = xLeft; x < xRight; ++x) {
                GPixel* addr = fDevice.getAddr(x, y);
                *addr = blendProc(source, *addr);
            }
        } else {
            if (!shader->setContext(myStack.top())) {
                return;
            }

            int count = xRight - xLeft;
            GPixel shaded[count];
            shader->shadeRow(xLeft, y, count, shaded);

            for (int x = xLeft; x < xRight; ++x) {
                GPixel* addr = fDevice.getAddr(x, y);
                *addr = blendProc(shaded[x - xLeft], *addr);
            }
        }
    }
};


std::unique_ptr<GFinal> GCreateFinal() {

    return std::unique_ptr<GFinal>(new MyFinal());
}