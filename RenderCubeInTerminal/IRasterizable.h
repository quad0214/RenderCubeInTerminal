#pragma once

#include "List.hpp"

class IRasterizable {
public:
	virtual void Rasterize(List* pixels, const List* vertices, const List* indices) = 0;
	virtual ~IRasterizable() {}
};