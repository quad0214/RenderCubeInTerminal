#pragma once
#include "SWRasterizer.h"

class PixelShader;
class PixelShaderManager {
public:
	struct OutPixel {
		float x;
		float y;
		float depth;
		wchar_t c;
	};

public:
	PixelShaderManager();
	~PixelShaderManager();

	void Initialize(uint32_t viewportWidth, uint32_t viewportHeight);
	void Terminate();
	void ChangeViewport(uint32_t viewportWidth, uint32_t viewportHeight);
	void SetupPixelShader(PixelShader* pixelShader);

	void Execute(SWRasterizer* rasterizer);

	inline const OutPixel& GetOutPixel(uint32_t index) const {
		assert(index < mOutPixelLen);

		return mOutputPixels[index];
	}
	inline uint32_t GetOutPixelLen() const {
		return mOutPixelLen;
	}


private:
	// todo : change output pixels memory from each pixel shader to all pixel shader
	//		create pixel shader manager, and borrow it.
	//		assume it uses in single thread
	OutPixel* mOutputPixels;
	uint32_t mViewportWidth = 0;
	uint32_t mViewportHeight = 0;

	uint32_t mOutPixelLen = 0;

	PixelShader* mPixelShader = nullptr;
};