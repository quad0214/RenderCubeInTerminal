#pragma once

// inspired by : https://youtu.be/p09i_hoFdd0

/*
* [assume]
* cube pos : (0, 0, 0, 1)
* camera pos : (0, 0, CAMERA_Z, 1)
*
* world space basis same with view space basis.
* screen space coordinate system :
*      origin is left top of screen
*      below = +y, right = +x
*
*
* [console structure]
* Text Section
* Render Section
*/

#include <iostream>
#define _USE_MATH_DEFINES
#include <math.h> // why cmath doesn't include math constants?
#include <chrono>
#include <thread>
#include <iomanip>
#include <cstdarg>

#ifdef _WIN32
#include <Windows.h>
#undef max
#undef near
#undef far
#endif
#define min(x, y) (x > y ? y : x)

#include "Constants.h"
#include "Math.h"

class PseudoRenderer {
public:    
    void Initialize(); // init program
    void Terminate(); // terminate program
    std::chrono::steady_clock::time_point Frame(std::chrono::steady_clock::time_point prevFrameSec);

private:
    void BeginScene(); // start of render
    void EndScene(); // end of render

    void PrintToTerminal();
    void RenderCube(const float rotationX, const float rotationY, const float rotationZ);
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

    void RenderSegmentOnRenderBuffer(const int screenX,
        const int screenY,
        const float depth,
        const wchar_t renderChar);

    //      related text
    void WriteLinesInConsoleBuffer(wchar_t* consoleBuffer, const wchar_t* format, ...);

    //      helper
    inline void FillEmptyInConsoleBuffer(wchar_t* consoleBuffer, int start, int len);

    inline void CalculateRotation(float* pResultX,
        float* pResultY,
        float* pResultZ,
        const float x,
        const float y,
        const float z,
        const float rotX,
        const float rotY,
        const float rotZ);
    //      source : https://stackoverflow.com/a/57740899
    template<typename T>
    static inline void memsetAnyByte(T* __restrict dst, T val, int len) {
        T* end = dst + (len & ~0x1f); //round down to nearest multiple of 32 | 0x1f = 32
        while (dst != end) { //copy 32 times
            dst[0] = val;
            dst[1] = val;
            dst[2] = val;
            dst[3] = val;
            dst[4] = val;
            dst[5] = val;
            dst[6] = val;
            dst[7] = val;
            dst[8] = val;
            dst[9] = val;
            dst[10] = val;
            dst[11] = val;
            dst[12] = val;
            dst[13] = val;
            dst[14] = val;
            dst[15] = val;
            dst[16] = val;
            dst[17] = val;
            dst[18] = val;
            dst[19] = val;
            dst[20] = val;
            dst[21] = val;
            dst[22] = val;
            dst[23] = val;
            dst[24] = val;
            dst[25] = val;
            dst[26] = val;
            dst[27] = val;
            dst[28] = val;
            dst[29] = val;
            dst[30] = val;
            dst[31] = val;
            dst += 32;
        }
        end += len & 0x1f; //remained | 0x1f = 32
        while (dst != end) *dst++ = val; //copy remaining bytes
    }
    constexpr float ClipRadian(float radian);


private :    
    // buffers
#ifdef _WIN32
    HANDLE mHFrontConsole = NULL;
    HANDLE mHBackConsole = NULL;
#endif
    wchar_t mConsoleBuffer[Constants::CONSOLE_SCREEN_HEIGHT * Constants::CONSOLE_SCREEN_WIDTH];
    wchar_t mRenderBuffer[Constants::RENDER_SCREEN_HEIGHT][Constants::RENDER_SCREEN_WIDTH];
    float mZBuffer[Constants::RENDER_SCREEN_HEIGHT][Constants::RENDER_SCREEN_WIDTH];

    // related with text
    int mTextLineIndex = 0;
    wchar_t mTextBuffer[Constants::TEXT_BUFFER_SIZE];

    // rotation
    float mRotationX = 0.0f;
    float mRotationY = 0.0f;
    float mRotationZ = 0.0f;
   

};