#include "Renderer.h"
#include "Constants.h"
#include "Math.h"

#undef max;


void Renderer::Initialize() {   
    // console
#ifdef _WIN32
    mHFrontConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    mHBackConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    SetConsoleActiveScreenBuffer(mHFrontConsole);
#endif

    // viewport
    mViewport = SWRasterizer::Viewport(0, 0, Constants::RENDER_SCREEN_WIDTH, Constants::RENDER_SCREEN_HEIGHT, 0, 1.0f);    

    // rasterizer
    mRasterize = new SWRasterizer;
    mRasterize->Initialize();
    mRasterize->SetupViewport(mViewport);

    // pixel shader
    mPixelShaderManager = new PixelShaderManager();
    mPixelShaderManager->Initialize(mViewport.width, mViewport.height);        
    mSimplePixelShader = new SimplePixelShader();                           

    // variable
    mRotationX = 0.0f;
    mRotationY = 0.0f;
    mRotationZ = 0.0f;
}

void Renderer::Terminate() {
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

    if (mRasterize != nullptr) {
        mRasterize->Terminate();
        delete mRasterize;
        mRasterize = nullptr;
    }

    if (mPixelShaderManager != nullptr) {
        mPixelShaderManager->Terminate();
        delete mPixelShaderManager;
        mPixelShaderManager = nullptr;
    }

    if (mSimplePixelShader != nullptr) {
        delete mSimplePixelShader;
        mSimplePixelShader = nullptr;
    }
}

std::chrono::steady_clock::time_point Renderer::Frame(std::chrono::steady_clock::time_point prevFrameSec)
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
    WriteLinesInConsoleBuffer(mConsoleBuffer,
        L"ms per frame: %2.3fms\nrotation: x(%3.2f) y(%3.2f) z(%3.2f) by origin axis", 
        frameDeltaSec * 1000.0f,
        Math::Rad2Deg(mRotationX),
        Math::Rad2Deg(mRotationY),
        Math::Rad2Deg(mRotationZ));

    //      print to terminal        
    PrintToTerminal();

    EndScene();

    return frameTime;
}

void Renderer::Render(const Vertex* vertices, uint32_t vertexNum, const uint32_t* indices, uint32_t indexNum, PixelShader* pixelShader)
{
    // rasterize
    mRasterize->Execute(vertices, vertexNum, indices, indexNum);

    // pixel shader
    mPixelShaderManager->SetupPixelShader(pixelShader);
    mPixelShaderManager->Execute(mRasterize);

    // output merger
    for (int i = 0; i < mPixelShaderManager->GetOutPixelLen(); i++) {
        PixelShaderManager::OutPixel outPixel = mPixelShaderManager->GetOutPixel(i);
        RenderSegmentOnRenderBuffer(static_cast<int>(outPixel.x),
            static_cast<int>(outPixel.y),
            outPixel.depth,
            outPixel.c);
    }
}


void Renderer::BeginScene() { // start of render
    // clear buffer
    ClearBuffer();

    // init text
    mTextLineIndex = 0;
}

void Renderer::EndScene() {
#ifdef _WIN32
    // flip front & back buffer
    HANDLE temp = mHFrontConsole;
    mHFrontConsole = mHBackConsole;
    mHBackConsole = temp;

    SetConsoleActiveScreenBuffer(mHFrontConsole);
#endif
}


void Renderer::PrintToTerminal()
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

