// inspired by : https://youtu.be/p09i_hoFdd0

#include <iostream>
#define _USE_MATH_DEFINES
#include <math.h> // why cmath doesn't include math constants?
#include <chrono>
#include <thread>
#include <iomanip>

#define TERMINAL_FONT_ASPECT (8.0f / 18.0f)

// assume 
// cube pos : (0, 0, 0, 1)
// camera pos : (0, 0, CAMERA_Z, 1)
// 
// world space basis same with view space basis.
// screen space coordinate system :
//      origin is left top of screen

// changable variable
const int CUBE_LEN = 70;
const int SCREEN_WIDTH = 50;
const int SCREEN_HEIGHT = SCREEN_WIDTH * TERMINAL_FONT_ASPECT;
constexpr float FOVY = 90.0f * M_PI / 180.0f;
const float DST_NEAR_TO_FAR = 200;
const float CAMERA_Z = -CUBE_LEN / 2.0f - 50.0f;
const float ROTATION_DELTA_X = 1.0f * M_PI / 180.0f;
const float ROTATION_DELTA_Y = 0.5f * M_PI / 180.0f;
const float ROTATION_DELTA_Z = 0.3f * M_PI / 180.0f;


char buffer[SCREEN_HEIGHT][SCREEN_WIDTH];
float zBuffer[SCREEN_HEIGHT][SCREEN_WIDTH];

void PrintToTerminal(int renderMs, float rotationX, float rotationY, float rotationZ);
void Render(const float rotationX, const float rotationY, const float rotationZ);
void PrintBuffer();
void ClearBuffer();
void CalculateScreenSpace(int* pScreenX,
    int* pScreenY,
    float* pDepth,
    const float objX,
    const float objY, 
    const float objZ, 
    const float rotationX, 
    const float rotationY, 
    const float rotationZ);

void RenderOnBuffer(const int screenX, 
    const int screenY, 
    const float depth,
    const char renderChar);

inline void CalculateRotation(float* pResultX,
    float* pResultY,
    float* pResultZ, 
    const float x,
    const float y,
    const float z, 
    const float rotX, 
    const float rotY,
    const float rotZ);

constexpr float Rad2Deg(float radian) { return radian * 180.0f / M_PI; }
constexpr float Deg2Rad(float degree) { return degree * M_PI / 180.0f; }
constexpr float ClipRadian(float radian);

int main()
{    
    // init
    float rotationX = 0.0f;
    float rotationY = 0.0f;
    float rotationZ = 0.0f;

    while (true) {              
        // update rotation
        rotationX += ROTATION_DELTA_X;
        rotationX = ClipRadian(rotationX);
        rotationY += ROTATION_DELTA_Y;
        rotationY = ClipRadian(rotationY);
        rotationZ += ROTATION_DELTA_Z;
        rotationZ = ClipRadian(rotationZ);

        // render
        auto start = std::chrono::high_resolution_clock::now();        
        Render(rotationX, rotationY, rotationZ);
        auto end = std::chrono::high_resolution_clock::now();
        
        float rotationMs = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();        

        // print to terminal        
        PrintToTerminal(rotationMs, rotationX, rotationY, rotationZ);                
    }
}

void PrintToTerminal(int renderMs, float rotationX, float rotationY, float rotationZ)
{
    // clear terminal
    std::cout << "\x1B[2J\x1B[H";

    // print information
    std::cout << "ms per frame: " << renderMs << "ms" << std::endl;
    std::cout << std::fixed << std::setprecision(2)
        << "rotation: "
        << "x(" << Rad2Deg(rotationX) << "deg)"
        << " y(" << Rad2Deg(rotationY) << "deg)"
        << " z(" << Rad2Deg(rotationZ) << "deg)"
        << " by origin axis"
        << std::endl;


    // print buffer
    PrintBuffer();

    std::cout << std::flush;
}

