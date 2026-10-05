#include <cmath>
#include <cstdlib>
#include <ctime>
#include "tgaimage.h"

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
    int dy = y2 - y1;

    for (int x = x1; x < x2; x++) {
        float t = (x - x1) / static_cast<float> (x2 - x1);
        float y = y1 + (t * (y2 - y1));
        if (steep) {
            framebuffer.set(y, x, color);
        } else {
            framebuffer.set(x, y, color);
        }
    }
}

int main(int argc, char** argv) {
    constexpr int width  = 64;
    constexpr int height = 64;
    TGAImage framebuffer(width, height, TGAImage::RGB);

    int ax =  7, ay =  3;
    int bx = 12, by = 37;
    int cx = 62, cy = 53;

    framebuffer.set(ax, ay, white);
    framebuffer.set(bx, by, white);
    framebuffer.set(cx, cy, white);
    
    // draw_line(framebuffer, ax, ay, bx, by, blue);
    // draw_line(framebuffer, ax, ay, cx, cy, red);
    // draw_line(framebuffer, cx, cy, bx, by, green);

    std::srand(std::time({}));
    for (int i=0; i<(1<<24); i++) {
        int ax = rand()%width, ay = rand()%height;
        int bx = rand()%width, by = rand()%height;
        draw_line(framebuffer, ax, ay, bx, by, { static_cast<uint8_t>(rand()%255), static_cast<uint8_t>(rand()%255), static_cast<uint8_t>(rand()%255), static_cast<uint8_t>(rand()%255) });
    }

    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}
