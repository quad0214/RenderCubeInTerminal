#pragma once
#include "PixelShader.h"

class SimplePixelShader : public PixelShader {	
public:
	void SetCharacter(wchar_t c);
	virtual PixelShaderManager::OutPixel ExecuteOnePixel(const Pixel& pixel) override;

private:
	wchar_t mCharacter = L' ';
};