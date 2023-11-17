#include "SWRasterizer.h"
#include "Math.h"
#include <memory>
#include <cassert>
#include <windows.h>

SWRasterizer::SWRasterizer() 
{

}

void SWRasterizer::Initialize()
{
	mVerticesPool[0] = new List(1, RESERVED_VERTICES_BYTES);
	mVerticesPool[1] = new List(1, RESERVED_VERTICES_BYTES);
	mIndicesPool[0] = new List(1, RESERVED_INDICES_BYTES);
	mIndicesPool[1] = new List(1, RESERVED_INDICES_BYTES);
	mPixels = new List(1, RESERVED_PIXELS_BYTES);

#ifdef FIXED_RASTERIZATION
	mRasterize = new RasterizeFixed;
#else
	mRasterize = new RasterizeFloating;
#endif
}

void SWRasterizer::Terminate()
{
	if (mVerticesPool[0]) {
		delete mVerticesPool[0];
		mVerticesPool[0] = nullptr;
	}

	if (mVerticesPool[1]) {
		delete mVerticesPool[1];
		mVerticesPool[1] = nullptr;
	}

	if (mIndicesPool[0]) {
		delete mIndicesPool[0];
		mIndicesPool[0] = nullptr;
	}

	if (mIndicesPool[1]) {
		delete mIndicesPool[1];
		mIndicesPool[1] = nullptr;
	}

	if (mPixels) {
		delete mPixels;
		mPixels = nullptr;
	}

	if (mRasterize) {
		delete mRasterize;
		mRasterize = nullptr;
	}
}

void SWRasterizer::Execute(const Vertex* vertices, int vertexNum, const uint32_t* indices, int indexNum)
{
	// clear vertex, index pool
	for (int i = 0; i < sizeof(mVerticesPool) / sizeof(mVerticesPool[0]); i++) {
		mVerticesPool[i]->Reset(sizeof(Vertex));
	}

	for (int i = 0; i < sizeof(mIndicesPool) / sizeof(mIndicesPool[0]); i++) {
		mIndicesPool[i]->Reset(sizeof(uint32_t));
	}

	// clear pixel list
	mPixels->Reset(sizeof(Pixel));

	
	mVertexNum = vertexNum;
	mIndexNum = indexNum;

#ifdef DEBUG_PROCESS_COORDINATE
	SetConsoleActiveScreenBuffer(GetStdHandle(STD_OUTPUT_HANDLE));
	std::cout << "vertex" << std::endl;
	for (int i = 0; i < vertexNum; i++) {
		std::cout << i << " : " << vertices[i].pos << std::endl;
	}

	std::cout << "index" << std::endl;
	for (int i = 0; i < indexNum; i++) {
		std::cout << i << " : " << indices[i] << std::endl;
	}
#endif

	// clip
	List* preClippedVertices = mVerticesPool[0];
	List* preClippedIndices = mIndicesPool[0];
	List* clippedVertices = mVerticesPool[1];
	List* clippedIndices = mIndicesPool[1];
	preClippedVertices->Add<Vertex>(vertices, vertexNum);
	preClippedIndices->Add<uint32_t>(indices, indexNum);
	Clip(&clippedVertices, &clippedIndices, preClippedVertices, preClippedIndices);

#ifdef DEBUG_PROCESS_COORDINATE
	std::cout << "clip vertex" << std::endl;
	for (int i = 0; i < clippedVertices->GetSize(); i++) {
		std::cout << i << " : " << (clippedVertices->At<Vertex>(i)).pos << std::endl;
	}

	std::cout << "clip index" << std::endl;
	for (int i = 0; i < clippedIndices->GetSize(); i++) {
		std::cout << i << " : " << clippedIndices->At<uint32_t>(i) << std::endl;
	}
#endif

	// perspective division
	DividePerspective(&clippedVertices);

#ifdef DEBUG_PROCESS_COORDINATE
	std::cout << "perspective deivision" << std::endl;
	for (int i = 0; i < clippedVertices->GetSize(); i++) {
		std::cout << i << " : " << (clippedVertices->At<Vertex>(i)).pos << std::endl;
	}
#endif

	// back face culling
	List* culledIndices = preClippedIndices;
	culledIndices->Reset(sizeof(uint32_t));
	CullBackFace(&culledIndices, clippedIndices, clippedVertices);

#ifdef DEBUG_PROCESS_COORDINATE
	std::cout << "cull back face" << std::endl;
	for (int i = 0; i < culledIndices->GetSize(); i++) {
		std::cout << i << " : " << culledIndices->At<uint32_t>(i) << std::endl;
	}
#endif

	// TODO : after back face culling, only do remain process with vertices not culled.

	// transform viewport
	TransformViewport(&clippedVertices);

#ifdef DEBUG_PROCESS_COORDINATE
	std::cout << "transform viewport" << std::endl;
	for (int i = 0; i < clippedVertices->GetSize(); i++) {
		std::cout << i << " : " << (clippedVertices->At<Vertex>(i)).pos << std::endl;
	}
#endif

	// convert to fixed point
#ifdef FIXED_RASTERIZATION	
	List* fixedVertices = preClippedVertices;
	fixedVertices->Reset(sizeof(FixedVertex));
	ConvertFixedPoint(&fixedVertices, clippedVertices);

#ifdef DEBUG_PROCESS_COORDINATE
	std::cout << "fixed point" << std::endl;
	for (int i = 0; i < fixedVertices->GetSize(); i++) {
		std::cout << i << " : " << (fixedVertices->At<FixedVertex>(i)).pos << std::endl;
	}
#endif

	mRasterize->Rasterize(mPixels, fixedVertices, culledIndices);
#else
	mRasterize->Rasterize(mPixels, clippedVertices, culledIndices);
#endif

	
	

	//RasterizeFixedPoint(fixedVertices, culledIndices);

#ifdef DEBUG_PROCESS_COORDINATE
	std::cout << "pixel" << std::endl;
	for (int i = 0; i < mPixels->GetSize(); i++) {
		std::cout << i << " : " << mPixels->At<Pixel>(i).pos << std::endl;
	}

	std::cout << "end rasterization" << std::endl;
#endif
}


