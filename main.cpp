#include "tgaimage.h"
#include "geometry.h"
#include "model.h"
#include "our_gl.h"
#include <vector>
#include <cmath>
#include <iostream>
using namespace std;

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);
const int width = 600;
const int height = 600;
float *zBuffer = NULL;

Model *model = NULL;

Vec3f light_dir(1, 1, 1);
Vec3f eye(1, 1, 3);
Vec3f center(0, 0, 0);
Vec3f up(0, 1, 0);

void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color)
{
    bool steep = false;

    if (abs(x0 - x1) < abs(y0 - y1))
    {
        swap(x0, y0);
        swap(x1, y1);
        steep = true;
    }

    if (x0 > x1)
    {
        swap(x0, x1);
        swap(y0, y1);
    }

    int dx = x1 - x0;

    int derror = 2 * abs(y1 - y0);
    int error = 0;
    int y = y0;

    if (steep)
    {
        for (int x = x0; x <= x1; ++x)
        {
            image.set(y, x, color);

            error += derror;

            if (error > dx)
            {
                y += (y1 > y0) ? 1 : -1;
                error -= 2 * dx;
            }
        }
    }
    else
    {
        for (int x = x0; x <= x1; ++x)
        {
            image.set(x, y, color);

            error += derror;

            if (error > dx)
            {
                y += (y1 > y0) ? 1 : -1;
                error -= 2 * dx;
            }
        }
    }
}

void drawModelLine(TGAImage &image)
{
    model = new Model("../obj/african_head.obj"); // 命令行控制方式构造model

    for (int i = 0; i < model->nfaces(); i++)
    {
        std::vector<int> face = model->face(i); // 创建face数组用于保存一个face的三个顶点坐标
        for (int j = 0; j < 3; j++)
        {
            Vec3f v0 = model->vert(face[j]);
            Vec3f v1 = model->vert(face[(j + 1) % 3]);
            // 根据顶点v0和v1画线
            // 先要进行模型坐标到屏幕坐标的转换。  (-1,-1)对应(0,0)   (1,1)对应(width,height)
            int x0 = (v0.x + 1.) * width / 2.;
            int y0 = (v0.y + 1.) * height / 2.;
            int x1 = (v1.x + 1.) * width / 2.;
            int y1 = (v1.y + 1.) * height / 2.;
            // 画线
            line(x0, y0, x1, y1, image, white);
        }
    }

    image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    image.write_tga_file("output2.tga");

    delete model;
}

void drawModelColor(TGAImage &image)
{
    model = new Model("../obj/african_head/african_head.obj");

    for (int i = width * height; i--; zBuffer[i] = -std::numeric_limits<float>::max())
        ;

    for (int i = 0; i < model->nfaces(); i++)
    {
        std::vector<int> face = model->face(i);
        Vec3f screen_coords[3];
        for (int j = 0; j < 3; j++)
        {
            Vec3f world_coords = model->vert(face[j]);
            screen_coords[j] = Vec3f((world_coords.x + 1.) * width / 2., (world_coords.y + 1.) * height / 2., world_coords.z);
        }
        // triangle(screen_coords[0], screen_coords[1], screen_coords[2], image, TGAColor(rand()%255, rand()%255, rand()%255, 255));
    }
    delete model;
    model = NULL;
    zBuffer = NULL;
}

Vec3f world2screen(Vec3f v)
{
    return Vec3f(int((v.x + 1.) * width / 2. + .5), int((v.y + 1.) * height / 2. + .5), v.z);
}

void drawModelLight(TGAImage &image)
{
    Vec3f light_dir(0, 0, -1);

    for (int i = width * height; i--; zBuffer[i] = -std::numeric_limits<float>::max())
        ;

    model = new Model("../obj/african_head/african_head.obj");
    for (int i = 0; i < model->nfaces(); i++)
    {
        std::vector<int> face = model->face(i);
        Vec3f screen_coords[3];
        Vec3f world_coords_list[3];

        for (int j = 0; j < 3; j++)
        {
            Vec3f world_coords = model->vert(face[j]);
            screen_coords[j] = world2screen(world_coords);
            world_coords_list[j] = world_coords;
        }

        Vec3f normal_vector = cross((world_coords_list[2] - world_coords_list[0]), (world_coords_list[1] - world_coords_list[0]));
        normal_vector.normalize();
        float intensity = normal_vector * light_dir;

        if (intensity > 0)
        {
            // triangle(screen_coords[0], screen_coords[1], screen_coords[2], image, TGAColor(intensity * 255, intensity * 255, intensity * 255, 255));
        }
    }
    delete model;
    model = NULL;
    zBuffer = NULL;
}

