#include "RasterizeFixed.h"

// todo : save is left, top line each triangle
// todo : left, top 변수 미리 계산해서 변수로 빼놓기
void RasterizeFixed::Rasterize(List* pixels, const List* fixedVertices, const List* indices)
{
	const static FP halfOne = FP(0.5f);
	const static FP one = FP(1.0f);
	const static FP four = FP(4.0f);
	const static FP eight = FP(8.0f);
	for (uint32_t indexIdx = 0; indexIdx < indices->GetSize(); indexIdx += 3) {
		const FixedVertex& v0 = fixedVertices->At<FixedVertex>(indices->At<uint32_t>(indexIdx));
		const FixedVertex& v1 = fixedVertices->At<FixedVertex>(indices->At<uint32_t>(indexIdx + 1));
		const FixedVertex& v2 = fixedVertices->At<FixedVertex>(indices->At<uint32_t>(indexIdx + 2));

		// calculate bbox of pixel covers triangle
		mMinX = min(v0.pos.x, min(v1.pos.x, v2.pos.x));
		mMaxX = max(v0.pos.x, max(v1.pos.x, v2.pos.x));
		mMinY = min(v0.pos.y, min(v1.pos.y, v2.pos.y));
		mMaxY = max(v0.pos.y, max(v1.pos.y, v2.pos.y));

		// conversion of common number from floating number to fixed number
		FP minXFloor = mMinX.Floor();
		FP minYFloor = mMinY.Floor();
		FP startX = mMinX - minXFloor > halfOne ? minXFloor + one + halfOne : minXFloor + halfOne;
		FP startY = mMinY - minYFloor > halfOne ? minYFloor + one + halfOne : minYFloor + halfOne;
		FixedVec4 startPos(startX, startY, 0, 0);

		// pre calculate 2 * triangle size
		mTriSizeMul2 = EdgeFunction(v0.pos, v1.pos, v2.pos);
		//		because of precision of fixed point, 
		//		difference positions of vertices before conversion of fixed point can be same after it.
		if (mTriSizeMul2 == 0) {
			continue;
		}

		// fixed number & partition rasterization
#if RASTERIZATION_TYPE == PARTITION_RASTERIZATION
		// pre calculate difference of edge value by 2^i x, 2^i y				
		mDxEdge01s[0] = (v1.pos.y - v0.pos.y).ToDoubleFixedPoint();
		mDxEdge12s[0] = (v2.pos.y - v1.pos.y).ToDoubleFixedPoint();
		mDxEdge20s[0] = (v0.pos.y - v2.pos.y).ToDoubleFixedPoint();
		mDyEdge01s[0] = (v0.pos.x - v1.pos.x).ToDoubleFixedPoint();
		mDyEdge12s[0] = (v1.pos.x - v2.pos.x).ToDoubleFixedPoint();
		mDyEdge20s[0] = (v2.pos.x - v0.pos.x).ToDoubleFixedPoint();

		DF startEdge01 = EdgeFunction(startPos, v0.pos, v1.pos);
		DF startEdge12 = EdgeFunction(startPos, v1.pos, v2.pos);
		DF startEdge20 = EdgeFunction(startPos, v2.pos, v0.pos);

#pragma UNROLL(3)
		for (int i = 1; i <= 3; i++) {
			mDxEdge01s[i] = mDxEdge01s[0] * (1 << i); // (2^i)			
			mDxEdge12s[i] = mDxEdge12s[0] * (1 << i);
			mDxEdge20s[i] = mDxEdge20s[0] * (1 << i);


			mDyEdge01s[i] = mDyEdge01s[0] * (1 << i);
			mDyEdge12s[i] = mDyEdge12s[0] * (1 << i);
			mDyEdge20s[i] = mDyEdge20s[0] * (1 << i);
		}

		// rasterize bunch of pixels starts with 8*8 pixels
		DF yEdge01 = startEdge01;
		DF yEdge12 = startEdge12;
		DF yEdge20 = startEdge20;
		for (FP y = startY; y <= mMaxY; y += eight) {

			DF xEdge01 = yEdge01;
			DF xEdge12 = yEdge12;
			DF xEdge20 = yEdge20;
			for (FP x = startX; x <= mMaxX; x += eight) {
				RasterizePart(pixels, v0.pos, v1.pos, v2.pos, x, y, 3, xEdge01, xEdge12, xEdge20);

				xEdge01 += mDxEdge01s[3];
				xEdge12 += mDxEdge12s[3];
				xEdge20 += mDxEdge20s[3];
			}

			yEdge01 += mDyEdge01s[3];
			yEdge12 += mDyEdge12s[3];
			yEdge20 += mDyEdge20s[3];

		}
#elif RASTERIZATION_TYPE == ADVANCED_RASTERIZATION
		mDxEdge01s[0] = (v1.pos.y - v0.pos.y).ToDoubleFixedPoint();
		mDxEdge12s[0] = (v2.pos.y - v1.pos.y).ToDoubleFixedPoint();
		mDxEdge20s[0] = (v0.pos.y - v2.pos.y).ToDoubleFixedPoint();
		mDyEdge01s[0] = (v0.pos.x - v1.pos.x).ToDoubleFixedPoint();
		mDyEdge12s[0] = (v1.pos.x - v2.pos.x).ToDoubleFixedPoint();
		mDyEdge20s[0] = (v2.pos.x - v0.pos.x).ToDoubleFixedPoint();

		DF yEdge01 = EdgeFunctionFixedPoint(startPos, v0.pos, v1.pos);
		DF yEdge12 = EdgeFunctionFixedPoint(startPos, v1.pos, v2.pos);
		DF yEdge20 = EdgeFunctionFixedPoint(startPos, v2.pos, v0.pos);
		for (FP y = startY;
			y <= mMaxY;
			y += one,
			yEdge01 += mDyEdge01s[0],
			yEdge12 += mDyEdge12s[0],
			yEdge20 += mDyEdge20s[0])
		{
			DF xEdge01 = yEdge01;
			DF xEdge12 = yEdge12;
			DF xEdge20 = yEdge20;

			for (FP x = startX;
				x <= mMaxX;
				x += one,
				xEdge01 += mDxEdge01s[0],
				xEdge12 += mDxEdge12s[0],
				xEdge20 += mDxEdge20s[0])
			{
				FixedVec4 pixelPos(x, y, FP::ZERO, FP::ZERO);
				DF edge01 = EdgeFunctionFixedPoint(pixelPos, v0.pos, v1.pos);
				DF edge12 = EdgeFunctionFixedPoint(pixelPos, v1.pos, v2.pos);
				DF edge20 = EdgeFunctionFixedPoint(pixelPos, v2.pos, v0.pos);

				//TODO : 조건 if 최적화
				if (edge01 > DF::ZERO
					|| edge12 > DF::ZERO
					|| edge20 > DF::ZERO) {
					continue;
				}

				if (edge01 == DF::ZERO
					&& !IsLeftLineFixedPoint(v0.pos, v1.pos)
					&& !IsTopLineFixedPoint(v0.pos, v1.pos)) {
					continue;
				}

				if (edge12 == DF::ZERO
					&& !IsLeftLineFixedPoint(v1.pos, v2.pos)
					&& !IsTopLineFixedPoint(v1.pos, v2.pos)) {
					continue;
				}

				if (edge20 == DF::ZERO
					&& !IsLeftLineFixedPoint(v2.pos, v0.pos)
					&& !IsTopLineFixedPoint(v2.pos, v0.pos)) {
					continue;
				}

				DF bary01 = edge01 / mTriSizeMul2;
				DF bary12 = edge12 / mTriSizeMul2;
				DF bary20 = edge20 / mTriSizeMul2;

				Vec4 pos = Vec4::ZERO;
				pos.x = x.ToFloat();
				pos.y = y.ToFloat();
				pos.w = bary12.ToDouble() / v0.pos.w.ToFloat()
					+ bary20.ToDouble() / v1.pos.w.ToFloat()
					+ bary01.ToDouble() / v2.pos.w.ToFloat();
				pos.z = 1.0f / pos.w;

				Pixel pixel;
				pixel.pos = pos;
				mPixels->Add(pixel);
			}

		}

		// fixed number & normal rasterization
#else
		for (FP y = startY; y <= mMaxY; y += one) {
			for (FP x = startX; x <= mMaxX; x += one) {
				FixedVec4 pixelPos(x, y, FP::ZERO, FP::ZERO);
				DF edge01 = EdgeFunction(pixelPos, v0.pos, v1.pos);
				DF edge12 = EdgeFunction(pixelPos, v1.pos, v2.pos);
				DF edge20 = EdgeFunction(pixelPos, v2.pos, v0.pos);

				//TODO : 조건 if 최적화
				if (edge01 > DF::ZERO
					|| edge12 > DF::ZERO
					|| edge20 > DF::ZERO) {
					continue;
				}

				if (edge01 == DF::ZERO
					&& !IsLeftLine(v0.pos, v1.pos)
					&& !IsTopLine(v0.pos, v1.pos)) {
					continue;
				}

				if (edge12 == DF::ZERO
					&& !IsLeftLine(v1.pos, v2.pos)
					&& !IsTopLine(v1.pos, v2.pos)) {
					continue;
				}

				if (edge20 == DF::ZERO
					&& !IsLeftLine(v2.pos, v0.pos)
					&& !IsTopLine(v2.pos, v0.pos)) {
					continue;
				}

				DF bary01 = edge01 / mTriSizeMul2;
				DF bary12 = edge12 / mTriSizeMul2;
				DF bary20 = edge20 / mTriSizeMul2;

				Vec4 pos = Vec4::ZERO;
				pos.x = x.ToFloat();
				pos.y = y.ToFloat();
				pos.w = bary12.ToDouble() / v0.pos.w.ToFloat()
					+ bary20.ToDouble() / v1.pos.w.ToFloat()
					+ bary01.ToDouble() / v2.pos.w.ToFloat();
				pos.z = 1.0f / pos.w;

				Pixel pixel;
				pixel.pos = pos;
				pixels->Add(pixel);
			}
		}
#endif
	}
}

