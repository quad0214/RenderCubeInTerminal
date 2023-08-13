#ifndef STATICMEMORYPOOL_HPP
#define STATICMEMORYPOOL_HPP

template<typename T>
class StaticMemoryPool {
public:
	StaticMemoryPool(int capacity);
	StaticMemoryPool(int capacity, const T* copyArr, int copyArrLen);
	~StaticMemoryPool();
	
	bool Add(T** pOutElement);
	bool Remove(T* element);	

	inline unsigned int GetSize() const;

private:
	T* mElements = nullptr;
	unsigned int* mIndexTable = nullptr;
	unsigned int mSizeByte = 0;
	unsigned int mCapacity = 0;	
};

#include "StaticMemoryPool.ipp";

#endif