struct PhongShader : public IShader
{
    Vec3f dirs[3];

    // 顶点着色器，参数为面索引和第几个顶点
    virtual Vec4f vertex(int iface, int nthvert)
    {
        // 计算每个顶点的光照
        dirs[nthvert] = model->normal(iface, nthvert);
        Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert)); // read the vertex from .obj file
        Vec4f result = Viewport * Projection * ModelView * gl_Vertex;

        // cout << "origin vertex " << gl_Vertex << endl;
        // cout << "screen vertex " << result << endl;

        return result; // transform it to screen coordinates
    }

    virtual bool fragment(Vec3f bar, TGAColor &color)
    {
        Vec3f normal(0, 0, 0);

        for (int i = 0; i < 3; ++i)
        {
            normal = normal + dirs[i];
        }

        normal = normal.normalize();

        float intensity = std::max(0.f, normal * light_dir);

        color = TGAColor(255, 255, 255) * intensity; // well duh
        // cout << "intensity: " << intensity << endl;
        return false; // no, we do not discard this pixel
    }
};

struct GraundShader : public IShader
{
    Vec3f varying_intensity; // written by vertex shader, read by fragment shader

    // 顶点着色器，参数为面索引和第几个顶点
    virtual Vec4f vertex(int iface, int nthvert)
    {
        // 计算每个顶点的光照
        varying_intensity[nthvert] = std::max(0.f, model->normal(iface, nthvert) * light_dir); // get diffuse lighting intensity
        Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert));                               // read the vertex from .obj file
        Vec4f result = Viewport * Projection * ModelView * gl_Vertex;

        // cout << "origin vertex " << gl_Vertex << endl;
        // cout << "screen vertex " << result << endl;

        return result; // transform it to screen coordinates
    }

    virtual bool fragment(Vec3f bar, TGAColor &color)
    {
        float intensity = varying_intensity * bar;   // interpolate intensity for the current pixel
        color = TGAColor(255, 255, 255) * intensity; // well duh
        // cout << "intensity: " << intensity << endl;
        return false; // no, we do not discard this pixel
    }
};

void drawModelWithShader(TGAImage &image)
{
    // for (int i=width*height; i--; zBuffer[i] = -std::numeric_limits<float>::max());
    TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);
    // PhongShader shader;
    GraundShader shader;

    model = new Model("../obj/african_head/african_head.obj");
    for (int i = 0; i < model->nfaces(); i++)
    {
        std::vector<int> face = model->face(i);
        Vec4f screen_coords[3];

        for (int j = 0; j < 3; j++)
        {
            screen_coords[j] = shader.vertex(i, j);
        }

        triangle(screen_coords, shader, image, zbuffer);
    }
    delete model;
    model = NULL;
    zBuffer = NULL;
}

int main(int argc, char **argv)
{
    TGAImage image(width, height, TGAImage::RGB);
    zBuffer = new float[width * height];

    lookat(eye, center, up);
    viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
    projection(-1.f / (eye - center).norm());
    light_dir.normalize();

    // 测试画线
    // line(13, 20, 80, 40, image, white);
    // line(20, 13, 40, 80, image, red);
    // line(80, 40, 13, 20, image, red);

    // drawModelLine(image); //代码方式构造model

    // 画三角形并填充
    // Vec2i t0[3] = {Vec2i(10, 70),   Vec2i(50, 160),  Vec2i(70, 80)};
    // Vec2i t1[3] = {Vec2i(180, 50),  Vec2i(150, 1),   Vec2i(70, 180)};
    // Vec2i t2[3] = {Vec2i(180, 150), Vec2i(120, 160), Vec2i(130, 180)};
    // triangle2d(t0[0], t0[1], t0[2], image, red);
    // triangle2d(t1[0], t1[1], t1[2], image, white);
    // triangle2d(t2[0], t2[1], t2[2], image, green);

    // 绘制模型
    // drawModelColor(image);

    // drawModelLight(image);

    drawModelWithShader(image);

    image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    image.write_tga_file("output.tga");

    delete zBuffer;
    return 0;
}