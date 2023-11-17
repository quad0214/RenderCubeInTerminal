#pragma once
#include "Math.h"

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