void SWRasterizer::SetupViewport(const Viewport& viewport)
{
	mViewport = viewport;
}

void SWRasterizer::Clip(List** pClippedVertices, List** pClippedIndices, const List* vertices, const List* indices)
{
	Vertex clippedVertices[9];
	uint32_t clippedIndices[21];

	int indexLen = indices->GetSize();
	for (int indexIdx = 0; indexIdx < indexLen; indexIdx+=3) {		
		Vertex v1 = vertices->At<Vertex>(indices->At<uint32_t>(indexIdx));
		Vertex v2 = vertices->At<Vertex>(indices->At<uint32_t>(indexIdx + 1));
		Vertex v3 = vertices->At<Vertex>(indices->At<uint32_t>(indexIdx + 2));
		Triangle tri = { v1, v2, v3};

		uint8_t clippedVertexNum = 0;
		uint8_t clippedIndexNum = 0;
		bool isClipped = false;
		ClipTriangle(&isClipped, &clippedIndexNum, &clippedVertexNum, clippedVertices, clippedIndices, tri);		

		for (int i = 0; i < clippedIndexNum; i++) {
			(*pClippedIndices)->Add<uint32_t>((*pClippedVertices)->GetSize() + clippedIndices[i]);
		}

		for (int i = 0; i < clippedVertexNum; i++) {
			(*pClippedVertices)->Add<Vertex>(clippedVertices[i]);
		}		
	}
}

