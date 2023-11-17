#pragma once
#include "IRasterizable.h"
#include "Primitive.h"

class RasterizeFixed : public IRasterizable {
public:
	virtual void Rasterize(List* pixels, const List* fixedVertices, const List* indices) override;
	virtual ~RasterizeFixed() override {}

private:
	inline DF EdgeFunction(const FixedVec4& pixelPos, const FixedVec4& v0Pos, const FixedVec4& v1Pos) const
	{
		FixedVec4 pMinusV0 = pixelPos - v0Pos;
		FixedVec4 v1MinusV0 = v1Pos - v0Pos;

		DF v0PX = pMinusV0.x.ToDoubleFixedPoint();
		DF v0V1Y = v1MinusV0.y.ToDoubleFixedPoint();
		DF v0PY = pMinusV0.y.ToDoubleFixedPoint();
		DF v0V1X = v1MinusV0.x.ToDoubleFixedPoint();

		return v0PX * v0V1Y - v0PY * v0V1X;
	}

	void RasterizePart(
		List* pixels,
		const FixedVec4& v0Pos,
		const FixedVec4& v1Pos,
		const FixedVec4& v2Pos,
		FP leftX,
		FP topY,
		uint8_t pixelLengthLog2,
		DF edge01,
		DF edge12,
		DF edge20);

	inline void AddPixelIsInTriangle(
		List* pixels,
		const FixedVec4& v0Pos,
		const FixedVec4& v1Pos,
		const FixedVec4& v2Pos,
		FP x,
		FP y,
		DF edge01,
		DF edge12,
		DF edge20);

	inline bool IsSelectPixel(
		const FixedVec4& v0Pos,
		const FixedVec4& v1Pos,
		const FixedVec4& v2Pos,
		FP x,
		FP y,
		DF edge01,
		DF edge12,
		DF edge20
	);

	inline bool IsLeftLine(const FixedVec4& prePos, const FixedVec4& nextPos) const;
	inline bool IsTopLine(const FixedVec4& prePos, const FixedVec4& nextPos) const;


	inline Vec4 InterpolatePos(
		FP x,
		FP y,
		const FixedVec4& v0Pos,
		const FixedVec4& v1Pos,
		const FixedVec4& v2Pos,
		DF edge01,
		DF edge12,
		DF edge20) const;

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