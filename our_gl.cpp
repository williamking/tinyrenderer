#include "our_gl.h"
#include <cmath>

using namespace std;

IShader::~IShader() {}

Matrix ModelView;
Matrix Viewport;
Matrix Projection;

// 计算点p的质心坐标
Vec3f barycentric(Vec2f *pts, Vec2f P) {
    Vec2f AC = pts[2] - pts[0];
    Vec2f AB = pts[1] - pts[0];
    Vec2f BC = pts[2] - pts[1];
    Vec2f PA = pts[0] - P;

    Vec3f u = cross(Vec3f(AC.x, AB.x, PA.x), Vec3f(AC.y, AB.y, PA.y));

    if (abs(u.z) < 1)
    {
        return Vec3f(-1, 1, 1);
    }

    return Vec3f(1.f-(u.x+u.y)/u.z, u.y/u.z, u.x/u.z);
}

// 观察视图矩阵
void lookat(Vec3f eye, Vec3f center, Vec3f up) {
    Vec3f z = (eye-center).normalize();
    Vec3f x = cross(up,z).normalize();
    Vec3f y = cross(z,x).normalize();
    Matrix Minv = Matrix::identity();
    Matrix Tr   = Matrix::identity();
    for (int i=0; i<3; i++) {
        Minv[0][i] = x[i];
        Minv[1][i] = y[i];
        Minv[2][i] = z[i];
        // Tr[i][3] = -center[i];
        Minv[i][3] = -center[i];
    }
    ModelView = Minv*Tr;
}

// 视口矩阵
void viewport(int x, int y, int w, int h) {
    float depth = 255.f;
    Matrix m = Matrix::identity();
    m[0][3] = x+w/2.f;
    m[1][3] = y+h/2.f;
    m[2][3] = depth/2.f;

    m[0][0] = w/2.f;
    m[1][1] = h/2.f;
    m[2][2] = depth/2.f;
    Viewport = m;
}

// 用位于 z 轴上距离原点 c 的相机计算中心投影
void projection(float c) {
    Matrix m = Matrix::identity();
    
    m[3][2] = c;
    
    Projection = m;
}

void triangle2d(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage &image, TGAColor color) {
    if (t0.y >t1.y) {
        swap(t0, t1);
    }

    if (t1.y > t2.y)
    {
        swap(t1, t2);
    }

    if (t0.y > t1.y)
    {
        swap(t0, t1);
    }

    int height = t2.y - t0.y;

    for (int i = 0; i < height; ++i)
    {
        bool isUp = !(i>t1.y-t0.y || t1.y==t0.y);
        int halfHeight = isUp ? t1.y - t0.y : t2.y - t1.y;
        float alpha = (float)i / height;
        float beta = isUp ? (float)i / halfHeight : (float)(i + t0.y - t1.y) / halfHeight;
        // int x1 = round(t0.x + (float)(t2.x - t0.x) * (y - t0.y) / height);
        // int x2 = round(isUp ? (t0.x + (float)(t1.x - t0.x) * (y - t0.y) / halfHeight) : (t1.x + (float)(t2.x - t1.x) * (y - t1.y) / halfHeight));
        Vec2i v1 = t0 + (t2 - t0) * alpha;
        Vec2i v2 = isUp ? (t0 + (t1 - t0) * beta) : (t1 + (t2 - t1) * beta);

        if (v1.x > v2.x) {
            swap(v1, v2);
        }

        for (int x = v1.x; x <= v2.x; ++x)
        {
            int y = i + t0.y;
            image.set(x, y, color);
        }
    }
}

// void triangle(Vec3f t0, Vec3f t1, Vec3f t2, TGAImage &image, TGAColor color) {
//     Vec3f originPts[3];
//     originPts[0] = t0;
//     originPts[1] = t1;
//     originPts[2] = t2;

//     if (t0.y >t1.y) {
//         swap(t0, t1);
//     }

//     if (t1.y > t2.y)
//     {
//         swap(t1, t2);
//     }

//     if (t0.y > t1.y)
//     {
//         swap(t0, t1);
//     }

