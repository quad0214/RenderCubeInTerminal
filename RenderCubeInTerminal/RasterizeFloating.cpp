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

