#include "VertoEngine.h"
#include <vector>
#include <cmath>
using namespace std;

struct vec3d { 
    float x, y, z; 
};


struct triangle 
{
    vec3d p[3];
    triangle() {}
    triangle(vec3d v1, vec3d v2, vec3d v3) {
        p[0] = v1; p[1] = v2; p[2] = v3;
    }
};

struct mesh 
{ 
    vector<triangle> tris; 
};

struct mat4x4
{
    float m[4][4] = { 0 }; 
};

class VertoEngine3D : public VertoEngine
{
public:
    VertoEngine3D() { m_sAppName = L"VertoEngine 3D Cube"; }

private:
    mesh meshCube;
    mat4x4 matProj;
    float fTheta = 0.0f;

    void MultiplyMatrixVector(vec3d& i, vec3d& o, mat4x4& m)
    {
        o.x = i.x * m.m[0][0] + i.y * m.m[1][0] + i.z * m.m[2][0] + m.m[3][0];
        o.y = i.x * m.m[0][1] + i.y * m.m[1][1] + i.z * m.m[2][1] + m.m[3][1];
        o.z = i.x * m.m[0][2] + i.y * m.m[1][2] + i.z * m.m[2][2] + m.m[3][2];
        float w = i.x * m.m[0][3] + i.y * m.m[1][3] + i.z * m.m[2][3] + m.m[3][3];

        if (w != 0.0f) { o.x /= w; o.y /= w; o.z /= w; }
    }

public:
    bool OnUserCreate() override
    {
        // Define cube
        meshCube.tris = {
            // SOUTH
            { {0,0,0}, {0,1,0}, {1,1,0} }, { {0,0,0}, {1,1,0}, {1,0,0} },
            // EAST
            { {1,0,0}, {1,1,0}, {1,1,1} }, { {1,0,0}, {1,1,1}, {1,0,1} },
            // NORTH
            { {1,0,1}, {1,1,1}, {0,1,1} }, { {1,0,1}, {0,1,1}, {0,0,1} },
            // WEST
            { {0,0,1}, {0,1,1}, {0,1,0} }, { {0,0,1}, {0,1,0}, {0,0,0} },
            // TOP
            { {0,1,0}, {0,1,1}, {1,1,1} }, { {0,1,0}, {1,1,1}, {1,1,0} },
            // BOTTOM
            { {1,0,1}, {0,0,1}, {0,0,0} }, { {1,0,1}, {0,0,0}, {1,0,0} }
        };

        // Projection
        float fNear = 0.1f, fFar = 1000.0f, fFov = 90.0f;
        float fAspectRatio = (float)ScreenHeight() / (float)ScreenWidth();
        float fFovRad = 1.0f / tanf(fFov * 0.5f / 180.0f * 3.14159f);

        matProj.m[0][0] = fAspectRatio * fFovRad;
        matProj.m[1][1] = fFovRad;
        matProj.m[2][2] = fFar / (fFar - fNear);
        matProj.m[3][2] = (-fFar * fNear) / (fFar - fNear);
        matProj.m[2][3] = 1.0f;
        matProj.m[3][3] = 0.0f;

        return true;
    }

    bool OnUserUpdate(float fElapsedTime) override
    {
        Fill(0, 0, ScreenWidth(), ScreenHeight(), PIXEL_SOLID, FG_BLACK);

        fTheta += 1.0f * fElapsedTime;

        mat4x4 matRotZ, matRotX;

        // Rotation Z
        matRotZ.m[0][0] = cosf(fTheta); matRotZ.m[0][1] = sinf(fTheta);
        matRotZ.m[1][0] = -sinf(fTheta); matRotZ.m[1][1] = cosf(fTheta);
        matRotZ.m[2][2] = 1; matRotZ.m[3][3] = 1;

        // Rotation X
        matRotX.m[0][0] = 1;
        matRotX.m[1][1] = cosf(fTheta * 0.5f); matRotX.m[1][2] = sinf(fTheta * 0.5f);
        matRotX.m[2][1] = -sinf(fTheta * 0.5f); matRotX.m[2][2] = cosf(fTheta * 0.5f);
        matRotX.m[3][3] = 1;

        for (auto tri : meshCube.tris)
        {
            triangle triProjected, triTranslated, triRotZ, triRotZX;

            MultiplyMatrixVector(tri.p[0], triRotZ.p[0], matRotZ);
            MultiplyMatrixVector(tri.p[1], triRotZ.p[1], matRotZ);
            MultiplyMatrixVector(tri.p[2], triRotZ.p[2], matRotZ);

            MultiplyMatrixVector(triRotZ.p[0], triRotZX.p[0], matRotX);
            MultiplyMatrixVector(triRotZ.p[1], triRotZX.p[1], matRotX);
            MultiplyMatrixVector(triRotZ.p[2], triRotZX.p[2], matRotX);

            triTranslated = triRotZX;
            triTranslated.p[0].z += 3.0f; triTranslated.p[1].z += 3.0f; triTranslated.p[2].z += 3.0f;

            MultiplyMatrixVector(triTranslated.p[0], triProjected.p[0], matProj);
            MultiplyMatrixVector(triTranslated.p[1], triProjected.p[1], matProj);
            MultiplyMatrixVector(triTranslated.p[2], triProjected.p[2], matProj);

            float scale = 0.25f;
            triProjected.p[0].x = (triProjected.p[0].x + 1.0f) * scale * ScreenWidth();
            triProjected.p[0].y = (1.0f - (triProjected.p[0].y + 1.0f) * scale) * ScreenHeight();
            triProjected.p[1].x = (triProjected.p[1].x + 1.0f) * scale * ScreenWidth();
            triProjected.p[1].y = (1.0f - (triProjected.p[1].y + 1.0f) * scale) * ScreenHeight();
            triProjected.p[2].x = (triProjected.p[2].x + 1.0f) * scale * ScreenWidth();
            triProjected.p[2].y = (1.0f - (triProjected.p[2].y + 1.0f) * scale) * ScreenHeight();

            DrawTriangle(
                (int)triProjected.p[0].x, (int)triProjected.p[0].y,
                (int)triProjected.p[1].x, (int)triProjected.p[1].y,
                (int)triProjected.p[2].x, (int)triProjected.p[2].y,
                PIXEL_SOLID, FG_WHITE
            );
        }

        return true;
    }
};

int main()
{
    VertoEngine3D demo;
    if (demo.ConstructConsole(256, 240, 4, 4))
        demo.Start();
    return 0;
}
