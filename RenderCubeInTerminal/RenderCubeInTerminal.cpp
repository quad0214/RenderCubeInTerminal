// inspired by : https://youtu.be/p09i_hoFdd0

//#undef _WIN32

/*
* [todo]
*   - double buffering
*   - rasterizer
*   - shading
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

#define TERMINAL_FONT_ASPECT (8.0f / 18.0f)

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

// [constants]
const int CUBE_LEN = 70; // length of cube side
const int RENDER_SCREEN_WIDTH = 50; // width of screen where cube is rendered
const int RENDER_SCREEN_HEIGHT = RENDER_SCREEN_WIDTH * TERMINAL_FONT_ASPECT; // height of screen where cube is rendered
constexpr float FOVY = 90.0f * M_PI / 180.0f; 
const float DST_NEAR_TO_FAR = 200;
const float CAMERA_Z = -CUBE_LEN / 2.0f - 50.0f;
const float ROTATION_X_SPEED = 10.0f * M_PI / 180.0f;
const float ROTATION_Y_SPEED = 5.0f * M_PI / 180.0f;
const float ROTATION_Z_SPEED = 2.5f * M_PI / 180.0f;
const wchar_t RENDER_CLEAR_CHAR = L'.';

const int CONSOLE_TEXT_HEIGHT = 5; // height of console infomration text. regarding only in windows
const int CONSOLE_SCREEN_WIDTH = RENDER_SCREEN_WIDTH; //  console width. regarding only in windows
const int CONSOLE_SCREEN_HEIGHT = RENDER_SCREEN_HEIGHT + CONSOLE_TEXT_HEIGHT; // console height. regarding only in windows
const int CONSOLE_MAX_TEXT_LEN = CONSOLE_TEXT_HEIGHT * CONSOLE_SCREEN_WIDTH;
const int TEXT_BUFFER_SIZE = CONSOLE_MAX_TEXT_LEN + 1;
const wchar_t CONSOLE_CLEAR_CHAR = L' ';

const int CONSOLE_TEXT_SECTION_START = 0;
const int CONSOLE_RENDER_SECTION_START = CONSOLE_TEXT_SECTION_START + CONSOLE_MAX_TEXT_LEN;

// [buffers]
#ifdef _WIN32
HANDLE mHFrontConsole = NULL;
HANDLE mHBackConsole = NULL;
#endif
wchar_t mConsoleBuffer[CONSOLE_SCREEN_HEIGHT * CONSOLE_SCREEN_WIDTH];
wchar_t mRenderBuffer[RENDER_SCREEN_HEIGHT][RENDER_SCREEN_WIDTH];
float mZBuffer[RENDER_SCREEN_HEIGHT][RENDER_SCREEN_WIDTH];

// [related with text]
int mTextLineIndex = 0;
wchar_t mTextBuffer[TEXT_BUFFER_SIZE];

// [methods]
void Initialize(); // init program
void Terminate(); // terminate program
 
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
constexpr float Rad2Deg(float radian) { return radian * 180.0f / M_PI; }
constexpr float Deg2Rad(float degree) { return degree * M_PI / 180.0f; }
constexpr float ClipRadian(float radian);

int main()
{    
    // init
    Initialize();

    float rotationX = 0.0f;
    float rotationY = 0.0f;
    float rotationZ = 0.0f;

    float frameDeltaSec = 0;
    auto prevFrameTime = std::chrono::high_resolution_clock::now();
    while (true) {       
        // calculate frame gap
        auto frameTime = std::chrono::high_resolution_clock::now();
        frameDeltaSec = ((std::chrono::duration<float>)(frameTime - prevFrameTime)).count();
        prevFrameTime = frameTime;

        // update rotation
        rotationX += ROTATION_X_SPEED * frameDeltaSec;
        rotationX = ClipRadian(rotationX);
        rotationY += ROTATION_Y_SPEED * frameDeltaSec;
        rotationY = ClipRadian(rotationY);
        rotationZ += ROTATION_Z_SPEED * frameDeltaSec;
        rotationZ = ClipRadian(rotationZ);

     
        // render
        BeginScene();
       
        //      print cube    
        RenderCube(rotationX, rotationY, rotationZ);
        
        //      print informations
        WriteLinesInConsoleBuffer(mConsoleBuffer, L"ms per frame: %2.3fms\nrotation: x(%3.2f) y(%3.2f) z(%3.2f) by origin axis", frameDeltaSec * 1000.0f, rotationX, rotationY, rotationZ);

        //      print to terminal        
        PrintToTerminal();

        EndScene();
    }

    // terminate
    Terminate();
}

void Initialize() {
    // console
#ifdef _WIN32
    mHFrontConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    mHBackConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    SetConsoleActiveScreenBuffer(mHFrontConsole);
#endif
}

void Terminate() {
#ifdef _WIN32
    // console
    if (mHFrontConsole != NULL) {
        CloseHandle(mHFrontConsole);
        mHFrontConsole = NULL;
    }

    if (mHBackConsole != NULL) {
        CloseHandle(mHBackConsole);
        mHBackConsole = NULL;
    }
#endif
}

void BeginScene() { // start of render
    // clear buffer
    ClearBuffer();

    // init text
    mTextLineIndex = 0;
}

void EndScene() {
#ifdef _WIN32
    // flip front & back buffer
    HANDLE temp = mHFrontConsole;
    mHFrontConsole = mHBackConsole;
    mHBackConsole = temp;
    
    SetConsoleActiveScreenBuffer(mHFrontConsole);
#endif
}


void PrintToTerminal()
{
#ifdef _WIN32    
    // write to console
    DWORD dwWrittenBytes = 0;
    WriteConsoleOutputCharacter(mHBackConsole, mConsoleBuffer, CONSOLE_SCREEN_WIDTH * CONSOLE_SCREEN_HEIGHT, { 0, 0 }, &dwWrittenBytes);
#else
    // clear terminal
    std::wcout << "\x1B[2J\x1B[H";

    for (int y = 0; y < CONSOLE_SCREEN_HEIGHT; y++) {
        for (int x = 0; x < CONSOLE_SCREEN_WIDTH; x++) {
            std::wcout << mConsoleBuffer[y * CONSOLE_SCREEN_WIDTH + x];
        }
        std::wcout << L'\n';
    }

    std::wcout << std::flush;
#endif
}

void RenderCube(const float rotationX, const float rotationY, const float rotationZ) {
    // render on buffer        
    int halfCubeLen = CUBE_LEN / 2;
    for (int planeY = -halfCubeLen; planeY <= halfCubeLen; planeY++) {
        for (int planeX = -halfCubeLen; planeX <= halfCubeLen; planeX++) {
            int screenX;
            int screenY;
            float depth;

            // render 6 side
            //      plane parallel to xy plane(rear)
            CalculateScreenSpace(&screenX, &screenY, &depth, planeX, planeY, -CUBE_LEN / 2, rotationX, rotationY, rotationZ);
            RenderSegmentOnRenderBuffer(screenX, screenY, depth, L'@');
            //      plane parallel to xy plane(front)
            CalculateScreenSpace(&screenX, &screenY, &depth, planeX, planeY, CUBE_LEN / 2, rotationX, rotationY, rotationZ);
            RenderSegmentOnRenderBuffer(screenX, screenY, depth, L'#');
            //      plane parallel to xz plane(botttom)
            CalculateScreenSpace(&screenX, &screenY, &depth, planeX, -CUBE_LEN / 2, planeY, rotationX, rotationY, rotationZ);
            RenderSegmentOnRenderBuffer(screenX, screenY, depth, L'$');
            //      plane parallel to xz plane(top)
            CalculateScreenSpace(&screenX, &screenY, &depth, planeX, CUBE_LEN / 2, planeY, rotationX, rotationY, rotationZ);
            RenderSegmentOnRenderBuffer(screenX, screenY, depth, L'%');
            //      plane parallel to yz plane(left)
            CalculateScreenSpace(&screenX, &screenY, &depth, -CUBE_LEN / 2, planeX, planeY, rotationX, rotationY, rotationZ);
            RenderSegmentOnRenderBuffer(screenX, screenY, depth, L'=');
            //      plane parallel to yz plane(right)
            CalculateScreenSpace(&screenX, &screenY, &depth, CUBE_LEN / 2, planeX, planeY, rotationX, rotationY, rotationZ);
            RenderSegmentOnRenderBuffer(screenX, screenY, depth, L'&');
        }
    }

    // move render buffer to conosle buffer
    memcpy(mConsoleBuffer + CONSOLE_RENDER_SECTION_START, mRenderBuffer, RENDER_SCREEN_WIDTH * RENDER_SCREEN_HEIGHT * sizeof(wchar_t));
}

void ClearBuffer() {
    memsetAnyByte(reinterpret_cast<wchar_t*>(mRenderBuffer), RENDER_CLEAR_CHAR, RENDER_SCREEN_HEIGHT * RENDER_SCREEN_WIDTH);
    memsetAnyByte(reinterpret_cast<float*>(mZBuffer), std::numeric_limits<float>::max(), RENDER_SCREEN_HEIGHT * RENDER_SCREEN_WIDTH);
    memsetAnyByte(reinterpret_cast<wchar_t*>(mConsoleBuffer), CONSOLE_CLEAR_CHAR, CONSOLE_SCREEN_HEIGHT * CONSOLE_SCREEN_WIDTH);
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
    
    *pScreenX = projX01 * RENDER_SCREEN_WIDTH;
    *pScreenY = projY01 * RENDER_SCREEN_HEIGHT;
    *pDepth = projZ01;
}

void RenderSegmentOnRenderBuffer(const int screenX,
    const int screenY, 
    const float depth,
    const wchar_t renderChar)
{
    // if input pixel is out of screen, doesn't render
    if (screenX < 0 || screenX >= RENDER_SCREEN_WIDTH
        || screenY < 0 || screenY >= RENDER_SCREEN_HEIGHT) {
        return;
    }

    // depth testing
    if (depth >= mZBuffer[screenY][screenX]) {
        return;
    }

    mZBuffer[screenY][screenX] = depth;
    mRenderBuffer[screenY][screenX] = renderChar;
}

void WriteLinesInConsoleBuffer(wchar_t* consoleBuffer, const wchar_t* format, ...) {
    if (mTextLineIndex >= CONSOLE_TEXT_HEIGHT) {
        return;
    }

    // get formatted string
    va_list args;
    va_start(args, format);
    vswprintf(mTextBuffer, CONSOLE_MAX_TEXT_LEN, format, args);
    va_end(args);

    // split line & adjust to console buffer
    wchar_t* temp = nullptr;
    wchar_t* line = wcstok_s(mTextBuffer, L"\n", &temp);
    size_t remainCharLen = CONSOLE_MAX_TEXT_LEN - mTextLineIndex * CONSOLE_SCREEN_WIDTH;
    while (remainCharLen > 0) {
        size_t lineCharLen = wcslen(line);
        memcpy(consoleBuffer + mTextLineIndex * CONSOLE_SCREEN_WIDTH, line, min(lineCharLen, remainCharLen) * sizeof(wchar_t));
        remainCharLen -= CONSOLE_SCREEN_WIDTH;

        mTextLineIndex += lineCharLen / CONSOLE_SCREEN_WIDTH + 1;

        line = wcstok_s(nullptr, L"\n", &temp);
        if (line == nullptr) {
            break;
        }

    }
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

inline void FillEmptyInConsoleBuffer(wchar_t* consoleBuffer, int start, int len)
{
    for (int i = 0; i < len; i++) {
        (consoleBuffer + start)[i] = L' ';
    }
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