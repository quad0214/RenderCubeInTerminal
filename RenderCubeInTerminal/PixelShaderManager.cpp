#include "PixelShader.h"
#include "PixelShaderManager.h"

PixelShaderManager::PixelShaderManager()
{
}

PixelShaderManager::~PixelShaderManager()
{
	Terminate();
}

void PixelShaderManager::Initialize(uint32_t viewportWidth, uint32_t viewportHeight)
{
	mViewportHeight = viewportHeight;
	mViewportWidth = viewportWidth;
	mOutputPixels = new OutPixel[viewportWidth * viewportHeight];
}

void PixelShaderManager::Terminate()
{

	if (mOutputPixels != nullptr) {
		delete[] mOutputPixels;
		mOutputPixels = nullptr;
	}
}

void PixelShaderManager::ChangeViewport(uint32_t viewportWidth, uint32_t viewportHeight)
{
	if (mViewportHeight == viewportHeight && mViewportWidth == viewportWidth) {
		return;
	}

	mViewportHeight = viewportHeight;
	mViewportWidth = viewportWidth;

	if (mOutputPixels != nullptr) {
		delete[] mOutputPixels;
		mOutputPixels = new OutPixel[viewportWidth * viewportHeight];
	}

}

void PixelShaderManager::SetupPixelShader(PixelShader* pixelShader)
{
	mPixelShader = pixelShader;
}

void PixelShaderManager::Execute(SWRasterizer* rasterizer)
{
	uint64_t pixelLength = rasterizer->GetPixelLength();

	for (int i = 0; i < pixelLength; i++) {
		const Pixel& inputPixel = rasterizer->GetPixel(i);
		mOutputPixels[i] = mPixelShader->ExecuteOnePixel(inputPixel);
	}

	mOutPixelLen = pixelLength;
}