void RasterizeFixed::RasterizePart(
	List* pixels,
	const FixedVec4& v0Pos,
	const FixedVec4& v1Pos,
	const FixedVec4& v2Pos,
	FP leftX,
	FP topY,
	uint8_t pixelLengthLog2,
	DF edge01,
	DF edge12,
	DF edge20)
{
	// pixel length = 1
	if (pixelLengthLog2 == 0) {
		AddPixelIsInTriangle(pixels, v0Pos, v1Pos, v2Pos, leftX, topY, edge01, edge12, edge20);
		return;
	}
	// pixel length 2
	else if (pixelLengthLog2 == 1) {
		AddPixelIsInTriangle(pixels, v0Pos, v1Pos, v2Pos, leftX, topY,
			edge01, edge12, edge20);
		AddPixelIsInTriangle(pixels, v0Pos, v1Pos, v2Pos, leftX + 1, topY,
			edge01 + mDxEdge01s[0],
			edge12 + mDxEdge12s[0],
			edge20 + mDxEdge20s[0]);
		AddPixelIsInTriangle(pixels, v0Pos, v1Pos, v2Pos, leftX, topY + 1,
			edge01 + mDyEdge01s[0],
			edge12 + mDyEdge12s[0],
			edge20 + mDyEdge20s[0]);
		AddPixelIsInTriangle(pixels, v0Pos, v1Pos, v2Pos, leftX + 1, topY + 1,
			edge01 + mDxEdge01s[0] + mDyEdge01s[0],
			edge12 + mDxEdge12s[0] + mDyEdge12s[0],
			edge20 + mDxEdge20s[0] + mDyEdge20s[0]);

		return;
	}

	// TODO : test advancing bary method
	//			remove
	//DF a = EdgeFunction(FixedVec4(leftX, topY, 0, 1), v0Pos, v1Pos);
	//DF b = EdgeFunction(FixedVec4(leftX, topY, 0, 1), v1Pos, v2Pos);
	//DF c = EdgeFunction(FixedVec4(leftX, topY, 0, 1), v2Pos, v0Pos);
	//DF s = EdgeFunction(v0Pos, v1Pos, v2Pos);
	//if (abs((a / s).ToDouble() - bary01.ToFloat()) > 0.01f
	//	|| abs((b / s).ToDouble() - bary12.ToFloat()) > 0.01f
	//	|| abs((c / s).ToDouble() - bary20.ToFloat()) > 0.01f) {
	//	SetConsoleActiveScreenBuffer(GetStdHandle(STD_OUTPUT_HANDLE));
	//	std::cout << "dif 01 : " << (a / s).ToDouble() << " " << bary01.ToFloat() << std::endl;
	//	std::cout << "dif 12 : " << (b / s).ToDouble() << " " << bary12.ToFloat() << std::endl;
	//	std::cout << "dif 20 : " << (c / s).ToDouble() << " " << bary20.ToFloat() << std::endl;
	//	__debugbreak();
	//}


	// four corners don't inside triangle, divide pixel sqaure finely
	DF rightTopEdge01 = edge01 + (mDxEdge01s[pixelLengthLog2] - mDxEdge01s[0]);
	DF rightTopEdge12 = edge12 + (mDxEdge12s[pixelLengthLog2] - mDxEdge12s[0]);
	DF rightTopEdge20 = edge20 + (mDxEdge20s[pixelLengthLog2] - mDxEdge20s[0]);

	DF leftBottomEdge01 = edge01 + (mDyEdge01s[pixelLengthLog2] - mDyEdge01s[0]);
	DF leftBottomEdge12 = edge12 + (mDyEdge12s[pixelLengthLog2] - mDyEdge12s[0]);
	DF leftBottomEdge20 = edge20 + (mDyEdge20s[pixelLengthLog2] - mDyEdge20s[0]);

	DF rightBottomEdge01 = edge01 + (mDxEdge01s[pixelLengthLog2] - mDxEdge01s[0]) + (mDyEdge01s[pixelLengthLog2] - mDyEdge01s[0]);
	DF rightBottomEdge12 = edge12 + (mDxEdge12s[pixelLengthLog2] - mDxEdge12s[0]) + (mDyEdge12s[pixelLengthLog2] - mDyEdge12s[0]);
	DF rightBottomEdge20 = edge20 + (mDxEdge20s[pixelLengthLog2] - mDxEdge20s[0]) + (mDyEdge20s[pixelLengthLog2] - mDyEdge20s[0]);

	FP rightX = leftX + (1 << pixelLengthLog2) - 1;
	FP bottomY = topY + (1 << pixelLengthLog2) - 1;

	// TODO : think what is faster btw simd comp, contiguous || in if
	bool isOutBBox = leftX < mMinX
		|| rightX > mMaxX
		|| topY < mMinY
		|| bottomY > mMaxY;

	// for not thinking top-left rule in 4*4, 8*8 pixel blocks in triangle,
	// select only pixel blocks in triangle and not on triangle sides.
	bool isOutOrOnTriangle = edge01 >= DF::ZERO
		|| edge12 >= DF::ZERO
		|| edge20 >= DF::ZERO
		|| rightTopEdge01 >= DF::ZERO
		|| rightTopEdge12 >= DF::ZERO
		|| rightTopEdge20 >= DF::ZERO
		|| leftBottomEdge01 >= DF::ZERO
		|| leftBottomEdge12 >= DF::ZERO
		|| leftBottomEdge20 >= DF::ZERO
		|| rightBottomEdge01 >= DF::ZERO
		|| rightBottomEdge12 >= DF::ZERO
		|| rightBottomEdge20 >= DF::ZERO;

	if (isOutBBox || isOutOrOnTriangle) {
		uint8_t finePixelLengthLog2 = pixelLengthLog2 - 1;
		FP finePixelLength(1 << finePixelLengthLog2);

		RasterizePart(pixels, v0Pos, v1Pos, v2Pos, leftX, topY, finePixelLengthLog2,
			edge01, edge12, edge20);
		RasterizePart(pixels, v0Pos, v1Pos, v2Pos, leftX + finePixelLength, topY, finePixelLengthLog2,
			edge01 + mDxEdge01s[finePixelLengthLog2],
			edge12 + mDxEdge12s[finePixelLengthLog2],
			edge20 + mDxEdge20s[finePixelLengthLog2]);
		RasterizePart(pixels, v0Pos, v1Pos, v2Pos, leftX, topY + finePixelLength, finePixelLengthLog2,
			edge01 + mDyEdge01s[finePixelLengthLog2],
			edge12 + mDyEdge12s[finePixelLengthLog2],
			edge20 + mDyEdge20s[finePixelLengthLog2]);
		RasterizePart(pixels, v0Pos, v1Pos, v2Pos, leftX + finePixelLength, topY + finePixelLength, finePixelLengthLog2,
			edge01 + mDxEdge01s[finePixelLengthLog2] + mDyEdge01s[finePixelLengthLog2],
			edge12 + mDxEdge12s[finePixelLengthLog2] + mDyEdge12s[finePixelLengthLog2],
			edge20 + mDxEdge20s[finePixelLengthLog2] + mDyEdge20s[finePixelLengthLog2]);
		return;
	}

	// add all pixels in four corners	
	FP y = topY;
	FP x = leftX;
	DF yEdge01 = edge01;
	DF yEdge12 = edge12;
	DF yEdge20 = edge20;
	uint8_t pixelLength = 1 << pixelLengthLog2;
	for (uint8_t yIdx = 0; yIdx < pixelLength; yIdx++) {

		DF xEdge01 = yEdge01;
		DF xEdge12 = yEdge12;
		DF xEdge20 = yEdge20;
		for (uint8_t xIdx = 0; xIdx < pixelLength; xIdx++) {
			// todo : create advanced partition rasterization in two methods
			// 1. advanced edge differenece only pixel blocks in triangle and not on sides of triangle
			// 2. add brances when pixel blocks include pixels on right or bottom sides of triangle

			Pixel pixel;
			pixel.pos = InterpolatePos(x,
				y,
				v0Pos,
				v1Pos,
				v2Pos,
				xEdge01,
				xEdge12,
				xEdge20);
			pixels->Add(pixel);


			xEdge01 += mDxEdge01s[0];
			xEdge12 += mDxEdge12s[0];
			xEdge20 += mDxEdge20s[0];
			x += FP::ONE;
		}

		yEdge01 += mDyEdge01s[0];
		yEdge12 += mDyEdge12s[0];
		yEdge20 += mDyEdge20s[0];

		y += FP::ONE;
		x = leftX;
	}

}

