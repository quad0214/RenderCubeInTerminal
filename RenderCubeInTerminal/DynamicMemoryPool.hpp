#ifndef DYNAMICMEMORYPOOL_HPP
#define DYNAMICMEMORYPOOL_HPP

#include <cstdint>
#include "LinkedList.hpp"


// TODO : 크기 제한 코드 추가, 풀 개수 : uint8 / 요소 개수 : uin32
template <typename T>
class DynamicMemoryPool {
public:
	struct Element {
		uint8_t elementTableIndex;
		T value;
	};

private:	
	struct ElementIndex {
		uint8_t elementTableIndex;
		uint32_t elementIndex;
	};

public:
	DynamicMemoryPool(uint32_t poolCapacity);
	~DynamicMemoryPool();

	void Add(Element** pOutElement);
	bool Remove(Element* element);

	void CleanUnusedPool();

	void Debug();

	inline uint32_t GetSize() const;

private:

	// TODO : linked list to array or dynamic array?
	//			at에서 너무 효율 떨어지는 거 같음
	LinkedList<Element*> mElementTableList;	
	LinkedList<ElementIndex*> mIndexTableList;
	Node<ElementIndex*>* mUsingIndexTableNode = nullptr;
	uint32_t mSizeByte = 0;
	int mUsingIndexTableIndex = -1;
	uint32_t mCapacity = 0;	
};

#include "DynamicMemoryPool.ipp"

#endif