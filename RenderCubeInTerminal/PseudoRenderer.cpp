#include "PseudoRenderer.h"
#include "Constants.h"

#undef max;


void PseudoRenderer::Initialize() {
    // console
#ifdef _WIN32
    mHFrontConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    mHBackConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    SetConsoleActiveScreenBuffer(mHFrontConsole);
#endif

    mRotationX = 0.0f;
    mRotationY = 0.0f;
    mRotationZ = 0.0f;
}

void PseudoRenderer::Terminate() {
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

std::chrono::steady_clock::time_point PseudoRenderer::Frame(std::chrono::steady_clock::time_point prevFrameSec)
{
    // calculate frame gap
    auto frameTime = std::chrono::high_resolution_clock::now();
    float frameDeltaSec = ((std::chrono::duration<float>)(frameTime - prevFrameSec)).count();    

    // update rotation
    mRotationX += Constants::ROTATION_X_SPEED * frameDeltaSec;
    mRotationX = ClipRadian(mRotationX);
    mRotationY += Constants::ROTATION_Y_SPEED * frameDeltaSec;
    mRotationY = ClipRadian(mRotationY);
    mRotationZ += Constants::ROTATION_Z_SPEED * frameDeltaSec;
    mRotationZ = ClipRadian(mRotationZ);


    // render
    BeginScene();

    //      print cube    
    RenderCube(mRotationX, mRotationY, mRotationZ);

    //      print informations
    WriteLinesInConsoleBuffer(mConsoleBuffer, L"ms per frame: %2.3fms\nrotation: x(%3.2f) y(%3.2f) z(%3.2f) by origin axis", frameDeltaSec * 1000.0f, mRotationX, mRotationY, mRotationZ);

    //      print to terminal        
    PrintToTerminal();

    EndScene();

    return frameTime;
}


void PseudoRenderer::BeginScene() { // start of render
    // clear buffer
    ClearBuffer();

    // init text
    mTextLineIndex = 0;
}

void PseudoRenderer::EndScene() {
#ifdef _WIN32
    // flip front & back buffer
    HANDLE temp = mHFrontConsole;
    mHFrontConsole = mHBackConsole;
    mHBackConsole = temp;

    SetConsoleActiveScreenBuffer(mHFrontConsole);
#endif
}


void PseudoRenderer::PrintToTerminal()
{
#ifdef _WIN32    
    // write to console
    DWORD dwWrittenBytes = 0;
    WriteConsoleOutputCharacter(mHBackConsole, mConsoleBuffer, Constants::CONSOLE_SCREEN_WIDTH * Constants::CONSOLE_SCREEN_HEIGHT, { 0, 0 }, &dwWrittenBytes);
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

void PseudoRenderer::RenderCube(const float rotationX, const float rotationY, const float rotationZ) {
    // render on buffer        
    int halfCubeLen = Constants::CUBE_LEN / 2;
    for (int planeY = -halfCubeLen; planeY <= halfCubeLen; planeY++) {
        for (int planeX = -halfCubeLen; planeX <= halfCubeLen; planeX++) {
            int screenX;
            int screenY;
            float depth;

            // render 6 side
            //      plane parallel to xy plane(rear)
            CalculateScreenSpace(&screenX, &screenY, &depth, planeX, planeY, -Constants::CUBE_LEN / 2, rotationX, rotationY, rotationZ);
            RenderSegmentOnRenderBuffer(screenX, screenY, depth, L'@');
            //      plane parallel to xy plane(front)
            CalculateScreenSpace(&screenX, &screenY, &depth, planeX, planeY, Constants::CUBE_LEN / 2, rotationX, rotationY, rotationZ);
            RenderSegmentOnRenderBuffer(screenX, screenY, depth, L'#');
            //      plane parallel to xz plane(botttom)
            CalculateScreenSpace(&screenX, &screenY, &depth, planeX, -Constants::CUBE_LEN / 2, planeY, rotationX, rotationY, rotationZ);
            RenderSegmentOnRenderBuffer(screenX, screenY, depth, L'$');
            //      plane parallel to xz plane(top)
            CalculateScreenSpace(&screenX, &screenY, &depth, planeX, Constants::CUBE_LEN / 2, planeY, rotationX, rotationY, rotationZ);
            RenderSegmentOnRenderBuffer(screenX, screenY, depth, L'%');
            //      plane parallel to yz plane(left)
            CalculateScreenSpace(&screenX, &screenY, &depth, -Constants::CUBE_LEN / 2, planeX, planeY, rotationX, rotationY, rotationZ);
            RenderSegmentOnRenderBuffer(screenX, screenY, depth, L'=');
            //      plane parallel to yz plane(right)
            CalculateScreenSpace(&screenX, &screenY, &depth, Constants::CUBE_LEN / 2, planeX, planeY, rotationX, rotationY, rotationZ);
            RenderSegmentOnRenderBuffer(screenX, screenY, depth, L'&');
        }
    }

    // move render buffer to conosle buffer
    memcpy(mConsoleBuffer + Constants::CONSOLE_RENDER_SECTION_START, mRenderBuffer, Constants::RENDER_SCREEN_WIDTH * Constants::RENDER_SCREEN_HEIGHT * sizeof(wchar_t));
}

void PseudoRenderer::ClearBuffer() {
    memsetAnyByte(reinterpret_cast<wchar_t*>(mRenderBuffer), Constants::RENDER_CLEAR_CHAR, Constants::RENDER_SCREEN_HEIGHT * Constants::RENDER_SCREEN_WIDTH);
    memsetAnyByte(reinterpret_cast<float*>(mZBuffer), std::numeric_limits<float>::max(), Constants::RENDER_SCREEN_HEIGHT * Constants::RENDER_SCREEN_WIDTH);
    memsetAnyByte(reinterpret_cast<wchar_t*>(mConsoleBuffer), Constants::CONSOLE_CLEAR_CHAR, Constants::CONSOLE_SCREEN_HEIGHT * Constants::CONSOLE_SCREEN_WIDTH);
}


void PseudoRenderer::CalculateScreenSpace(int* pScreenX,
    int* pScreenY,
    float* pDepth,
    const float objX,
    const float objY,
    const float objZ,
    const float rotationX,
    const float rotationY,
    const float rotationZ)
{
    float near = 1.0f / tanf(Constants::FOVY / 2.0f);
    float far = near + Constants::DST_NEAR_TO_FAR;

    float worldX;
    float worldY;
    float worldZ;
    CalculateRotation(&worldX, &worldY, &worldZ, objX, objY, objZ, rotationX, rotationY, rotationZ);

    // because of font which aspect isn't 1, not calculate aspect in projecting processing.
    // it effects showing normal cube by shorten height of cube when it is rendered
    float projX = worldX * near;
    float projY = worldY * near;
    float projZ = (worldZ + abs(Constants::CAMERA_Z) - near) / (far - near);
    float projW = worldZ + abs(Constants::CAMERA_Z);

    float projX01 = (projX / projW + 1.0f) / 2.0f;
    float projY01 = (projY / projW + 1.0f) / 2.0f;
    float projZ01 = projZ / projW;

    *pScreenX = projX01 * Constants::RENDER_SCREEN_WIDTH;
    *pScreenY = projY01 * Constants::RENDER_SCREEN_HEIGHT;
    *pDepth = projZ01;
}

void PseudoRenderer::RenderSegmentOnRenderBuffer(const int screenX,
    const int screenY,
    const float depth,
    const wchar_t renderChar)
{
    // if input pixel is out of screen, doesn't render
    if (screenX < 0 || screenX >= Constants::RENDER_SCREEN_WIDTH
        || screenY < 0 || screenY >= Constants::RENDER_SCREEN_HEIGHT) {
        return;
    }

    // depth testing
    if (depth >= mZBuffer[screenY][screenX]) {
        return;
    }

    mZBuffer[screenY][screenX] = depth;
    mRenderBuffer[screenY][screenX] = renderChar;
}

void PseudoRenderer::WriteLinesInConsoleBuffer(wchar_t* consoleBuffer, const wchar_t* format, ...) {
    if (mTextLineIndex >= Constants::CONSOLE_TEXT_HEIGHT) {
        return;
    }

    // get formatted string
    va_list args;
    va_start(args, format);
    vswprintf(mTextBuffer, Constants::CONSOLE_MAX_TEXT_LEN, format, args);
    va_end(args);

    // split line & adjust to console buffer
    wchar_t* temp = nullptr;
    wchar_t* line = wcstok_s(mTextBuffer, L"\n", &temp);
    size_t remainCharLen = Constants::CONSOLE_MAX_TEXT_LEN - mTextLineIndex * Constants::CONSOLE_SCREEN_WIDTH;
    while (remainCharLen > 0) {
        size_t lineCharLen = wcslen(line);
        memcpy(consoleBuffer + mTextLineIndex * Constants::CONSOLE_SCREEN_WIDTH, line, min(lineCharLen, remainCharLen) * sizeof(wchar_t));
        remainCharLen -= Constants::CONSOLE_SCREEN_WIDTH;

        mTextLineIndex += lineCharLen / Constants::CONSOLE_SCREEN_WIDTH + 1;

        line = wcstok_s(nullptr, L"\n", &temp);
        if (line == nullptr) {
            break;
        }

    }
}

inline void PseudoRenderer::CalculateRotation(float* pResultX,
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

inline void PseudoRenderer::FillEmptyInConsoleBuffer(wchar_t* consoleBuffer, int start, int len)
{
    for (int i = 0; i < len; i++) {
        (consoleBuffer + start)[i] = L' ';
    }
}

constexpr float PseudoRenderer::ClipRadian(float radian)
{
    if (radian > 2.0f * M_PI) {
        return radian - M_PI * floorf(radian / M_PI);
    }
    else if (radian < 0) {
        return radian + M_PI * ceilf(-radian / M_PI);
    }

    return radian;
}
