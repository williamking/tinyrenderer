#ifndef __OUR_GL_H__
#define __OUR_GL_H__

#include "tgaimage.h"
#include "geometry.h"

extern Matrix ModelView;
extern Matrix Viewport;
extern Matrix Projection;

void viewport(int x, int y, int w, int h);
void projection(float coeff=0.f); // coeff = -1/c
void lookat(Vec3f eye, Vec3f center, Vec3f up);

struct VertexInfo {
    int iface;
    int nthvert;
    Vec3f pos;
}

struct IShader {
    virtual ~IShader();
    virtual Vec4f vertex(VertexInfo) = 0;
    virtual bool fragment(Vec3f bar, TGAColor &color) = 0;
};

void triangle(Vec4f *pts, IShader &shader, TGAImage &image, TGAImage &zbuffer);
// void triangle(Vec3f t0, Vec3f t1, Vec3f t2, TGAImage &image, TGAColor color);

#endif //__OUR_GL_H__