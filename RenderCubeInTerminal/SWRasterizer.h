#pragma once

//#define DEBUG_PROCESS_COORDINATE
// todo : divide swrasterizer's pipeline. remove conversion to fixedvec4 in floating pipeline
//#define FIXED_NUMBER_RASTERIZATION
//#define PARTITION_RASTERIZATION

#include "DynamicMemoryPool.hpp"
#include "Math.h"
#include "List.hpp"

class SWRasterizer {
public:
	struct Vertex {
		Vec4 pos;

		Vertex() : pos(Vec4::ZERO) {
		}

		Vertex(Vec4 pos) : pos(pos) {}
	};

	struct FixedVertex {
		FixedVec4 pos;

		FixedVertex() : pos(FixedVec4::ZERO) {

		}

		FixedVertex(FixedVec4 pos) : pos(pos) {

		}

		FixedVertex(Vec4 pos) : pos(FixedVec4(FP(pos.x), FP(pos.y), FP(pos.z), FP(pos.w))) {
		}


	};

	struct Triangle {
		union {
			Vertex vertices[3];
			struct {
				Vertex v1;
				Vertex v2;
				Vertex v3;
			};
		};

		Triangle(Vertex v1, Vertex v2, Vertex v3) : v1(v1), v2(v2), v3(v3) {}
	};

	struct Pixel {
		Vec4 pos;

		Pixel() : pos(Vec4::ZERO) {}
		Pixel(Vec4 pos) : pos(pos) {}
	};

	// limit size of viewport for avoiding fixed point overflow
	// -2^(FP::INT_BITS_LEN-2) <= x, y <= 2^(FP::INT_BITS_LEN-2) - 1	
	// TODO : think size of viewport. in direct3d 11 functional specification, 
	//		uses 16.8 fixed point and allows range of x, y of viewport is [-2^15, 2^15 - 1].
	//		but maximun value of edge function is (2^15)^2. it exceeds range of fixed point.
	// TODO : check size of viewport
	struct Viewport {
		uint32_t leftX;
		uint32_t topY;
		uint32_t width;
		uint32_t height;
		float minZ;
		float maxZ;
	};

private:
	enum class PlaneID {
		NegX = 0,
		PosX,
		NegY,
		PosY,
		NegZ,
		PosZ,
		Length
	};

	static constexpr float HOMOGENEOUS_VERTEX_MIN_Z = 1e-6f;
	static constexpr uint64_t RESERVED_VERTICES_BYTES = 1024 * 1024 * 1; // 1mb
	static constexpr uint64_t RESERVED_INDICES_BYTES = 1024 * 1024 * 0.5f; // 0.5mb
	static constexpr uint64_t RESERVED_PIXELS_BYTES = 128 * 128 * sizeof(Pixel);

public:
	SWRasterizer();

	// assume cw
	void Initialize();
	void Terminate();

	void Execute(const Vertex* vertices, int vertexNum, const uint32_t* indices, int indexNum);
	inline uint64_t GetPixelLength() const {
		return mPixels->GetSize();
	}
	inline const Pixel& GetPixel(uint32_t index) const {
		return mPixels->At<SWRasterizer::Pixel>(index);
	}

	void SetupViewport(const Viewport& viewport);

private:
	// clip
	void Clip(List** pClippedVertices, List** pClippedIndices, const List* vertices, const List* indices);
	void ClipTriangle(bool* pIsClipped, uint8_t* pClippedIndexNum, uint8_t* pClippedVertexNum, Vertex(&clippedVertices)[9], uint32_t(&clippedIndices)[21], const Triangle& triangle);
	inline bool IsInPlane(PlaneID planeID, const Vertex& clipVertex);
	inline float GetSignedDstWithPlane(PlaneID planeID, const Vertex& clipVertex);
	inline Vertex CalculateInterVertex(PlaneID planeID, const Vertex& inV, const Vertex& outV);

	// perspective division
	void DividePerspective(List** pVertices);

	// back face culling
	void CullBackFace(List** pCulledIndices, const List* indices, const List* vertices);

	// viewport
	void TransformViewport(List** pVertices);

	// Convert to FP
	void ConvertFixedPoint(List** pOutVertices, const List* vertices);

	// Rasterization
	void RasterizeFixedPoint(const List* fixedVertices, const List* indices);
	void RasterizeFloatingPoint(const List* floatingVertices, const List* indices);

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

	void RasterizePartFixedPoint(
		const FixedVec4& v0Pos,
		const FixedVec4& v1Pos,
		const FixedVec4& v2Pos,
		FP leftX,
		FP topY,
		uint8_t pixelLengthLog2,		
		DF edge01,
		DF edge12,
		DF edge20);
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

	inline void AddPixelIsInTriangleFixedPoint(
		const FixedVec4& v0Pos,
		const FixedVec4& v1Pos,
		const FixedVec4& v2Pos,
		FP x,
		FP y,
		DF edge01,
		DF edge12,
		DF edge20);
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

	inline bool IsSelectPixelFixedPoint(
		const FixedVec4& v0Pos,
		const FixedVec4& v1Pos,
		const FixedVec4& v2Pos,
		FP x,
		FP y,
		DF edge01,
		DF edge12,
		DF edge20
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

	inline bool IsLeftLineFixedPoint(const FixedVec4& prePos, const FixedVec4& nextPos) const;
	inline bool IsTopLineFixedPoint(const FixedVec4& prePos, const FixedVec4& nextPos) const;
	inline bool IsLeftLineFloatingPoint(const Vec4& prePos, const Vec4& nextPos) const;
	inline bool IsTopLineFloatingPoint(const Vec4& prePos, const Vec4& nextPos) const;


	inline Vec4 InterpolatePosFixedPoint(
		FP x,
		FP y,
		const FixedVec4& v0Pos,
		const FixedVec4& v1Pos,
		const FixedVec4& v2Pos, 
		DF edge01,
		DF edge12, 
		DF edge20) const;

private:	
	int mVertexNum = 0;
	int mIndexNum = 0;
	Viewport mViewport = {0};

	List* mPixels = nullptr;
	List* mVerticesPool[2] = {nullptr, };
	List* mIndicesPool[2] = {nullptr, };

	// dxEdgeFunctionValue, 2 * dxEdgeFunctionValue, 4 * dxEdgeFunctionValue, 8 * dxEdgeFunctionValue
	DF mDxEdge01s[4];
	DF mDxEdge12s[4];
	DF mDxEdge20s[4];
	// dyEdgeFunctionValue, 2 * dyEdgeFunctionValue, 4 * dyEdgeFunctionValue, 8 * dyEdgeFunctionValue
	DF mDyEdge01s[4];
	DF mDyEdge12s[4];
	DF mDyEdge20s[4];
	
	// bbox covers triangle
	FP mMinX = FP::ZERO;
	FP mMaxX = FP::ZERO;
	FP mMinY = FP::ZERO;
	FP mMaxY = FP::ZERO;



};