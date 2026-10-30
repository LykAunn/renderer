#include <cmath>
#include <cstdlib>
#include <ctime>
#include "tgaimage.h"
#include "objParser.cpp"
#include <random>
#include "vec3.cpp"
#include "matrix.cpp"

constexpr int width = 1200;
constexpr int height = 1200;
std::vector<double> zbuffer(width * height, -std::numeric_limits<double>::max());

mat<4,4> ModelView, Viewport, Perspective;

template <int n>
vec<n> operator*(const vec<n>& vector, const mat<n, n>& matrix) {
    vec<n> res;
    for (int i = 0; i < n; i++) {
        double sum = 0.0;
        for (int j = 0; j < n; j++) {
            sum += matrix[i][j] * vector[i];            
        }
        res[i] = sum;
    }
    return res;
}

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

void perspective(const double f) {
    Perspective = {{{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, -1/f, 1}}};
}

void viewport(const double x, const double y, const double w, const double h) {
    Viewport = {{{w/2., 0, 0, x+w/2.}, {0, h/2., 0, y+h/2.}, {0,0,1,0}, {0,0,0,1}}};
}

void lookat(const vec3 eye, const vec3 center, const vec3 up) {
    vec3 n = normalised(eye - center);
    vec3 l = normalised(cross(up, n));
    vec3 m = normalised(cross(n, l));
    ModelView = mat<4,4>{{{l.x, l.y, l.z, 0}, {m.x, m.y, m.z, 0}, {n.x, n.y, n.z, 0}, {0, 0, 0, 1}}} *
                mat<4, 4>{{{1, 0, 0, -center.x}, {0, 1, 0, -center.y}, {0, 0, 1, -center.z}, {0, 0, 0, 1}}};
}

void draw_triangle(TGAImage& framebuffer, float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3, TGAColor color) {
    float det = static_cast<float>((x2 - x1) * (y3 - y1) - (y2 - y1) * (x3 - x1));

    if (det <= 0.0f) return; // backface culling + discarding triangle that cover less than 1 pixel
    float inv_det = 1.0f / det;

    float min_x = std::max(0, static_cast<int>(std::min({x1, x2, x3}))); // bounding box for triangle
    float max_x = std::min(framebuffer.width() - 1, static_cast<int>(std::max({x1, x2, x3}))); // top left and bottom right
    
    float min_y = std::max(0, static_cast<int>(std::min({y1, y2, y3})));
    float max_y = std::min(framebuffer.width() - 1, static_cast<int>(std::max({y1, y2, y3})));

    for (int y = min_y; y <= max_y; y++) {
        for (int x = min_x; x <= max_x; x++) {

            double px = x + 0.5;
            double py = y + 0.5;

            float uAC = ((x2 - x1) * (py - y1) - (y2 - y1) * (px - x1)) * inv_det;
            float uAB = ((px - x1) * (y3 - y1) - (y - y1) * (x3 - x1)) * inv_det;

            float u = 1 - (uAB + uAC);
            
            if (uAC >= -1e-5 && uAB >= -1e-5 && u >= -1e-5) {
                double z = u * z1 + uAB * z2 + uAC * z3;
                if (z > get_zbuffer(x, y)) {
                    set_zbuffer(x, y, z);
                    framebuffer.set(x, y, color);
                }
            }
        }
    }
}

void draw_object(TGAImage& framebuffer, vec3 v1, vec3 v2, vec3 v3, const mat4& MVP) {
    // Promote to 4D
    vec4 v[3] = {
        {v1.x, v1.y, v1.z, 1.0}, 
        {v2.x, v2.y, v2.z, 1.0}, 
        {v3.x, v3.y, v3.z, 1.0}
    };
    float vx[3], vy[3], vz[3];

    // Perform transformations
    for (int i = 0; i < 3; i ++) {
        v[i] = MVP * v[i];
        
        // Perspective divide
        for (int j = 0; j < 3; j ++) {
            v[i][j] = v[i][j] / v[i][3]; // Divide own component with W component
        }
        v[i][3] = 1.0;
        
        // Map to screen pixels
        v[i] = v[i] * Viewport;
        
        vx[i] = v[i].x;
        vy[i] = v[i].y;
        vz[i] = v[i].z;
    }
    
    draw_triangle(framebuffer, vx[0], vy[0],vz[0], vx[1], vy[1], vz[1], vx[2], vy[2], vz[2], random_color());
}

int main(int argc, char** argv) {
    TGAImage framebuffer(width, height, TGAImage::RGB);
    
    Parser object_parser = Parser();
    
    constexpr vec3 eye{0, 0, 2};
    constexpr vec3 center{0, 0, 0};
    constexpr vec3 up{0, 1, 0};
    
    lookat(eye, center, up);
    perspective(norm(eye - center));
    viewport(width / 16, height / 16, width * 7 / 8, height * 7 / 8);

    // Shrink the model to 50% size
    mat<4,4> Scale = {{{0.5, 0, 0, 0}, 
                    {0, 0.5, 0, 0}, 
                    {0, 0, 0.5, 0}, 
                    {0, 0, 0, 1}}};

    // Positive X moves right, Positive Y moves up
    double dx = 0.5; // Shift right by 0.5 units
    double dy = 0.5; // Shift up by 0.5 units
    double dz = 0.0;

    mat<4,4> Translation = {{{1, 0, 0, dx}, 
                            {0, 1, 0, dy}, 
                            {0, 0, 1, dz}, 
                            {0, 0, 0, 1}}};
                   
    mat4 MVP = Perspective * ModelView * Translation * Scale ;

    object_parser.load("./diablo3_pose.obj");
    // object_parser.load("./african_head.obj");

    for (Vec3f triangle : object_parser.faces) {
        draw_object(framebuffer, object_parser.vertices[triangle.v0],
             object_parser.vertices[triangle.v1], object_parser.vertices[triangle.v2], MVP);
    }
    
    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}