inline void RasterizeFixed::AddPixelIsInTriangle(List* pixels, const FixedVec4& v0Pos, const FixedVec4& v1Pos, const FixedVec4& v2Pos, FP x, FP y, DF edge01, DF edge12, DF edge20)
{
	// out viewport
	bool isOutBBox = x < mMinX
		|| x >= mMaxX
		|| y < mMinY
		|| y >= mMaxY;
	if (isOutBBox) {
		return;
	}

	// not in triangle
	bool isSelect = IsSelectPixel(v0Pos, v1Pos, v2Pos, x, y, edge01, edge12, edge20);
	if (isSelect == false) {
		return;
	}


	Pixel pixel;
	pixel.pos = InterpolatePos(x,
		y,
		v0Pos,
		v1Pos,
		v2Pos,
		edge01,
		edge12,
		edge20);


	pixels->Add(pixel);
}

inline bool RasterizeFixed::IsSelectPixel(const FixedVec4& v0Pos, const FixedVec4& v1Pos, const FixedVec4& v2Pos, FP x, FP y, DF edge01, DF edge12, DF edge20)
{
	//TODO : 조건 if 최적화
	if (edge01 > DF::ZERO
		|| edge12 > DF::ZERO
		|| edge20 > DF::ZERO) {
		return false;
	}


	if (edge01 == DF::ZERO
		&& !IsLeftLine(v0Pos, v1Pos)
		&& !IsTopLine(v0Pos, v1Pos)) {
		return false;
	}

	if (edge12 == DF::ZERO
		&& !IsLeftLine(v1Pos, v2Pos)
		&& !IsTopLine(v1Pos, v2Pos)) {
		return false;
	}

	if (edge20 == DF::ZERO
		&& !IsLeftLine(v2Pos, v0Pos)
		&& !IsTopLine(v2Pos, v0Pos)) {
		return false;
	}

	return true;
}



