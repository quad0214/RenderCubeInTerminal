#pragma once
#include "IRasterizable.h"
#include "Primitive.h"

class RasterizeFloating : public IRasterizable {
public:
	void Rasterize(List* pixels, const List* floatingVertices, const List* indices) override;	
	virtual ~RasterizeFloating() override {}

private:
	inline DF EdgeFunctionFixedPoint(const FixedVec4& pixelPos, const FixedVec4& v0Pos, const FixedVec4& v1Pos) const
	{
		FixedVec4 pMinusV0 = pixelPos - v0Pos;
		FixedVec4 v1MinusV0 = v1Pos - v0Pos;

		DF v0PX = pMinusV0.x.ToDoubleFixedPoint();
		DF v0V1Y = v1MinusV0.y.ToDoubleFixedPoint();
		DF v0PY = pMinusV0.y.ToDoubleFixedPoint();
		DF v0V1X = v1MinusV0.x.ToDoubleFixedPoint();

		return v0PX * v0V1Y - v0PY * v0V1X;
	}

	inline float EdgeFunctionFloatingPoint(const Vec4& pixelPos, const Vec4& v0Pos, const Vec4 v1Pos) const
	{
		Vec4 pMinusV0 = pixelPos - v0Pos;
		Vec4 v1MinusV0 = v1Pos - v0Pos;

		return pMinusV0.x * v1MinusV0.y - pMinusV0.y * v1MinusV0.x;
	}

	void RasterizePartFloatingPoint(
		const Vec4& v0Pos,
		const Vec4& v1Pos,
		const Vec4& v2Pos,
		float leftX,
		float topY,
		uint8_t pixelLengthLog2,
		float edge01,
		float edge12,
		float edge20);


	inline void AddPixelIsInTriangleFloatingPoint(
		const Vec4& v0Pos,
		const Vec4& v1Pos,
		const Vec4& v2Pos,
		float x,
		float y,
		float edge01,
		float edge12,
		float edge23
	);

	inline bool IsSelectPixelFloatingPoint(
		const Vec4& v0Pos,
		const Vec4& v1Pos,
		const Vec4& v2Pos,
		float x,
		float y,
		float edge01,
		float edge12,
		float edge20
	);

	inline bool IsLeftLineFloatingPoint(const Vec4& prePos, const Vec4& nextPos) const;
	inline bool IsTopLineFloatingPoint(const Vec4& prePos, const Vec4& nextPos) const;


private:
	// dxEdgeFunctionValue, 2 * dxEdgeFunctionValue, 4 * dxEdgeFunctionValue, 8 * dxEdgeFunctionValue
	DF mDxEdge01s[4];
	DF mDxEdge12s[4];
	DF mDxEdge20s[4];
	// dyEdgeFunctionValue, 2 * dyEdgeFunctionValue, 4 * dyEdgeFunctionValue, 8 * dyEdgeFunctionValue
	DF mDyEdge01s[4];
	DF mDyEdge12s[4];
	DF mDyEdge20s[4];

	DF mTriSizeMul2;

	// bbox covers triangle
	FP mMinX = FP::ZERO;
	FP mMaxX = FP::ZERO;
	FP mMinY = FP::ZERO;
	FP mMaxY = FP::ZERO;



};