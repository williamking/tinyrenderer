#include <vector>
#include "shader.h"

Model *model = NULL;
Vec3f light_dir(1,1,1);
Vec3f       eye(1,1,3);
Vec3f    center(0,0,0);
Vec3f        up(0,1,0);
// 环境光强度
float laka = 30.0 / 255;
// 漫反射光强度
float ldkd = 1;
// 光源强度
float lsks = 0.6;