void SWRasterizer::ClipTriangle(bool* pIsClipped, uint8_t* pClippedIndexNum, uint8_t* pClippedVertexNum, Vertex(&clippedVertices)[9], uint32_t(&clippedIndices)[21], const Triangle& triangle)
{	
	*pIsClipped = false;

	int vertexNum = 3;
	// clip vertex buffer size : triangle vertex 3 + can be added vertex 6
	//		During one clip on one plane, can be added max 1 vertex
	// TODO : move to member
	Vertex buffer1[9];	
	Vertex buffer2[9];
	Vertex* unClippedVertices = buffer1;
	Vertex* partClippedVertices = buffer2;	
	for (int i = 0; i < 3; i++) {
		unClippedVertices[i] = triangle.vertices[i];
	}


	// clipping
	int planeLen = static_cast<int>(PlaneID::Length);
	for (int planeID = 0; planeID < planeLen; planeID++) {		
		
		int partClippedVertexIndex = 0;
		const Vertex* pPrevV = &unClippedVertices[0];
		bool isPrevVInPlane = IsInPlane(static_cast<PlaneID>(planeID), *pPrevV);
		for (int vertexIdx = 1; vertexIdx < vertexNum + 1; vertexIdx++) {			
			const Vertex& v = unClippedVertices[vertexIdx % vertexNum];						
			bool isVInPlane = IsInPlane(static_cast<PlaneID>(planeID), v);

			// all in
			if (isPrevVInPlane && isVInPlane) {
				partClippedVertices[partClippedVertexIndex++] = v;
			}
			// v1 in
			else if (isPrevVInPlane) {		
				*pIsClipped = true;
				partClippedVertices[partClippedVertexIndex++] = CalculateInterVertex(static_cast<PlaneID>(planeID), *pPrevV, v);;
			}
			// v2 in
			else if (isVInPlane) {
				*pIsClipped = true;
				partClippedVertices[partClippedVertexIndex++] = CalculateInterVertex(static_cast<PlaneID>(planeID), *pPrevV, v);
				partClippedVertices[partClippedVertexIndex++] = v;
			}
			else {
				*pIsClipped = true;
			}

			// swap 			
			pPrevV = &v;
			isPrevVInPlane = isVInPlane;
		}

		// swap buffer & vertex num
		vertexNum = partClippedVertexIndex;
		if (vertexNum == 0) {
			break;			
		}

		if (planeID != planeLen - 1) {
			auto* temp = unClippedVertices;
			unClippedVertices = partClippedVertices;
			partClippedVertices = temp;
		}			
	}

	// return
	// if triangle doesn't be created
	if (vertexNum - 2 < 0) {
		*pClippedVertexNum = 0;
		*pClippedIndexNum = 0;
		return;
	}

	// set vertex
	memcpy(clippedVertices, partClippedVertices, sizeof(Vertex) * vertexNum);
	// set indices
	for (int i = 0; i < (vertexNum - 2); i++) {
		clippedIndices[i * 3] = 0;
		clippedIndices[i * 3 + 1] = (i + 1) % vertexNum;
		clippedIndices[i * 3 + 2] = (i + 2) % vertexNum;
	}
	// set index & vertex num
	*pClippedVertexNum = vertexNum;
	*pClippedIndexNum = (vertexNum - 2) * 3;
}

inline bool SWRasterizer::IsInPlane(PlaneID planeID, const Vertex& clipVertex)
{
	switch (planeID) {
		// -x
		case PlaneID::NegX:
			return clipVertex.pos.x >= -clipVertex.pos.w;
			// +x
		case PlaneID::PosX:
			return clipVertex.pos.x <= clipVertex.pos.w;
			// -y
		case PlaneID::NegY:
			return clipVertex.pos.y >= -clipVertex.pos.w;
			// +y
		case PlaneID::PosY:
			return clipVertex.pos.y <= clipVertex.pos.w;
			// -z
		case PlaneID::NegZ:
			return clipVertex.pos.z >= 0;
			// +z
		case PlaneID::PosZ:
			return clipVertex.pos.z <= clipVertex.pos.w;
		default:
			assert(false);
	}

	return false;
}

inline float SWRasterizer::GetSignedDstWithPlane(PlaneID planeID, const Vertex& clipVertex)
{
	switch (planeID) {
		// -x
		case PlaneID::NegX:
			return clipVertex.pos.x + clipVertex.pos.w;
			// +x
		case PlaneID::PosX:
			return clipVertex.pos.x - clipVertex.pos.w;
			// -y
		case PlaneID::NegY:
			return clipVertex.pos.y + clipVertex.pos.w;
			// +y
		case PlaneID::PosY:
			return clipVertex.pos.y - clipVertex.pos.w;
			// -z
		case PlaneID::NegZ:
			return clipVertex.pos.z;
			// +z
		case PlaneID::PosZ:
			return clipVertex.pos.z - clipVertex.pos.w;
		default:
			assert(false);
	}

	return 0.0f;
}

