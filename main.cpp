#include <cmath>
#include <cstdlib>
#include <ctime>
#include "tgaimage.h"
#include "objParser.cpp"
#include <random>

TGAColor random_color() {
    static std::mt19937 rng(std::random_device{}());
    static std::uniform_int_distribution<int> dist(0, 255);

    return TGAColor{
        static_cast<std::uint8_t>(dist(rng)), // R
        static_cast<std::uint8_t>(dist(rng)), // G
        static_cast<std::uint8_t>(dist(rng)), // B
        255                                  // Alpha
    };
}

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
            y += (y2 > y1 ? 1 : -1);
            ierror -= 2 * dx;
        }
    }
}

void draw_triangle_scan(TGAImage& framebuffer, int x1, int y1, int x2, int y2, int x3, int y3, TGAColor color) {
    if (y1 > y2) { std::swap(y1, y2); std::swap(x1, x2);}
    if (y1 > y3) { std::swap(y1, y3); std::swap(x1, x3);}
    if (y2 > y3) { std::swap(y2, y3); std::swap(x2, x3);}

    for (int y = y1; y <= y2; y++) {
        float a = static_cast<float>((y - y1)) / (y3 - y1); // Progress on long line
        int xa = static_cast<int>(x1 + ((x3 - x1) * a)); // x of long line

        float b = static_cast<float>((y - y1)) / (y2 - y1); // Progress on short line
        int xb = static_cast<int>(x1 + ((x2 - x1) * b)); // x of short line

        draw_line(framebuffer, xa, y, xb, y, color);
    }

    for (int y = y2; y <= y3; y++) {
        float a = static_cast<float>((y - y1)) / (y3 - y1); // Progress on long line
        int xa = static_cast<int>(x1 + ((x3 - x1) * a)); // x of long line

        float b = static_cast<float>((y - y2)) / (y3 - y2);
        int xb = static_cast<int>(x2 + ((x3 - x2) * b));

        draw_line(framebuffer, xa, y, xb, y, color);
    }
}

void draw_bounding_box(TGAImage& framebuffer, int x1, int y1, int x2, int y2, int x3, int y3, TGAColor color ) {
    int min_x = std::min({x1, x2, x3});
    int min_y = std::min({y1, y2, y3});

    int max_x = std::max({x1, x2, x3});
    int max_y = std::max({y1, y2, y3});

    for (int y = min_y; y <= max_y; y++) {
        draw_line(framebuffer, min_x, y, max_x, y, color);
    }
}

void draw_triangle(TGAImage& framebuffer, int x1, int y1, int x2, int y2, int x3, int y3, TGAColor color) {
    float det = static_cast<float>((x2 - x1) * (y3 - y1) - (y2 - y1) * (x3 - x1));

    if (std::abs(det) < 1e-3f) return;
    float inv_det = 1.0f / det;

    float min_x = std::max(0, std::min({x1, x2, x3}));
    float max_x = std::min(framebuffer.width() - 1, std::max({x1, x2, x3}));
    
    float min_y = std::max(0, std::min({y1, y2, y3}));
    float max_y = std::min(framebuffer.width() - 1, std::max({y1, y2, y3}));

    for (int y = min_y; y <= max_y; y++) {
        for (int x = min_x; x <= max_x; x++) {
            float uAC = ((x2 - x1) * (y - y1) - (y2 - y1) * (x - x1)) * inv_det;

            float uAB = ((x - x1) * (y3 - y1) - (y - y1) * (x3 - x1)) * inv_det;

            float u = 1 - (uAB + uAC);
            
            if (uAC >= 0.0f && uAB >= 0.0f && u >= 0.0f) {
                framebuffer.set(x, y, color);
            }
        }
    }
}

void draw_object(TGAImage& framebuffer, Vec3v v1, Vec3v v2, Vec3v v3, int width, int height) {
    Vec3v v[3] = {v1, v2, v3};
    int vx[3], vy[3];
    for (int i = 0; i < 3; i ++) {
        vx[i] = static_cast<int>(((v[i].x + 1 )* width) / 2);
        vy[i] = static_cast<int>(((v[i].y + 1) * width) / 2);
    }

    draw_triangle(framebuffer, vx[0], vy[0], vx[1], vy[1], vx[2], vy[2], random_color());
}

void outline_triangle(TGAImage& framebuffer, Vec3v v1, Vec3v v2, Vec3v v3, int width, int height) {
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

void draw_raw_rectangle(TGAImage& framebuffer, int x1, int y1, int x2, int y2, int x3, int y3, TGAColor color) {
    draw_line(framebuffer, x1, y1, x2, y2, color);
    draw_line(framebuffer, x1, y1, x3, y3, color);
    draw_line(framebuffer, x3, y3, x2, y2, color);
}

int main(int argc, char** argv) {
    constexpr int width  = 1024;
    constexpr int height = 1024;
    TGAImage framebuffer(width, height, TGAImage::RGB);
    Parser object_parser = Parser();

    int ax =  7, ay =  3;
    int bx = 12, by = 37;
    int cx = 62, cy = 53;

    // draw_triangle(framebuffer,  7, 45, 35, 100, 45,  60, red);
    // draw_triangle(framebuffer, 120, 35, 90,   5, 45, 110, white);
    // draw_triangle(framebuffer, 115, 83, 80,  90, 85, 120, green);

    // framebuffer.set(ax, ay, white);
    // framebuffer.set(bx, by, white);
    // framebuffer.set(cx, cy, white);

    // object_parser.load("./diablo3_pose.obj");
    object_parser.load("./african_head.obj");

    for (Vec3f triangle : object_parser.faces) {
        draw_object(framebuffer, object_parser.vertices[triangle.v0],
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
