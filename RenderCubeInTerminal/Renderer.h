#pragma once


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
#include "SWRasterizer.h"
#include "SimplePixelShader.h"
#include "Primitive.h"

class Renderer {    

public:
    void Initialize(); // init program
    void Terminate(); // terminate program
    std::chrono::steady_clock::time_point Frame(std::chrono::steady_clock::time_point prevFrameSec);
    void Render(const Vertex* vertices, uint32_t vertexNum, const uint32_t* indices, uint32_t indexNum, PixelShader* pixelShader);

private:
    void BeginScene(); // start of render
    void EndScene(); // end of render

    void PrintToTerminal();
    // move to outside of Renderer
    void RenderCube(const float rotationX, const float rotationY, const float rotationZ);
    void ClearBuffer();
        
    void TransformVertexPosition(Vertex* pVertex,
        const Vertex& vertex,
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


private:
    // buffers
#ifdef _WIN32
    HANDLE mHFrontConsole = NULL;
    HANDLE mHBackConsole = NULL;
#endif
    wchar_t mConsoleBuffer[Constants::CONSOLE_SCREEN_HEIGHT * Constants::CONSOLE_SCREEN_WIDTH];
    wchar_t mRenderBuffer[Constants::RENDER_SCREEN_HEIGHT][Constants::RENDER_SCREEN_WIDTH];
    float mZBuffer[Constants::RENDER_SCREEN_HEIGHT][Constants::RENDER_SCREEN_WIDTH];

    // TODO : create model class & model parser 
    const Vertex mVertices[8]{
        Vertex(Vec4(Constants::CUBE_LEN / 2, -Constants::CUBE_LEN / 2, -Constants::CUBE_LEN / 2, 1)),
        Vertex(Vec4(Constants::CUBE_LEN / 2, -Constants::CUBE_LEN / 2, Constants::CUBE_LEN / 2, 1)),
        Vertex(Vec4(-Constants::CUBE_LEN / 2, -Constants::CUBE_LEN / 2, Constants::CUBE_LEN / 2, 1)),
        Vertex(Vec4(-Constants::CUBE_LEN / 2, -Constants::CUBE_LEN / 2, -Constants::CUBE_LEN / 2, 1)),
        Vertex(Vec4(Constants::CUBE_LEN / 2, Constants::CUBE_LEN / 2, -Constants::CUBE_LEN / 2, 1)),
        Vertex(Vec4(Constants::CUBE_LEN / 2, Constants::CUBE_LEN / 2, Constants::CUBE_LEN / 2, 1)),
        Vertex(Vec4(-Constants::CUBE_LEN / 2, Constants::CUBE_LEN / 2, Constants::CUBE_LEN / 2, 1)),
        Vertex(Vec4(-Constants::CUBE_LEN / 2, Constants::CUBE_LEN / 2, -Constants::CUBE_LEN / 2, 1)),
    };

    const uint32_t mIndices[36]{            
        7, 4, 0,
        7, 0, 3,
        4, 5, 1,
        4, 1, 0,
        5, 6, 2,
        5, 2, 1,
        6, 7, 3,
        6, 3, 2,
        6, 5, 4,
        6, 4, 7,
        3, 0, 1,
        3, 1, 2
    };

    // related with text
    int mTextLineIndex = 0;
    wchar_t mTextBuffer[Constants::TEXT_BUFFER_SIZE];

    // rotation
    float mRotationX = 0.0f;
    float mRotationY = 0.0f;
    float mRotationZ = 0.0f;

    // rasterizer
    SWRasterizer* mRasterize = nullptr;

    // pixelShader
    PixelShaderManager* mPixelShaderManager = nullptr;
    SimplePixelShader* mSimplePixelShader =nullptr;

    // viewport
    SWRasterizer::Viewport mViewport;
};