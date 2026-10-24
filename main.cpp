#include <cmath>
#include <cstdlib>
#include <ctime>
#include "tgaimage.h"
#include "objParser.cpp"
#include <random>
#include "vec3.cpp"
#include "matrix.cpp"

constexpr int width = 800;
constexpr int height = 800;
std::vector<double> zbuffer(width * height, -std::numeric_limits<double>::max());

mat<4,4> ModelView, Viewport, Perspective;

void set_zbuffer(int x, int y, double val) {
    int index = y * width + x;
    zbuffer[index] = val;
}

double get_zbuffer(int x, int y) {
    int index = y * width + x;
    return zbuffer[index];
}

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

vec3 rot(vec3 vector) {
    const double a = M_PI / 6.0;
    const double c = std::cos(a);
    const double s = std::sin(a);

    mat3 Ry;
    Ry[0] = vec3{ c, 0.0, s };
    Ry[1] = vec3{ 0.0, 1.0, 0.0 };
    Ry[2] = vec3{ -s, 0.0, s };

    return Ry * vector;
}

vec3 persp(vec3 v) {
    const double c = 3;
    return v / (1 - v.z / c);
}

void perspective(const double f) {
    Perspective = {{{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, -1/f, 1}}};
}

vec3 project(vec3 v) {
    return { (v.x + 1.) * width / 2,
             (v.y + 1.) * height / 2,
             (v.z + 1.) * 255./2 };
}

void viewport(const int x, const int y, const int w, const int h) {
    Viewport = {{{w/2., 0, 0, x+w/2.}, {0, h/2., 0, y+h/2.}, {0,0,1,0}, {0,0,0,1}}};
}

void lookat(const vec3 eye, const vec3 center, const vec3 up) {
    vec3 n = normalised(eye - center);
    vec3 l = normalised(cross(up, n));
    vec3 m = normalised(cross(n, l));
    ModelView = mat<4,4>{{{l.x, l.y, l.z, 0}, {m.x, m.y, m.z, 0}, {n.x, n.y, n.z, 0}, {0, 0, 0, 1}}} *
                mat<4, 4>{{{1, 0, 0, -center.x}, {0, 1, 0, -center.y}, {0, 0, 1, -center.z}, {0, 0, 0, 1}}};
}

void draw_triangle(TGAImage& framebuffer, int x1, int y1, int z1, int x2, int y2, int z2, int x3, int y3, int z3, TGAColor color) {
    float det = static_cast<float>((x2 - x1) * (y3 - y1) - (y2 - y1) * (x3 - x1));

    if (std::abs(det) < 1e-3f) return; // backface culling + discarding triangle that cover less than 1 pixel
    float inv_det = 1.0f / det;

    float min_x = std::max(0, std::min({x1, x2, x3})); // bounding box for triangle
    float max_x = std::min(framebuffer.width() - 1, std::max({x1, x2, x3})); // top left and bottom right
    
    float min_y = std::max(0, std::min({y1, y2, y3}));
    float max_y = std::min(framebuffer.width() - 1, std::max({y1, y2, y3}));

    for (int y = min_y; y <= max_y; y++) {
        for (int x = min_x; x <= max_x; x++) {
            float uAC = ((x2 - x1) * (y - y1) - (y2 - y1) * (x - x1)) * inv_det;

            float uAB = ((x - x1) * (y3 - y1) - (y - y1) * (x3 - x1)) * inv_det;

            float u = 1 - (uAB + uAC);
            
            if (uAC >= 0.0f && uAB >= 0.0f && u >= 0.0f) {
                unsigned char z = static_cast<unsigned char>(u * z1 + uAB * z2 + uAC * z3);
                if (z > get_zbuffer(x, y)) {
                    set_zbuffer(x, y, z);
                    framebuffer.set(x, y, color);
                }
            }
        }
    }
}

void draw_object(TGAImage& framebuffer, vec3 v1, vec3 v2, vec3 v3) {
    vec3 v[3] = {v1, v2, v3};
    int vx[3], vy[3], vz[3];
    for (int i = 0; i < 3; i ++) {
        v[i] = project(persp(rot(v[i])));
        vx[i] = static_cast<int>(v[i].x);
        vy[i] = static_cast<int>(v[i].y);
        vz[i] = static_cast<int>(v[i].z);
    }

    draw_triangle(framebuffer, vx[0], vy[0],vz[0], vx[1], vy[1], vz[1], vx[2], vy[2], vz[2], random_color());
}

int main(int argc, char** argv) {
    TGAImage framebuffer(width, height, TGAImage::RGB);

    Parser object_parser = Parser();

    constexpr vec3 eye{-1, 0, 2};
    constexpr vec3 center{0,0,0};
    constexpr vec3 up{0, 1, 0};

    lookat(eye, center, up);
    perspective(norm(eye - center));
    viewport(width / 16, height / 16, width * 7 / 8, height * 7 / 8);

    // object_parser.load("./diablo3_pose.obj");
    object_parser.load("./african_head.obj");

    for (Vec3f triangle : object_parser.faces) {
        draw_object(framebuffer, object_parser.vertices[triangle.v0],
             object_parser.vertices[triangle.v1], object_parser.vertices[triangle.v2]);
    }
    
    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}