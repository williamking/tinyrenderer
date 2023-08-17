#include "tgaimage.h"
#include "geometry.h"
#include "model.h"
#include <vector>
#include <cmath>
#include <iostream>
using namespace std;

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);
const int width = 600;
const int height = 600;

Model *model = NULL;

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

void triangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage &image, TGAColor color)
{
    if (t0.y > t1.y)
    {
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
        bool isUp = !(i > t1.y - t0.y || t1.y == t0.y);
        int halfHeight = isUp ? t1.y - t0.y : t2.y - t1.y;
        float alpha = (float)i / height;
        float beta = isUp ? (float)i / halfHeight : (float)(i + t0.y - t1.y) / halfHeight;
        // int x1 = round(t0.x + (float)(t2.x - t0.x) * (y - t0.y) / height);
        // int x2 = round(isUp ? (t0.x + (float)(t1.x - t0.x) * (y - t0.y) / halfHeight) : (t1.x + (float)(t2.x - t1.x) * (y - t1.y) / halfHeight));
        Vec2i v1 = t0 + (t2 - t0) * alpha;
        Vec2i v2 = isUp ? (t0 + (t1 - t0) * beta) : (t1 + (t2 - t1) * beta);

        if (v1.x > v2.x)
        {
            swap(v1, v2);
        }

        for (int x = v1.x; x <= v2.x; ++x)
        {
            int y = i + t0.y;
            image.set(x, y, color);
        }
    }
}

void drawModelLine(TGAImage &image)
{
    model = new Model("../obj/african_head/african_head.obj"); // 命令行控制方式构造model

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

    // image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    // image.write_tga_file("output2.tga");

    delete model;
}

void drawModelColor(TGAImage &image)
{
    model = new Model("../obj/african_head/african_head.obj");
    for (int i = 0; i < model->nfaces(); i++)
    {
        std::vector<int> face = model->face(i);
        Vec2i screen_coords[3];
        for (int j = 0; j < 3; j++)
        {
            Vec3f world_coords = model->vert(face[j]);
            screen_coords[j] = Vec2i((world_coords.x + 1.) * width / 2., (world_coords.y + 1.) * height / 2.);
        }
        triangle(screen_coords[0], screen_coords[1], screen_coords[2], image, TGAColor(rand() % 255, rand() % 255, rand() % 255, 255));
    }
    delete model;
}

int main(int argc, char **argv)
{
    TGAImage image(width, height, TGAImage::RGB);

    // 测试画线
    // line(13, 20, 80, 40, image, white);
    // line(20, 13, 40, 80, image, red);

    // drawModelLine(image); // 代码方式构造model

    // 画三角形并填充
    // Vec2i t0[3] = {Vec2i(10, 70), Vec2i(50, 160), Vec2i(70, 80)};
    // Vec2i t1[3] = {Vec2i(180, 50), Vec2i(150, 1), Vec2i(70, 180)};
    // Vec2i t2[3] = {Vec2i(180, 150), Vec2i(120, 160), Vec2i(130, 180)};
    // triangle(t0[0], t0[1], t0[2], image, red);
    // triangle(t1[0], t1[1], t1[2], image, white);
    // triangle(t2[0], t2[1], t2[2], image, green);

    // 绘制模型
    // drawModelColor(image);

    image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    image.write_tga_file("output.tga");

    return 0;
}