#include <vector>
#include <cmath>
#include "our_gl.h"
#include "model.h"

extern Model *model;
extern Vec3f light_dir;
extern Vec3f       eye;
extern Vec3f    center;
extern Vec3f        up;

// 环境光强度
extern float laka;
// 漫反射光强度
extern float ldkd;
// 光源强度
extern float lsks;

struct PhongShader : public IShader {
    Vec3f dirs[3];
    mat<2,3,float> varying_uv;
    mat<4,4,float> uniform_M;   //  Projection*ModelView
    mat<4,4,float> uniform_MIT; // (Projection*ModelView).invert_transpose()

    // 顶点着色器，参数为面索引和第几个顶点
    virtual Vec4f vertex(VertexInfo info) {
        int iface = info.iface;
        int nthvert = info.nthvert;
        // 记录每个顶点的法向量
        dirs[nthvert] = model->normal(iface, nthvert);
        varying_uv.set_col(nthvert, model->uv(iface, nthvert));
        Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert)); // read the vertex from .obj file
        Vec4f result = Viewport       *Projection*ModelView*gl_Vertex;

        // cout << "origin vertex " << gl_Vertex << endl;
        // cout << "screen vertex " << result << endl;
        
        return result; // transform it to screen coordinates

    }

    virtual bool fragment(Vec3f bar, TGAColor &color) {
        Vec2f uv = varying_uv*bar;
        Vec3f n = proj<3>(uniform_MIT * embed<4>(model->normal(uv))).normalize();
        Vec3f l = proj<3>(uniform_M * embed<4>(light_dir)).normalize();

        Vec3f reflect = (l - n * (2 * (n * l))).normalize() * -1;

        float intensity = laka + std::max(0.f, ldkd * (n * l)) + lsks * pow(std::max(0.f, reflect * eye), model->specular(uv));
   
        // 镜面反射强度，这里的计算原理是，在观察空间中，视野方向向量是（0,0,-1），如果反射的向量是r，那么直接计算器反射的强度则是r点乘(0,0,-1)，即-r.z，前面的reflect变量正是计算-r的向量。
        float spec = pow(std::max(reflect.z, 0.0f), model->specular(uv));
        // 漫反射强度
        float diff = std::max(0.f, n*l);
        TGAColor c = model->diffuse(uv);
        color = c;
        for (int i=0; i<3; i++) color[i] = std::min<float>(5 + c[i]*(diff + .6*spec), 255);

        // float intensity = laka + std::max(0.f, ldkd * (n * l)) + lsks * pow(std::max(0.f, reflect.z), model->specular(uv));
        // color = model->diffuse(uv) * intensity; // well duh
        // cout << "intensity: " << intensity << endl;

        return false;                              // no, we do not discard this pixel
    }
};

struct GraundShader : public IShader {
    Vec3f varying_intensity; // written by vertex shader, read by fragment shader

    // 顶点着色器，参数为面索引和第几个顶点
    virtual Vec4f vertex(VertexInfo info) {
        int iface = info.iface;
        int nthvert = info.nthvert;
        // 计算每个顶点的光照
        varying_intensity[nthvert] = std::max(0.f, model->normal(iface, nthvert)*light_dir); // get diffuse lighting intensity
        Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert)); // read the vertex from .obj file
        Vec4f result = Viewport*Projection*ModelView*gl_Vertex;

        // cout << "origin vertex " << gl_Vertex << endl;
        // cout << "screen vertex " << result << endl;
        
        return result; // transform it to screen coordinates

    }

    virtual bool fragment(Vec3f bar, TGAColor &color) {
        float intensity = varying_intensity*bar;   // interpolate intensity for the current pixel
        color = TGAColor(255, 255, 255) * intensity; // well duh
        // cout << "intensity: " << intensity << endl;
        return false;                              // no, we do not discard this pixel
    }
};