//     int height = round(t2.y - t0.y);

//     for (int i = 0; i < height; ++i)
//     {
//         bool isUp = !(i>t1.y-t0.y || t1.y==t0.y);
//         float halfHeight = isUp ? t1.y - t0.y : t2.y - t1.y;
//         float alpha = (float)i / height;
//         float beta = isUp ? (float)i / halfHeight : (float)(i + t0.y - t1.y) / halfHeight;
//         // int x1 = round(t0.x + (float)(t2.x - t0.x) * (y - t0.y) / height);
//         // int x2 = round(isUp ? (t0.x + (float)(t1.x - t0.x) * (y - t0.y) / halfHeight) : (t1.x + (float)(t2.x - t1.x) * (y - t1.y) / halfHeight));
//         Vec3f v1 = t0 + (t2 - t0) * alpha;
//         Vec3f v2 = isUp ? (t0 + (t1 - t0) * beta) : (t1 + (t2 - t1) * beta);

//         if (v1.x > v2.x) {
//             swap(v1, v2);
//         }

//         for (int x = round(v1.x); x <= round(v2.x); ++x)
//         {
//             int y = round(i + t0.y);
//             Vec3f p = Vec3f(x, y, 0);
//             Vec3f centric = barycentric(originPts, p);
//             float z = originPts[0].z * centric.x + originPts[1].z * centric.y + originPts[2].z * centric.z; 
//             if (z > zBuffer[y * width + x])
//             {
//                 zBuffer[y * width + x] = z;
//                 image.set(x, y, color);
//             }
//         }
//     }
// }



void triangle(Vec4f *pts, IShader &shader, TGAImage &image, TGAImage &zbuffer) {
    // cout << "draw triangle" << endl;
    Vec2f bboxmin( std::numeric_limits<float>::max(),  std::numeric_limits<float>::max());
    Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
    Vec2f clamp(image.get_width()-1, image.get_height()-1);
    for (int i=0; i<3; i++) {
        for (int j=0; j<2; j++) {
            bboxmin[j] = std::max(0.f,      std::min(bboxmin[j], pts[i][j]/pts[i][3]));
            bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], pts[i][j]/pts[i][3]));
        }
    }

    Vec2i p;
    TGAColor color;
    int width = image.get_width();

    Vec2f pt2s[3];
    pt2s[0] = Vec2f(pts[0][0]/pts[0][3], pts[0][1]/pts[0][3]);
    pt2s[1] = Vec2f(pts[1][0]/pts[1][3], pts[1][1]/pts[1][3]);
    pt2s[2] = Vec2f(pts[2][0]/pts[2][3], pts[2][1]/pts[2][3]);

    // cout << "min: " << bboxmin.x << " " << bboxmin.y << endl;
    // cout << "max: " << bboxmax.x << " " << bboxmax.y << endl;

    for (p.x = bboxmin.x; p.x <= bboxmax.x; ++p.x) {
        for (p.y = bboxmin.y; p.y <= bboxmax.y; ++p.y)
        {
            Vec3f centric = barycentric(pt2s, p);
            // Vec3f centric = barycentric(proj<2>(pts[0]/pts[0][3]), proj<2>(pts[1]/pts[1][3]), proj<2>(pts[2]/pts[2][3]), proj<2>(p));
            if (centric.x >= 0 && centric.y >= 0 && centric.z >= 0) {
                float z = pts[0][2] * centric.x + pts[1][2] * centric.y + pts[2][2] * centric.z; 
                float w = pts[0][3] * centric.x + pts[1][3] * centric.y + pts[2][3] * centric.z;

                int depth = std::max(0, std::min(255, int(z / w + 0.5)));
                int index = int(p.y * width + p.x + 0.5);

                if (depth > zbuffer.get(p.x, p.y).bgra[0])
                {
                    // cout << color.raw[0] << endl;
                    if (!shader.fragment(centric, color)) {
                        image.set(p.x, p.y, color);
                        zbuffer.set(p.x, p.y, TGAColor(depth));
                        // cout << color.raw[0] << endl;
                    }
                }
            }
        }
    }
}