inline Vertex SWRasterizer::CalculateInterVertex(PlaneID planeID, const Vertex& inV, const Vertex& outV)
{
	float v0Dst = GetSignedDstWithPlane(planeID, inV);
	float v1Dst = GetSignedDstWithPlane(planeID, outV);
	float t = v0Dst / (v0Dst - v1Dst);
	// TODO : add attribute interpolation
	Vertex interV;
	interV.pos = Vec4::Lerp(inV.pos, outV.pos, t);

	return interV;	
}

void SWRasterizer::DividePerspective(List** pVertices)
{
	for (int i = 0; i < (*pVertices)->GetSize(); i++) {
		float w = (*pVertices)->At<Vertex>(i).pos.w;
		if ((*reinterpret_cast<int32_t*>(&w) & 0) == 0) {
			// TODO : add min fraction value that be kept when convert fixed point
			// TODO : thinking how impact error cuz of adding this constant.
			//			and interpolation if able
			w += HOMOGENEOUS_VERTEX_MIN_Z;
		}

		Vec4 dividedPos = (*pVertices)->At<Vertex>(i).pos;
		dividedPos.x /= w;
		dividedPos.y /= w;
		dividedPos.z /= w;

		(*pVertices)->At<Vertex>(i).pos = dividedPos;
	}
}

void SWRasterizer::CullBackFace(List** pCulledIndices, const List* indices, const List* vertices)
{
	for (int i = 0; i < indices->GetSize(); i += 3) {
		Vertex v1 = vertices->At<Vertex>(indices->At<uint32_t>(i));
		Vertex v2 = vertices->At<Vertex>(indices->At<uint32_t>(i + 1));
		Vertex v3 = vertices->At<Vertex>(indices->At<uint32_t>(i + 2));

		Vec4 dif1 = v2.pos - v1.pos;
		Vec4 dif2 = v3.pos - v1.pos;

		// z of (dif1.xy, 0) x (dif2.xy, 0)
		float d = dif1.x* dif2.y - dif2.x * dif1.y;
		// when cw, d is less than 0
		if (d >= 0) {
			continue;
		}

		(*pCulledIndices)->Add<uint32_t>(indices->At<uint32_t>(i));
		(*pCulledIndices)->Add<uint32_t>(indices->At<uint32_t>(i+1));
		(*pCulledIndices)->Add<uint32_t>(indices->At<uint32_t>(i+2));
	}
}

void SWRasterizer::TransformViewport(List** pVertices)
{
	// ndc to viewport	
	//		viewport : +x : right, +y : down. origin is left top 
	for (uint32_t i = 0; i < (*pVertices)->GetSize(); i++) {
		Vec4 pos = (*pVertices)->At<Vertex>(i).pos;
		pos.x = (pos.x + 1) * 0.5f * mViewport.width + mViewport.leftX;
		pos.y = (1 - pos.y) * 0.5f * mViewport.height + mViewport.topY;
		pos.z = pos.z * (mViewport.maxZ - mViewport.minZ) + mViewport.minZ;

		(*pVertices)->At<Vertex>(i).pos = pos;
	}	
}

void SWRasterizer::ConvertFixedPoint(List** pOutVertices, const List* vertices)
{
	for (uint32_t i = 0; i < vertices->GetSize(); i++) {
		(*pOutVertices)->Add(FixedVertex(vertices->At<Vertex>(i).pos));
	}
}

// todo : save is left, top line each triangle
// todo : left, top 변수 미리 계산해서 변수로 빼놓기
void SWRasterizer::RasterizeFixedPoint(const List* fixedVertices, const List* indices)
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
		mTriSizeMul2 = EdgeFunctionFixedPoint(v0.pos, v1.pos, v2.pos);
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

		DF startEdge01 = EdgeFunctionFixedPoint(startPos, v0.pos, v1.pos);
		DF startEdge12 = EdgeFunctionFixedPoint(startPos, v1.pos, v2.pos);
		DF startEdge20 = EdgeFunctionFixedPoint(startPos, v2.pos, v0.pos);

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
				RasterizePartFixedPoint(v0.pos, v1.pos, v2.pos, x, y, 3, xEdge01, xEdge12, xEdge20);

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
#endif
	}
}

