#ifndef DYNAMICMEMORYPOOL_IPP
#define DYNAMICMEMORYPOOL_IPP

#include "DynamicMemoryPool.hpp"
#include "LinkedList.hpp"
#include <cstring>
#include <iostream>


template<typename T>
DynamicMemoryPool<T>::DynamicMemoryPool(uint32_t poolCapacity) : mCapacity(poolCapacity){	
}

template<typename T>
DynamicMemoryPool<T>::~DynamicMemoryPool()
{
	// element table list
	{
		Node<Element*>* cur = mElementTableList.GetHead();
		while (cur != nullptr) {
			delete[] cur->value;
			cur->value = nullptr;
			cur = cur->pNext;
		}
	}

	// index table list
	{
		Node<ElementIndex*>* cur = mIndexTableList.GetHead();
		while (cur != nullptr) {
			delete[] cur->value;
			cur->value = nullptr;
			cur = cur->pNext;
		}
	}
}

template<typename T>
void DynamicMemoryPool<T>::Add(Element** pOutElement)
{
	// when new index is out of all element table
	// TODO : element 새로 만들시 동일한 값(index|0) 쓰는 거 최적화
	if (mSizeByte + 1 > mElementTableList.GetSize() * mCapacity) {
		Element* newElementTable = new Element[mCapacity];
		mElementTableList.AddLast(newElementTable);

		for (int i = 0; i < mCapacity; i++) {
			newElementTable[i] = { static_cast<uint8_t>(mElementTableList.GetSize() - 1), 0 };
		}
		
	}	
	
	// when new index is out of using index table
	if (mSizeByte + 1 > (mUsingIndexTableIndex + 1) * mCapacity) {
		// create index table
		mIndexTableList.AddLast(new ElementIndex[mCapacity]);
		for (int i = 0; i < mCapacity; i++)
			mIndexTableList.GetLast()[i] = {static_cast<uint8_t>(mIndexTableList.GetSize() - 1), static_cast<uint32_t>(i)};

		// move using table to next
		mUsingIndexTableIndex++;
		mUsingIndexTableNode = mUsingIndexTableNode == nullptr ? mIndexTableList.GetHead() : mUsingIndexTableNode->pNext;
	}

	const uint32_t nextEndIdxOfIndexTable = mSizeByte - mCapacity * mUsingIndexTableIndex;
	const ElementIndex& index = mUsingIndexTableNode->value[nextEndIdxOfIndexTable];
	Element* elementTable = mElementTableList.At(index.elementTableIndex);	

	*pOutElement = &elementTable[index.elementIndex];
	mSizeByte++;
}

template<typename T>
bool DynamicMemoryPool<T>::Remove(Element* element)
{
	// pool doesn't have elements
	if (mSizeByte < 0) {
		return false;
	}

	const Element* elementTable = mElementTableList.At(element->elementTableIndex);			
	uint32_t endIdxOfIndexTable = (mSizeByte - 1) - mUsingIndexTableIndex * mCapacity;
	uint32_t elementIdxInPool = element - elementTable;
	// element doesn't place in pool
	if (elementIdxInPool < 0 || elementIdxInPool >= mCapacity) {
		return false;
	}

	// update available element index table
	mUsingIndexTableNode->value[endIdxOfIndexTable] = { element->elementTableIndex,  elementIdxInPool};

	// reduce size
	mSizeByte--;
	if (mSizeByte <= mUsingIndexTableIndex * mCapacity) {
		mUsingIndexTableIndex--;
		mUsingIndexTableNode = mUsingIndexTableNode->pPrev;
	}

	return true;
}

template<typename T>
void DynamicMemoryPool<T>::CleanUnusedPool()
{
	// clean unused index table
	if (mUsingIndexTableIndex < static_cast<int>(mIndexTableList.GetSize() - 1)) {		
		int remainTableListNum = mIndexTableList.GetSize() - 1 - mUsingIndexTableIndex;
		for (; remainTableListNum > 0; remainTableListNum--) {
			delete[] mIndexTableList.GetLast();
			mIndexTableList.RemoveLast();
		}		
	}
}

template<typename T>
inline void DynamicMemoryPool<T>::Debug()
{
	using namespace std;
	cout << "element" << endl;
	Node<Element*>* elementNode = mElementTableList.GetHead();
	while (elementNode != nullptr) {
		cout << (int)(elementNode->value[0].elementTableIndex) << endl;
		for (int i = 0; i < mCapacity; i++) {
			cout << elementNode->value[i].value << " ";
		}
		cout << endl;
		elementNode = elementNode->pNext;
	}

	cout << "index" << endl;
	Node<ElementIndex*>* indexNode = mIndexTableList.GetHead();
	while (indexNode != nullptr) {
		for (int i = 0; i < mCapacity; i++) {
			cout << (int)(indexNode->value[i].elementTableIndex) << "|" << indexNode->value[i].elementIndex << " ";
		}
		cout << endl;
		indexNode = indexNode->pNext;
	}
	
}

template<typename T>
uint32_t DynamicMemoryPool<T>::GetSize() const
{
	return mSizeByte;
}

#endif