#pragma once
#include "Primitive.h"
#include "PixelShaderManager.h"

class PixelShader {
public:
	virtual inline PixelShaderManager::OutPixel ExecuteOnePixel(const Pixel& pixel) = 0;
};