void SWRasterizer::RasterizeFloatingPoint(const List* floatingVertices, const List* indices)
{
	uint64_t indexLen = indices->GetSize();
	for (int indexIdx = 0; indexIdx < indexLen; indexIdx+=3) {
		Vertex v0 = floatingVertices->At<Vertex>(indices->At<uint32_t>(indexIdx));
		Vertex v1 = floatingVertices->At<Vertex>(indices->At<uint32_t>(indexIdx + 1));
		Vertex v2 = floatingVertices->At<Vertex>(indices->At<uint32_t>(indexIdx + 2));

		// calculate bbox covers triangle
		float minX = min(v0.pos.x, min(v1.pos.x, v2.pos.x));
		float maxX = max(v0.pos.x, max(v1.pos.x, v2.pos.x));
		float minY = min(v0.pos.y, min(v1.pos.y, v2.pos.y));
		float maxY = max(v0.pos.y, max(v1.pos.y, v2.pos.y));

		float minXFloor = floor(minX);
		float minYFloor = floor(minY);
		float startX = minX - minXFloor > 0.5f ? minXFloor + 1.5f : minXFloor + 0.5f;
		float startY = minY - minYFloor > 0.5f ? minYFloor + 1.5f : minYFloor + 0.5f;

		float triSizeMul2 = (v2.pos.x - v0.pos.x) * (v1.pos.y - v0.pos.y) - (v2.pos.y - v0.pos.y) * (v1.pos.x - v0.pos.x);

#if RASTERIZATION_TYPE == NORMAL_RASTERIZATION
		for (float y = startY; y <= mMaxY; y += 1.0f) {
			for (float x = startX; x <= mMaxX; x += 1.0f) {
				float edge01 = (x - v0.pos.x) * (v1.pos.y - v0.pos.y) - (y - v0.pos.y) * (v1.pos.x - v0.pos.x);
				float edge12 = (x - v1.pos.x) * (v2.pos.y - v1.pos.y) - (y - v1.pos.y) * (v2.pos.x - v1.pos.x);
				float edge20 = (x - v2.pos.x) * (v0.pos.y - v2.pos.y) - (y - v2.pos.y) * (v0.pos.x - v2.pos.x);				

				//TODO : 조건 if 최적화
				if (edge01 > 0
					|| edge12 > 0
					|| edge20 > 0) {
					continue;
				}

				if (edge01 == DF::ZERO
					&& !IsLeftLineFloatingPoint(v0.pos, v1.pos)
					&& !IsLeftLineFloatingPoint(v0.pos, v1.pos)) {
					continue;
				}

				if (edge12 == DF::ZERO
					&& !IsLeftLineFloatingPoint(v1.pos, v2.pos)
					&& !IsLeftLineFloatingPoint(v1.pos, v2.pos)) {
					continue;
				}

				if (edge20 == DF::ZERO
					&& !IsLeftLineFloatingPoint(v2.pos, v0.pos)
					&& !IsLeftLineFloatingPoint(v2.pos, v0.pos)) {
					continue;
				}

				float bary01 = edge01 / triSizeMul2;
				float bary12 = edge12 / triSizeMul2;
				float bary20 = edge20 / triSizeMul2;

				Vec4 pos = Vec4::ZERO;
				pos.x = x;
				pos.y = y;
				pos.w = bary12 / v0.pos.w
					+ bary20 / v1.pos.w
					+ bary01 / v2.pos.w;
				pos.z = 1.0f / pos.w;

				Pixel pixel;
				pixel.pos = pos;
				mPixels->Add(pixel);
			}
		}

#elif RATSERIZATION_TYPE == PARTITION_RASTERIZATION
#elif RASTERIZATION_TYPE == ADVANCED_RASTERIZATION
#endif

	}
}


