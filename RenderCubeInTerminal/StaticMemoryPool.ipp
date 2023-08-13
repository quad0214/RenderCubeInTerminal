#ifndef STATICMEMORYPOOL_IPP
#define STATICMEMORYPOOL_IPP

#include "StaticMemoryPool.hpp"
#include <cstring>

template<typename T>
StaticMemoryPool<T>::StaticMemoryPool(int capacity) : mCapacity(capacity)
{
	mElements = new T[capacity];
	mIndexTable = new unsigned[capacity];
	for (int i = 0; i < capacity; i++) {
		mIndexTable[i] = i;
	}	

	mSizeByte = 0;
}

template<typename T>
StaticMemoryPool<T>::StaticMemoryPool(int capacity, const T* copyArr, int copyArrLen) : StaticMemoryPool(capacity)
{
	std::memcpy(mElements, copyArr, copyArrLen * sizeof(T));
	mSizeByte = copyArrLen;
}

template<typename T>
inline StaticMemoryPool<T>::~StaticMemoryPool()
{
	if (mElements) {
		delete[] mElements;
		mElements = nullptr;
	}

	if (mIndexTable) {
		delete[] mIndexTable;
		mIndexTable = nullptr;
	}
}

template<typename T>
bool StaticMemoryPool<T>::Add(T** pOutElement)
{
	if (pOutElement == nullptr) {
		return false;
	}

	if (mSizeByte >= mCapacity) {
		return false;
	}

	int newIndex = mIndexTable[mSizeByte];
	*pOutElement = mElements[newIndex];
	mSizeByte++;

	return true;
}

template<typename T>
bool StaticMemoryPool<T>::Remove(T* element)
{
	int index = element - mElements;

	// not created or not in memory
	if (index >= mSizeByte) {
		return false;
	}

	mIndexTable[mSizeByte - 1] = index;
	mSizeByte--;

	return true;
}

template<typename T>
inline unsigned int StaticMemoryPool<T>::GetSize() const
{
	return mSizeByte;
}

#endif