inline bool RasterizeFixed::IsLeftLine(const FixedVec4& prePos, const FixedVec4& nextPos) const
{
	return (nextPos - prePos).y < 0;
}

inline bool RasterizeFixed::IsTopLine(const FixedVec4& prePos, const FixedVec4& nextPos) const
{
	return ((nextPos - prePos).y == 0) && ((nextPos - prePos).x > 0);
}


inline Vec4 RasterizeFixed::InterpolatePos(
	FP x,
	FP y,
	const FixedVec4& v0Pos,
	const FixedVec4& v1Pos,
	const FixedVec4& v2Pos,
	DF edge01,
	DF edge12,
	DF edge20) const
{
	DF bary01 = edge01 / mTriSizeMul2;
	DF bary12 = edge12 / mTriSizeMul2;
	DF bary20 = edge20 / mTriSizeMul2;


	Vec4 pos = Vec4::ZERO;
	pos.x = x.ToFloat();
	pos.y = y.ToFloat();

	pos.w = static_cast<float>(edge12.ToDouble()) / v0Pos.w.ToFloat()
		+ static_cast<float>(edge20.ToDouble()) / v1Pos.w.ToFloat()
		+ static_cast<float>(edge01.ToDouble()) / v2Pos.w.ToFloat();

	pos.z = 1.0f / pos.w;

	return pos;
}