void SWRasterizer::RasterizePartFixedPoint(
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
		AddPixelIsInTriangleFixedPoint(v0Pos, v1Pos, v2Pos, leftX, topY, edge01, edge12, edge20);
		return;
	}
	// pixel length 2
	else if (pixelLengthLog2 == 1) {
		AddPixelIsInTriangleFixedPoint(v0Pos, v1Pos, v2Pos, leftX, topY,
			edge01, edge12, edge20);
		AddPixelIsInTriangleFixedPoint(v0Pos, v1Pos, v2Pos, leftX + 1, topY,
			edge01 + mDxEdge01s[0],
			edge12 + mDxEdge12s[0],
			edge20 + mDxEdge20s[0]);
		AddPixelIsInTriangleFixedPoint(v0Pos, v1Pos, v2Pos, leftX, topY + 1,
			edge01 + mDyEdge01s[0],
			edge12 + mDyEdge12s[0],
			edge20 + mDyEdge20s[0]);
		AddPixelIsInTriangleFixedPoint(v0Pos, v1Pos, v2Pos, leftX + 1, topY + 1,
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

		RasterizePartFixedPoint(v0Pos, v1Pos, v2Pos, leftX, topY, finePixelLengthLog2, 
			edge01, edge12, edge20);
		RasterizePartFixedPoint(v0Pos, v1Pos, v2Pos, leftX + finePixelLength, topY, finePixelLengthLog2, 
			edge01 + mDxEdge01s[finePixelLengthLog2],
			edge12 + mDxEdge12s[finePixelLengthLog2],
			edge20 + mDxEdge20s[finePixelLengthLog2]);
		RasterizePartFixedPoint(v0Pos, v1Pos, v2Pos, leftX, topY + finePixelLength, finePixelLengthLog2, 
			edge01 + mDyEdge01s[finePixelLengthLog2],
			edge12 + mDyEdge12s[finePixelLengthLog2],
			edge20 + mDyEdge20s[finePixelLengthLog2]);
		RasterizePartFixedPoint(v0Pos, v1Pos, v2Pos, leftX + finePixelLength, topY + finePixelLength, finePixelLengthLog2, 
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
			pixel.pos = InterpolatePosFixedPoint(x,
				y,
				v0Pos,
				v1Pos,
				v2Pos,
				xEdge01,
				xEdge12,
				xEdge20);
			mPixels->Add(pixel);

			
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

inline void SWRasterizer::AddPixelIsInTriangleFixedPoint(const FixedVec4& v0Pos, const FixedVec4& v1Pos, const FixedVec4& v2Pos, FP x, FP y, DF edge01, DF edge12, DF edge20)
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
	bool isSelect = IsSelectPixelFixedPoint(v0Pos, v1Pos, v2Pos, x, y, edge01, edge12, edge20);
	if (isSelect == false) {
		return;
	}


	Pixel pixel;
	pixel.pos = InterpolatePosFixedPoint(x,
		y,
		v0Pos,
		v1Pos,
		v2Pos,
		edge01,
		edge12,
		edge20);


	mPixels->Add(pixel);
}

inline bool SWRasterizer::IsSelectPixelFixedPoint(const FixedVec4& v0Pos, const FixedVec4& v1Pos, const FixedVec4& v2Pos, FP x, FP y, DF edge01, DF edge12, DF edge20)
{
	//TODO : 조건 if 최적화
	if (edge01 > DF::ZERO 
		|| edge12 > DF::ZERO 
		|| edge20 > DF::ZERO) {
		return false;
	}


	if (edge01 == DF::ZERO 
		&& !IsLeftLineFixedPoint(v0Pos, v1Pos)
		&& !IsTopLineFixedPoint(v0Pos, v1Pos)) {
		return false;
	}

	if (edge12 == DF::ZERO
		&& !IsLeftLineFixedPoint(v1Pos, v2Pos)
		&& !IsTopLineFixedPoint(v1Pos, v2Pos)) {
		return false;
	}

	if (edge20 == DF::ZERO
		&& !IsLeftLineFixedPoint(v2Pos, v0Pos)
		&& !IsTopLineFixedPoint(v2Pos, v0Pos)) {
		return false;
	}

	return true;
}



inline bool SWRasterizer::IsLeftLineFixedPoint(const FixedVec4& prePos, const FixedVec4& nextPos) const
{
	return (nextPos - prePos).y < 0;
}

inline bool SWRasterizer::IsTopLineFixedPoint(const FixedVec4& prePos, const FixedVec4& nextPos) const
{
	return ((nextPos - prePos).y == 0) && ((nextPos - prePos).x > 0);
}

inline bool SWRasterizer::IsLeftLineFloatingPoint(const Vec4& prePos, const Vec4& nextPos) const
{
	return (nextPos - prePos).y < 0;
}

inline bool SWRasterizer::IsTopLineFloatingPoint(const Vec4& prePos, const Vec4& nextPos) const
{
	return ((nextPos - prePos).y == 0) && ((nextPos - prePos).x > 0);
}

inline Vec4 SWRasterizer::InterpolatePosFixedPoint(
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



