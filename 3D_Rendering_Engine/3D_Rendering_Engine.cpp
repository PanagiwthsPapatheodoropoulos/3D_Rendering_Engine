#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#define SDL_MAIN_HANDLED
#include <SDL.h>

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;


struct Vec3 {
    float x, y, z;
    Vec3(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {}
    Vec3 operator+(const Vec3& v) const { return Vec3(x + v.x, y + v.y, z + v.z); }
    Vec3 operator-(const Vec3& v) const { return Vec3(x - v.x, y - v.y, z - v.z); }
    Vec3 operator*(float f) const { return Vec3(x * f, y * f, z * f); }
    float dot(const Vec3& v) const { return x * v.x + y * v.y + z * v.z; }
    Vec3 cross(const Vec3& v) const { return Vec3(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x); }
    Vec3 normalize() const { float mag = sqrt(x * x + y * y + z * z); return Vec3(x / mag, y / mag, z / mag); }
};

struct Triangle {
    Vec3 v[3];
    SDL_Color color;
};

struct Mesh {
    std::vector<Triangle> tris;
};

struct Mat4x4 {
    float m[4][4] = { 0 };
};

class Renderer {
public:
    Renderer() {
        SDL_Init(SDL_INIT_VIDEO);
        window = SDL_CreateWindow("3D Renderer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    }

    ~Renderer() {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    void clear() {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
    }

    void drawTriangle(const Triangle& tri) {
        SDL_SetRenderDrawColor(renderer, tri.color.r, tri.color.g, tri.color.b, tri.color.a);
        SDL_RenderDrawLine(renderer, tri.v[0].x, tri.v[0].y, tri.v[1].x, tri.v[1].y);
        SDL_RenderDrawLine(renderer, tri.v[1].x, tri.v[1].y, tri.v[2].x, tri.v[2].y);
        SDL_RenderDrawLine(renderer, tri.v[2].x, tri.v[2].y, tri.v[0].x, tri.v[0].y);
    }

    void present() {
        SDL_RenderPresent(renderer);
    }

private:
    SDL_Window* window;
    SDL_Renderer* renderer;
};

Mat4x4 matrixMultiply(const Mat4x4& m1, const Mat4x4& m2) {
    Mat4x4 result;
    for (int c = 0; c < 4; c++)
        for (int r = 0; r < 4; r++)
            result.m[r][c] = m1.m[r][0] * m2.m[0][c] + m1.m[r][1] * m2.m[1][c] + m1.m[r][2] * m2.m[2][c] + m1.m[r][3] * m2.m[3][c];
    return result;
}

Vec3 matrixVectorMultiply(const Mat4x4& m, const Vec3& i) {
    Vec3 v;
    v.x = i.x * m.m[0][0] + i.y * m.m[1][0] + i.z * m.m[2][0] + m.m[3][0];
    v.y = i.x * m.m[0][1] + i.y * m.m[1][1] + i.z * m.m[2][1] + m.m[3][1];
    v.z = i.x * m.m[0][2] + i.y * m.m[1][2] + i.z * m.m[2][2] + m.m[3][2];
    float w = i.x * m.m[0][3] + i.y * m.m[1][3] + i.z * m.m[2][3] + m.m[3][3];

    if (w != 0.0f) {
        v.x /= w; v.y /= w; v.z /= w;
    }
    return v;
}

Mat4x4 makeProjectionMatrix(float fovDegrees, float aspectRatio, float near, float far) {
    float fovRad = 1.0f / tanf(fovDegrees * 0.5f / 180.0f * 3.14159f);
    Mat4x4 matrix;
    matrix.m[0][0] = aspectRatio * fovRad;
    matrix.m[1][1] = fovRad;
    matrix.m[2][2] = far / (far - near);
    matrix.m[3][2] = (-far * near) / (far - near);
    matrix.m[2][3] = 1.0f;
    matrix.m[3][3] = 0.0f;
    return matrix;
}

int main() {
    Renderer renderer;
    Mesh cubeMesh;

    // Define cube
    cubeMesh.tris = {
        // SOUTH
        { Vec3(0.0f, 0.0f, 0.0f),    Vec3(0.0f, 1.0f, 0.0f),    Vec3(1.0f, 1.0f, 0.0f) },
        { Vec3(0.0f, 0.0f, 0.0f),    Vec3(1.0f, 1.0f, 0.0f),    Vec3(1.0f, 0.0f, 0.0f) },
        // EAST
        { Vec3(1.0f, 0.0f, 0.0f),    Vec3(1.0f, 1.0f, 0.0f),    Vec3(1.0f, 1.0f, 1.0f) },
        { Vec3(1.0f, 0.0f, 0.0f),    Vec3(1.0f, 1.0f, 1.0f),    Vec3(1.0f, 0.0f, 1.0f) },
        // NORTH
        { Vec3(1.0f, 0.0f, 1.0f),    Vec3(1.0f, 1.0f, 1.0f),    Vec3(0.0f, 1.0f, 1.0f) },
        { Vec3(1.0f, 0.0f, 1.0f),    Vec3(0.0f, 1.0f, 1.0f),    Vec3(0.0f, 0.0f, 1.0f) },
        // WEST
        { Vec3(0.0f, 0.0f, 1.0f),    Vec3(0.0f, 1.0f, 1.0f),    Vec3(0.0f, 1.0f, 0.0f) },
        { Vec3(0.0f, 0.0f, 1.0f),    Vec3(0.0f, 1.0f, 0.0f),    Vec3(0.0f, 0.0f, 0.0f) },
        // TOP
        { Vec3(0.0f, 1.0f, 0.0f),    Vec3(0.0f, 1.0f, 1.0f),    Vec3(1.0f, 1.0f, 1.0f) },
        { Vec3(0.0f, 1.0f, 0.0f),    Vec3(1.0f, 1.0f, 1.0f),    Vec3(1.0f, 1.0f, 0.0f) },
        // BOTTOM
        { Vec3(1.0f, 0.0f, 1.0f),    Vec3(0.0f, 0.0f, 1.0f),    Vec3(0.0f, 0.0f, 0.0f) },
        { Vec3(1.0f, 0.0f, 1.0f),    Vec3(0.0f, 0.0f, 0.0f),    Vec3(1.0f, 0.0f, 0.0f) },
    };

    // Projection Matrix
    float fNear = 0.1f;
    float fFar = 1000.0f;
    float fFov = 90.0f;
    float fAspectRatio = (float)SCREEN_HEIGHT / (float)SCREEN_WIDTH;
    Mat4x4 matProj = makeProjectionMatrix(fFov, fAspectRatio, fNear, fFar);

    float fTheta = 0.0f;
    bool quit = false;
    SDL_Event e;

    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
        }

        renderer.clear();

        // Rotation matrices
        Mat4x4 matRotZ, matRotX;
        fTheta += 0.01f;

        matRotZ.m[0][0] = cosf(fTheta);
        matRotZ.m[0][1] = sinf(fTheta);
        matRotZ.m[1][0] = -sinf(fTheta);
        matRotZ.m[1][1] = cosf(fTheta);
        matRotZ.m[2][2] = 1;
        matRotZ.m[3][3] = 1;

        matRotX.m[0][0] = 1;
        matRotX.m[1][1] = cosf(fTheta * 0.5f);
        matRotX.m[1][2] = sinf(fTheta * 0.5f);
        matRotX.m[2][1] = -sinf(fTheta * 0.5f);
        matRotX.m[2][2] = cosf(fTheta * 0.5f);
        matRotX.m[3][3] = 1;

        for (auto tri : cubeMesh.tris) {
            Triangle triProjected, triTransformed;

            for (int i = 0; i < 3; i++) {
                Vec3 v = tri.v[i];
                v = matrixVectorMultiply(matRotZ, v);
                v = matrixVectorMultiply(matRotX, v);
                v.z += 3.0f;
                triTransformed.v[i] = v;
            }

            // Calculate triangle normal
            Vec3 normal, line1, line2;
            line1 = triTransformed.v[1] - triTransformed.v[0];
            line2 = triTransformed.v[2] - triTransformed.v[0];
            normal = line1.cross(line2).normalize();

            // Backface culling
            if (normal.z < 0) {
                for (int i = 0; i < 3; i++) {
                    triProjected.v[i] = matrixVectorMultiply(matProj, triTransformed.v[i]);
                    triProjected.v[i].x += 1.0f; triProjected.v[i].y += 1.0f;
                    triProjected.v[i].x *= 0.5f * SCREEN_WIDTH;
                    triProjected.v[i].y *= 0.5f * SCREEN_HEIGHT;
                }

                // Calculate lighting
                Vec3 light_direction = Vec3(0.0f, 0.0f, -1.0f);
                float dp = normal.dot(light_direction);
                triProjected.color = { static_cast<Uint8>(dp * 255.0f), static_cast<Uint8>(dp * 255.0f), static_cast<Uint8>(dp * 255.0f), 255 };

                renderer.drawTriangle(triProjected);
            }
        }

        renderer.present();
        SDL_Delay(10);
    }

    return 0;
}
