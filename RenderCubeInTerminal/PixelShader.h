#pragma once
#include "SWRasterizer.h"
#include "PixelShaderManager.h"

class PixelShader {
public:
	virtual inline PixelShaderManager::OutPixel ExecuteOnePixel(const SWRasterizer::Pixel& pixel) = 0;
};