void Renderer::RenderCube(const float rotationX, const float rotationY, const float rotationZ) {    
    // vertex shader
    int vertexNum = sizeof(mVertices) / sizeof(mVertices[0]);

    // todo : pooling
    Vertex* projVertices = new Vertex[vertexNum];
    for (int i = 0; i < vertexNum; i++) {
        TransformVertexPosition(&projVertices[i], mVertices[i], rotationX, rotationY, rotationZ);
        //TransformVertexPosition(&projVertices[i], mVertices[i], rotationX, 0, 0);        
    }
   
    int indexNum = sizeof(mIndices) / sizeof(mIndices[0]);
    for (int partIndex = 0; partIndex < indexNum / 6; partIndex++) {
        wchar_t printChars[6] = { L'@', L'#' , L'$' , L'%' , L'=' , L'&' };
        mSimplePixelShader->SetCharacter(printChars[partIndex]);
        Render(projVertices, vertexNum, mIndices + partIndex * 6, 6, mSimplePixelShader);        
    }


    // debugging info : test top-left rule using two triangle contiguous
   /* Vec4 pixelPoses[4]{
        Vec4(3.5f, 3.5f, 0, 0),
        Vec4(1.5f, 5.5f, 0, 0),
        Vec4(5.5f, 5.5f, 0, 0),
        Vec4(3.5f, 7.5f, 0, 0)
    };

    int vertexNum = 4;
    Vertex vertices[4]{};
    for (int i = 0; i < 4; i++) {
        float near = 1.0f / tanf(Constants::FOVY / 2.0f);
        vertices[i].pos = 
            Vec4(
                near * (pixelPoses[i].x * 2.0f / Constants::RENDER_SCREEN_WIDTH - 1.0f),
                near * (-pixelPoses[i].y * 2.0f / Constants::RENDER_SCREEN_HEIGHT + 1.0f),
                0,
                near);
    }

    uint32_t indices[6]{
        2, 1, 0,
        1, 2, 3
    };
    mSimplePixelShader->SetCharacter(L'#');
    Render(vertices, 4, indices, 6, mSimplePixelShader);*/
    

    // move render buffer to conosle buffer
    memcpy(mConsoleBuffer + Constants::CONSOLE_RENDER_SECTION_START, mRenderBuffer, Constants::RENDER_SCREEN_WIDTH * Constants::RENDER_SCREEN_HEIGHT * sizeof(wchar_t));

    delete[] projVertices;    
}

void Renderer::ClearBuffer() {
    memsetAnyByte(reinterpret_cast<wchar_t*>(mRenderBuffer), Constants::RENDER_CLEAR_CHAR, Constants::RENDER_SCREEN_HEIGHT * Constants::RENDER_SCREEN_WIDTH);
    memsetAnyByte(reinterpret_cast<float*>(mZBuffer), std::numeric_limits<float>::max(), Constants::RENDER_SCREEN_HEIGHT * Constants::RENDER_SCREEN_WIDTH);
    memsetAnyByte(reinterpret_cast<wchar_t*>(mConsoleBuffer), Constants::CONSOLE_CLEAR_CHAR, Constants::CONSOLE_SCREEN_HEIGHT * Constants::CONSOLE_SCREEN_WIDTH);
}

void Renderer::TransformVertexPosition(Vertex* pVertex, const Vertex& vertex, const float rotationX, const float rotationY, const float rotationZ)
{
    assert(pVertex != nullptr);

    float near = 1.0f / tanf(Constants::FOVY / 2.0f);
    float far = near + Constants::DST_NEAR_TO_FAR;

    float worldX;
    float worldY;
    float worldZ;
    CalculateRotation(&worldX, &worldY, &worldZ, vertex.pos.x, vertex.pos.y, vertex.pos.z, rotationX, rotationY, rotationZ);

    // because of font which aspect isn't 1, not calculate aspect in projecting processing.
    // it effects showing normal cube by shorten height of cube when it is rendered
    float projX = worldX * near;
    float projY = worldY * near;
    float projZ = (worldZ + abs(Constants::CAMERA_Z) - near) / (far - near);
    float projW = worldZ + abs(Constants::CAMERA_Z);
    
    pVertex->pos = Vec4(projX, projY, projZ, projW);
}

void Renderer::RenderSegmentOnRenderBuffer(const int screenX,
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

void Renderer::WriteLinesInConsoleBuffer(wchar_t* consoleBuffer, const wchar_t* format, ...) {
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

inline void Renderer::CalculateRotation(float* pResultX,
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

inline void Renderer::FillEmptyInConsoleBuffer(wchar_t* consoleBuffer, int start, int len)
{
    for (int i = 0; i < len; i++) {
        (consoleBuffer + start)[i] = L' ';
    }
}

constexpr float Renderer::ClipRadian(float radian)
{
    if (radian > 2.0f * M_PI) {
        return radian - M_PI * floorf(radian / M_PI);
    }
    else if (radian < 0) {
        return radian + M_PI * ceilf(-radian / M_PI);
    }

    return radian;
}
