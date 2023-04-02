#include "tgaimage.h";
using std;

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);


function line(int x0, int x1, int y0, int y1, TGAImage &image, TGAColor color)
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

    int derror = 2 * (y1 - y0);
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

int main(int argc, char** argv) {
    TGAImage image(100, 100, TGAImage::RGB);
    for (int i=0; i<1000000; i++) {
        line(13, 20, 80, 40, image, white);
        line(20, 13, 40, 80, image, red);
        line(80, 40, 13, 20, image, red);
    }
    image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    image.write_tga_file("output.tga");
    return 0;
}