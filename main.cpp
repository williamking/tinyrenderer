#include "tgaimage.h";
#include <vector>
using std;

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);


void line(int x0, int x1, int y0, int y1, TGAImage &image, TGAColor color)
{
    bool steep = false;
    
    if (abs(x0 - x1) < abs(y0 - y1)) {
        swap(x0, y0);
        swap(x1, y1);
    }

    if (x0 > x1)
    {
        swap(x0, x1);
        swap(y0, y1);
    }

    int dx  = x1 - x0;

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
    } else {
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

void triangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage &image, TGAColor color) {
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

    for (int y = t0.y; y <= t2.y; ++y)
    {
        bool isUp = y <= t1.y;
        int halfHeight = isUp ? t1.y - t0.y : t2.y - t1.y;
        int x1 = Math.round(x0.x + (float)(x2.x - x0.x) * (y - t0.y) / height));
        int x2 = Math.round(isUp ? (x0.x + (float)(x1.x - x0.x) * (y - t0.y) / halfHeight) : (x1.x + (float)(x2.x - x1.x) * (y - t1.y) / halfHeight));

        if (x1 > x2) {
            swap(x1, x2);
        }

        for (int x = x1; x <= x2; ++x)
        {
            image.set(x, y, color);
        }
    }
}


int main(int argc, char** argv) {
    TGAImage image(100, 100, TGAImage::RGB);
    // for (int i=0; i<1000000; i++) {
    //     line(13, 20, 80, 40, image, white);
    //     line(20, 13, 40, 80, image, red);
    //     line(80, 40, 13, 20, image, red);
    // }

    Vec2i t0[3] = {Vec2i(10, 70),   Vec2i(50, 160),  Vec2i(70, 80)}; 
    Vec2i t1[3] = {Vec2i(180, 50),  Vec2i(150, 1),   Vec2i(70, 180)}; 
    Vec2i t2[3] = {Vec2i(180, 150), Vec2i(120, 160), Vec2i(130, 180)}; 
    triangle(t0[0], t0[1], t0[2], image, red); 
    triangle(t1[0], t1[1], t1[2], image, white); 
    triangle(t2[0], t2[1], t2[2], image, green);

    image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    image.write_tga_file("output.tga");
    return 0;
}