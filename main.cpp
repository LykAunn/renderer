#include <cmath>
#include <cstdlib>
#include <ctime>
#include "tgaimage.h"
#include "objParser.cpp"

constexpr TGAColor white   = {255, 255, 255, 255}; // attention, BGRA order
constexpr TGAColor green   = {  0, 255,   0, 255};
constexpr TGAColor red     = {  0,   0, 255, 255};
constexpr TGAColor blue    = {255, 128,  64, 255};
constexpr TGAColor yellow  = {  0, 200, 255, 255};

void draw_line(TGAImage& framebuffer,int x1, int y1, int x2, int y2, TGAColor color) {
    bool steep = std::abs(x1-x2) < std::abs(y1 - y2);

    if (steep) { // If line is steep, transpose x and y
        std::swap(x1, y1);
        std::swap(x2, y2);
    }

    if (x1 > x2) { // Swap start and end points
        std::swap(x1,x2);
        std::swap(y1, y2);
    }
    int dx = x2 - x1;
    int dy = std::abs(y2 - y1);

    int y = y1;
    int ierror = 0;
    for (int x = x1; x < x2; x++) {
        if (steep) {
            framebuffer.set(y, x, color);
        } else {
            framebuffer.set(x, y, color);
        }
        ierror += 2 * dy;
        if (ierror > dx) {
            y += y2 > y1 ? 1 : -1;
            ierror -= 2 * dx;
        }
    }
}

void draw_rectangle(TGAImage& framebuffer, Vec3v v1, Vec3v v2, Vec3v v3, int width, int height) {
    Vec3v v[3] = {v1, v2, v3};
    int vx[3], vy[3];
    for (int i = 0; i < 3; i ++) {
        vx[i] = static_cast<int>(((v[i].x + 1 )* width) / 2);
        vy[i] = static_cast<int>(((v[i].y + 1) * width) / 2);
    }

    draw_line(framebuffer, vx[0], vy[0], vx[1], vy[1], blue);
    draw_line(framebuffer, vx[2], vy[2], vx[1], vy[1], blue);
    draw_line(framebuffer, vx[0], vy[0], vx[2], vy[2], blue);
    
}

int main(int argc, char** argv) {
    constexpr int width  = 1024;
    constexpr int height = 1024;
    TGAImage framebuffer(width, height, TGAImage::RGB);
    Parser object_parser = Parser();

    int ax =  7, ay =  3;
    int bx = 12, by = 37;
    int cx = 62, cy = 53;

    // framebuffer.set(ax, ay, white);
    // framebuffer.set(bx, by, white);
    // framebuffer.set(cx, cy, white);

    object_parser.load("./diablo3_pose.obj");
    // object_parser.load("./african_head.obj");


    for (Vec3f triangle : object_parser.faces) {
        draw_rectangle(framebuffer, object_parser.vertices[triangle.v0],
             object_parser.vertices[triangle.v1], object_parser.vertices[triangle.v2]
            , width, height);
    }
    
    // draw_line(framebuffer, ax, ay, bx, by, blue);
    // draw_line(framebuffer, ax, ay, cx, cy, red);
    // draw_line(framebuffer, cx, cy, bx, by, green);

    // std::srand(std::time({}));
    // for (int i=0; i<(1<<24); i++) {
    //     int ax = rand()%width, ay = rand()%height;
    //     int bx = rand()%width, by = rand()%height;
    //     draw_line(framebuffer, ax, ay, bx, by, { static_cast<uint8_t>(rand()%255), static_cast<uint8_t>(rand()%255), static_cast<uint8_t>(rand()%255), static_cast<uint8_t>(rand()%255) });
    // }

    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}