void Render(const float rotationX, const float rotationY, const float rotationZ) {
    // clear buffer
    ClearBuffer();

    // render on buffer        
    int halfCubeLen = CUBE_LEN / 2;
    for (int planeY = -halfCubeLen; planeY <= halfCubeLen; planeY++) {
        for (int planeX = -halfCubeLen; planeX <= halfCubeLen; planeX++) {
            int screenX;
            int screenY;
            float depth;

            // render 6 side
            //      plane parallel to xy plane(rear)
            CalculateScreenSpace(&screenX, &screenY, &depth, planeX, planeY, -CUBE_LEN/2, rotationX, rotationY, rotationZ);
            RenderOnBuffer(screenX, screenY, depth, '@');
            //      plane parallel to xy plane(front)
            CalculateScreenSpace(&screenX, &screenY, &depth, planeX, planeY, CUBE_LEN / 2, rotationX, rotationY, rotationZ);
            RenderOnBuffer(screenX, screenY, depth, '#');
            //      plane parallel to xz plane(botttom)
            CalculateScreenSpace(&screenX, &screenY, &depth, planeX, -CUBE_LEN / 2, planeY, rotationX, rotationY, rotationZ);
            RenderOnBuffer(screenX, screenY, depth, '$');
            //      plane parallel to xz plane(top)
            CalculateScreenSpace(&screenX, &screenY, &depth, planeX, CUBE_LEN / 2, planeY, rotationX, rotationY, rotationZ);
            RenderOnBuffer(screenX, screenY, depth, '%');
            //      plane parallel to yz plane(left)
            CalculateScreenSpace(&screenX, &screenY, &depth, -CUBE_LEN / 2, planeX, planeY, rotationX, rotationY, rotationZ);
            RenderOnBuffer(screenX, screenY, depth, '=');
            //      plane parallel to yz plane(right)
            CalculateScreenSpace(&screenX, &screenY, &depth, CUBE_LEN / 2, planeX, planeY, rotationX, rotationY, rotationZ);
            RenderOnBuffer(screenX, screenY, depth, '&');
        }
    }    
}

void ClearBuffer() {
    memset(buffer, (int)'.', sizeof(char) * SCREEN_WIDTH * SCREEN_HEIGHT);
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            zBuffer[y][x] = std::numeric_limits<float>::max();
        }
    }    
}

void PrintBuffer()
{
    for (int i = 0; i < SCREEN_HEIGHT; i++) {
        for (int j = 0; j < SCREEN_WIDTH; j++) {
            std::cout << buffer[i][j];
        }
        std::cout << '\n';
    }    
}

void CalculateScreenSpace(int* pScreenX,
    int* pScreenY,
    float* pDepth,
    const float objX,
    const float objY,
    const float objZ,
    const float rotationX,
    const float rotationY,
    const float rotationZ)
{
    float near = 1.0f / tanf(FOVY / 2.0f);
    float far = near + DST_NEAR_TO_FAR;

    float worldX;
    float worldY;
    float worldZ;
    CalculateRotation(&worldX, &worldY, &worldZ, objX, objY, objZ, rotationX, rotationY, rotationZ);
   
    // because of font which aspect isn't 1, not calculate aspect in projecting processing.
    // it effects showing normal cube by shorten height of cube when it is rendered
    float projX = worldX * near;
    float projY = worldY * near;
    float projZ = (worldZ + abs(CAMERA_Z) - near) / (far - near);
    float projW = worldZ + abs(CAMERA_Z);

    float projX01 = (projX / projW + 1.0f) / 2.0f;
    float projY01 = (projY / projW + 1.0f) / 2.0f;
    float projZ01 = projZ / projW;
    
    *pScreenX = projX01 * SCREEN_WIDTH;
    *pScreenY = projY01 * SCREEN_HEIGHT;
    *pDepth = projZ01;
}

void RenderOnBuffer(const int screenX,
    const int screenY, 
    const float depth,
    const char renderChar)
{
    // if input pixel is out of screen, doesn't render
    if (screenX < 0 || screenX >= SCREEN_WIDTH
        || screenY < 0 || screenY >= SCREEN_HEIGHT) {
        return;
    }

    // depth testing
    if (depth >= zBuffer[screenY][screenX]) {
        return;
    }

    zBuffer[screenY][screenX] = depth;
    buffer[screenY][screenX] = renderChar;
}

inline void CalculateRotation(float* pResultX,
    float* pResultY,
    float* pResultZ,
    const float x,
    const float y,
    const float z,
    const float rotX,
    const float rotY,
    const float rotZ)
{
    float cosA = cos(rotX);
    float cosB = cos(rotY);
    float cosC = cos(rotZ);
    float sinA = sin(rotX);
    float sinB = sin(rotY);
    float sinC = sin(rotZ);

    *pResultX = x * cosB * cosC + z * sinB - y * cosB * sinC;
    *pResultY = x * (sinA * sinB * cosC + cosA * sinC) + y * (cosA * cosC - sinA * sinB * sinC) - z * sinA * cosB;
    *pResultZ = x * (sinA * sinC - cosA * sinB * cosC) + y * (cosA * sinB * sinC + sinA * cosC) + z * cosA * cosB;
}

constexpr float ClipRadian(float radian)
{
    if (radian > 2.0f * M_PI) {
        return radian - M_PI * floorf(radian / M_PI);
    }
    else if (radian < 0) {
        return radian + M_PI * ceilf(-radian / M_PI);
    }

    return radian;
}


