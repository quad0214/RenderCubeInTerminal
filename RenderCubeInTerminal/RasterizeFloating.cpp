#include "RasterizeFloating.h"

void RasterizeFloating::Rasterize(List* pixels, const List* floatingVertices, const List* indices)
{
	uint64_t indexLen = indices->GetSize();
	for (int indexIdx = 0; indexIdx < indexLen; indexIdx += 3) {
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
		for (float y = startY; y <= maxY; y += 1.0f) {
			for (float x = startX; x <= maxX; x += 1.0f) {

				float edge01;
				float edge12;
				float edge20;

				edge01 = (x - v0.pos.x) * (v1.pos.y - v0.pos.y) - (y - v0.pos.y) * (v1.pos.x - v0.pos.x);
				edge12 = (x - v1.pos.x) * (v2.pos.y - v1.pos.y) - (y - v1.pos.y) * (v2.pos.x - v1.pos.x);
				edge20 = (x - v2.pos.x) * (v0.pos.y - v2.pos.y) - (y - v2.pos.y) * (v0.pos.x - v2.pos.x);

				if (edge01 == edge12 && edge12 == edge20) {
					__debugbreak();
				}

				// edge function
				// = (x - v0.x) * (v1.y - v0.y) - (y - v0.y) * (v1.x - v0.x)
				// = (eA[0] - eA[1]) * (eA[2] - eA[3]) - (eA[4] - eA[5]) * (eA[6] - eA[7])
				// = eB[0] * eB[1] - eB[2] * eB[3]
				// = eC[0] - eC[1]
				// = edge

				__declspec(align(16)) float eA01[8]{x, v0.pos.x, v1.pos.y, v0.pos.y, y, v0.pos.y, v1.pos.x, v0.pos.x};
				__declspec(align(16)) float eA12[8]{x, v1.pos.x, v2.pos.y, v1.pos.y, y, v1.pos.y, v2.pos.x, v1.pos.x};
				__declspec(align(16)) float eA20[8]{x, v2.pos.x, v0.pos.y, v2.pos.y, y, v2.pos.y, v0.pos.x, v2.pos.x};
				__declspec(align(16)) float eB01[4];
				__declspec(align(16)) float eB12[4];
				__declspec(align(16)) float eB20[4];
				__declspec(align(16)) float eC01[2];
				__declspec(align(16)) float eC12[2];
				__declspec(align(16)) float eC20[2];

				eB01[0] = eA01[0] - eA01[1];
				eB01[1] = eA01[2] - eA01[3];
				eB01[2] = eA01[4] - eA01[5];
				eB01[3] = eA01[6] - eA01[7];
				
				eB12[0] = eA12[0] - eA12[1];
				eB12[1] = eA12[2] - eA12[3];
				eB12[2] = eA12[4] - eA12[5];
				eB12[3] = eA12[6] - eA12[7];
				
				eB20[0] = eA20[0] - eA20[1];
				eB20[1] = eA20[2] - eA20[3];
				eB20[2] = eA20[4] - eA20[5];
				eB20[3] = eA20[6] - eA20[7];
								
				eC01[0] = eB01[0] * eB01[1];
				eC01[1] = eB01[2] * eB01[3];
				eC12[0] = eB12[0] * eB12[1];
				eC12[1] = eB12[2] * eB12[3];
				eC20[0] = eB20[0] * eB20[1];
				eC20[1] = eB20[2] * eB20[3];

				edge01 = eC01[0] - eC01[1];
				edge12 = eC12[0] - eC12[1];
				edge20 = eC20[0] - eC20[1];

				

				//TODO : 조건 if 최적화
				if (edge01 > 0
					|| edge12 > 0
					|| edge20 > 0) {
					continue;
				}


				if (edge01 == DF::ZERO
					&& !IsLeftLineFloatingPoint(v0.pos, v1.pos)
					&& !IsTopLineFloatingPoint(v0.pos, v1.pos)) {
					continue;
				}

				if (edge12 == DF::ZERO
					&& !IsLeftLineFloatingPoint(v1.pos, v2.pos)
					&& !IsTopLineFloatingPoint(v1.pos, v2.pos)) {
					continue;
				}

				if (edge20 == DF::ZERO
					&& !IsLeftLineFloatingPoint(v2.pos, v0.pos)
					&& !IsTopLineFloatingPoint(v2.pos, v0.pos)) {
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
				pixels->Add(pixel);
			}
		}

#elif RATSERIZATION_TYPE == PARTITION_RASTERIZATION
#elif RASTERIZATION_TYPE == ADVANCED_RASTERIZATION
#endif

	}
}

inline bool RasterizeFloating::IsLeftLineFloatingPoint(const Vec4& prePos, const Vec4& nextPos) const
{
	return (nextPos - prePos).y < 0;
}

inline bool RasterizeFloating::IsTopLineFloatingPoint(const Vec4& prePos, const Vec4& nextPos) const
{
	return ((nextPos - prePos).y == 0) && ((nextPos - prePos).x > 0);
}

