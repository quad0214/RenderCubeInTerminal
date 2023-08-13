#include "SimplePixelShader.h"

void SimplePixelShader::SetCharacter(wchar_t c)
{
    mCharacter = c;
}

PixelShaderManager::OutPixel SimplePixelShader::ExecuteOnePixel(const SWRasterizer::Pixel& pixel)
{
    PixelShaderManager::OutPixel out;
    out.x = pixel.pos.x;
    out.y = pixel.pos.y;
    out.depth = pixel.pos.z;
    out.c = mCharacter;

    return out;
}
