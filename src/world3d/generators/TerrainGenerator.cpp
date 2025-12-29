#include "world3d/generators/TerrainGenerator.hpp"
#include <fstream>
#include <vector>
#include <cmath>
#include <iostream>
#include <filesystem>
#include <iomanip>

// GLM for vector math
#include <glm/glm.hpp>

namespace World3D::Generators {

// Define M_PI or use std::numbers in C++20 if available, but math.h works
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// === Noise Functions ===

// Simple hash function for pseudo-random values
static inline int hash(int x, int y, int seed) {
    int n = x + y * 57 + seed * 131;
    n = (n << 13) ^ n;
    return (n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff;
}

// Generate random float [0, 1) from hash
static inline float hashToFloat(int x, int y, int seed) {
    return hash(x, y, seed) / 2147483648.0f;
}

// Smooth interpolation (smoothstep)
static inline float smoothstep(float t) {
    return t * t * (3.0f - 2.0f * t);
}

// 2D Noise function (value noise)
static float noise2D(float x, float y, int seed) {
    int xi = static_cast<int>(std::floor(x));
    int yi = static_cast<int>(std::floor(y));
    float xf = x - xi;
    float yf = y - yi;

    // Get corner values
    float v00 = hashToFloat(xi, yi, seed);
    float v10 = hashToFloat(xi + 1, yi, seed);
    float v01 = hashToFloat(xi, yi + 1, seed);
    float v11 = hashToFloat(xi + 1, yi + 1, seed);

    // Smooth interpolation
    float u = smoothstep(xf);
    float v = smoothstep(yf);

    // Bilinear interpolation
    float v0 = v00 * (1.0f - u) + v10 * u;
    float v1 = v01 * (1.0f - u) + v11 * u;
    return v0 * (1.0f - v) + v1 * v;
}

// Fractional Brownian Motion (multi-octave noise)
static float fbm(float x, float y, int octaves, float persistence, int seed) {
    float total = 0.0f;
    float frequency = 1.0f;
    float amplitude = 1.0f;
    float maxValue = 0.0f;

    for (int i = 0; i < octaves; i++) {
        total += noise2D(x * frequency, y * frequency, seed + i) * amplitude;
        maxValue += amplitude;
        amplitude *= persistence;
        frequency *= 2.0f;
    }

    return total / maxValue; // Normalize to [0, 1]
}

// === Terrain Generation ===

float TerrainGenerator::getHeight(float x, float y, float cx, float cy, Type type) {
    if (type == Type::Flat) return 0.0f;

    float val = 0.0f;
    const int seed = 42; // Fixed seed for reproducibility
    
    if (type == Type::Hills) {
        // Gentle rolling hills with 4 octaves
        float baseNoise = fbm(x * 0.005f, y * 0.005f, 5, 0.5f, seed); // Lower freq, more octaves
        val = (baseNoise - 0.5f) * 60.0f; // Range: -30 to +30 (Steepers)
        
        // Add a subtle central peak
        float d2 = std::pow(x, 2) + std::pow(y, 2);
        val += std::exp(-d2 / 15000.0f) * 30.0f;
    } 
    else if (type == Type::Mountains) {
        // Sharp mountainous terrain with 6 octaves
        float baseNoise = fbm(x * 0.006f, y * 0.006f, 7, 0.55f, seed); // More rugged
        val = (baseNoise - 0.5f) * 300.0f; // Much higher amplitude for >35deg slopes
        
        // Add dramatic central peak
        float d2 = std::pow(x, 2) + std::pow(y, 2);
        val += std::exp(-d2 / 20000.0f) * 150.0f;
        
        // Add sharp ridges using noise derivative
        float ridgeNoise = fbm(x * 0.015f, y * 0.015f, 4, 0.5f, seed + 100);
        val += (1.0f - std::abs(ridgeNoise - 0.5f) * 2.0f) * 20.0f;
    }
    else if (type == Type::Canyon) {
        // Canyon/valley with erosion-like features
        float baseNoise = fbm(x * 0.01f, y * 0.01f, 6, 0.5f, seed);
        
        // Create central valley (inverted)
        float d2 = std::pow(x, 2) + std::pow(y / 3.0f, 2); // Elongated
        val = -std::exp(-d2 / 25000.0f) * 150.0f;
        
        // Add erosion detail
        val += (baseNoise - 0.5f) * 25.0f;
    }
    else if (type == Type::Showcase) {
        // "Showcase" / Complete Demo
        // Designed to capture ALL soil types:
        // 1. High Flat Plateau -> Latossolos
        // 2. Steep Slopes -> Neossolos
        // 3. Moderate Slopes -> Argissolos/Cambissolos
        // 4. Low Valleys -> Gleissolos (if wet)
        
        float baseNoise = fbm(x * 0.004f, y * 0.004f, 6, 0.5f, seed);
        
        // Define Plateau Shape (Central high area)
        // Use a smooth step or sigmoid to create a flat top
        float shape = baseNoise; 
        
        // Quantize/Flatten top to create plateau
        // Map 0..1 noise to -100..+200
        // But force ranges > 0.7 to be flat
        
        if (shape > 0.65f) {
            // Plateau (Top) - Very subtle noise to ensure slope < 8 deg for Latossolos
            // Flatten significantly
            val = 150.0f + (shape - 0.65f) * 2.0f; 
        } else if (shape < 0.3f) {
            // Valley (Bottom)
            val = -50.0f + (shape - 0.3f) * 20.0f; // Low, gentle
        } else {
            // Slopes (Steep transition)
            // Remap 0.3..0.65 to -50..150
            float t = (shape - 0.3f) / (0.65f - 0.3f); // 0..1
            // Use smoothstep for organic transition
            t = t * t * (3.0f - 2.0f * t);
            val = -50.0f + t * 200.0f;
            
            // Add ruggedness to slopes specifically (Neossolos)
            float rug = fbm(x * 0.02f, y * 0.02f, 3, 0.5f, seed+99);
            val += (rug - 0.5f) * 30.0f; 
        }
    }

    return val;
}

bool TerrainGenerator::generate(const std::string& filename, int width, int height, float spacing, Type type, std::function<void(float, const std::string&)> onProgress) {
    // Ensure directory exists
    std::filesystem::path path(filename);
    if (path.has_parent_path()) {
        std::filesystem::create_directories(path.parent_path());
    }

    if (onProgress) onProgress(0.0f, "Initializing...");
    std::cout << "[Generator] Generating terrain " << width << "x" << height << " to " << filename << "..." << std::endl;

    std::vector<glm::vec3> vertices;
    // Reserve memory to avoid reallocations
    vertices.reserve(width * height);
    
    std::vector<glm::vec3> normals;
    normals.reserve(width * height);
    
    std::vector<glm::ivec3> faces;
    faces.reserve((width - 1) * (height - 1) * 2);

    float cx = (width * spacing) / 2.0f;
    float cy = (height * spacing) / 2.0f;

    // 1. Generate Vertices and Normals
    if (onProgress) onProgress(0.1f, "Generating Vertices...");
    for (int i = 0; i < width; i++) {
        // Report progress periodically (every 1% or so)
        if (onProgress && i % (width / 100 + 1) == 0) {
            float pct = 0.1f + (0.4f * ((float)i / width)); // 10% to 50%
            onProgress(pct, "Generating Vertices...");
        }

        for (int j = 0; j < height; j++) {
            float x = (i * spacing) - cx;
            float y = (j * spacing) - cy;
            float z = getHeight(x, y, 0.0f, 0.0f, type);

            vertices.push_back(glm::vec3(x, y, z));

            // Analytic Normal Calculation
            float eps = 0.1f;
            float hl = getHeight(x - eps, y, 0.0f, 0.0f, type);
            float hr = getHeight(x + eps, y, 0.0f, 0.0f, type);
            float hd = getHeight(x, y - eps, 0.0f, 0.0f, type);
            float hu = getHeight(x, y + eps, 0.0f, 0.0f, type);

            // -dz/dx, -dz/dy, 1 (scaled by 2*eps)
            float nx = -2.0f * eps * (hr - hl);
            float ny = -2.0f * eps * (hu - hd);
            float nz = 4.0f * eps * eps;

            normals.push_back(glm::normalize(glm::vec3(nx, ny, nz)));
        }
    }

    // 2. Generate Faces
    if (onProgress) onProgress(0.5f, "Generating Topology...");
    for (int i = 0; i < width - 1; i++) {
        if (onProgress && i % (width / 100 + 1) == 0) {
            float pct = 0.5f + (0.2f * ((float)i / (width - 1))); // 50% to 70%
            onProgress(pct, "Generating Topology...");
        }

        for (int j = 0; j < height - 1; j++) {
            // 1-based indices
            int base = i * height + j + 1;
            
            // Grid layout: inner loop is height (j)
            // v(i, j)   = base
            // v(i, j+1) = base + 1
            // v(i+1, j) = base + height
            // v(i+1, j+1)= base + height + 1

            int v1 = base;
            int v2 = base + 1;
            int v3 = base + height;
            int v4 = base + height + 1;

            // Face 1: v1-v2-v3
            faces.push_back(glm::ivec3(v1, v2, v3));
            // Face 2: v2-v4-v3
            faces.push_back(glm::ivec3(v2, v4, v3));
        }
    }

    // 3. Write OBJ
    if (onProgress) onProgress(0.7f, "Writing to disk...");
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "[Generator] Failed to open file: " << filename << std::endl;
        return false;
    }

    file << "# Procedural Terrain generated by SisterPEC Engine\n";
    file << "o Terrain\n";
    file << std::fixed << std::setprecision(4);

    for (const auto& v : vertices) {
        file << "v " << v.x << " " << v.y << " " << v.z << "\n";
    }

    for (const auto& n : normals) {
        file << "vn " << n.x << " " << n.y << " " << n.z << "\n";
    }

    for (const auto& f : faces) {
        // format f v//vn v//vn v//vn
        // Assuming vertex index == normal index
        file << "f " << f.x << "//" << f.x << " " 
                     << f.y << "//" << f.y << " " 
                     << f.z << "//" << f.z << "\n";
    }

    file.close();
    std::cout << "[Generator] Complete. " << vertices.size() << " vertices, " << faces.size() << " faces." << std::endl;
    return true;
}

} // namespace World3D::Generators
