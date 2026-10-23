#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#include "vec3.cpp"

struct Vec3f {
    int v0, v1, v2;
};

class Parser {
public:
    std::vector<vec3> vertices;
    std::vector<Vec3f> faces;

    Parser() = default;

    bool load(const std::string& filename) {
        std::ifstream file(filename);

        // Check if file is opened
        if (!file.is_open()) {
            std::cerr << "Failed to open: " << filename << "\n";
            return false;
        }

        std::string line;
        // Read line by line
        while (getline(file, line)) {
            std::string token = "";
            std::istringstream iss(line);
            iss >> token;
            
            if (token == "v") {
                float x,y,z;
                if (iss >> x >> y >> z) {
                    vertices.push_back(vec3{x, y, z});
                }

            } else if (token == "f") {
                int v_indices[3];
                int vt, vn;
                char slash;

                for (int i = 0; i < 3; i ++) {
                    if (iss >> v_indices[i] >> slash >> vt >> slash >> vn) {
                        v_indices[i] -= 1;
                    }
                }

                faces.push_back({v_indices[0], v_indices[1], v_indices[2]});
            }
        }
        file.close();
        return true;
    }
};