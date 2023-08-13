//#pragma once

#ifndef LINKEDLIST_IPP
#define LINKEDLIST_IPP
#include "LinkedList.hpp"

template<typename T>
LinkedList<T>::LinkedList() : mHead(nullptr), mTail(nullptr), mSizeByte(0)
{

}

template<typename T>
LinkedList<T>::~LinkedList()
{
	Node<T>* cur = mHead;
	while (cur != nullptr) {
		Node<T>* temp = cur;
		cur = cur->pNext;
		delete temp;
	}

	mHead = nullptr;
	mTail = nullptr;
}

template<typename T>
void LinkedList<T>::AddFirst(const T& value)
{
	mSizeByte++;
	if (mHead == nullptr) {
		mHead = mTail = new Node<T>(value, nullptr, nullptr);
		return;
	}

	Node<T>* newNode = new Node<T>(value, mHead, nullptr);	
	mHead->pPrev = newNode;
	mHead = newNode;	
}

template<typename T>
void LinkedList<T>::AddLast(const T& value)
{
	mSizeByte++;
	if (mHead == nullptr) {
		mHead = mTail = new Node<T>(value, nullptr, nullptr);		
		return;
	}
	
	Node<T>* newNode = new Node<T>(value, nullptr, mTail);	
	mTail->pNext = newNode;
	mTail = newNode;	
}

template<typename T>
bool LinkedList<T>::RemoveFirst()
{
	if (mHead == nullptr) {
		return false;
	}

	mSizeByte--;
	Node<T>* deleted = mHead;

	if (mHead == mTail) {
		mHead = mTail = nullptr;
	}
	else {
		mHead->pNext->pPrev = nullptr;
		mHead = mHead->pNext;		
	}

	delete deleted;	
}

template<typename T>
bool LinkedList<T>::RemoveLast()
{
	if (mTail == nullptr) {
		return false;
	}

	mSizeByte--;

	Node<T>* deleted = mTail;

	if (mHead == mTail) {
		mHead = mTail = nullptr;
	}
	else {
		mTail->pPrev->pNext = nullptr;
		mTail = mTail->pPrev;
	}

	delete deleted;	
}

template<typename T>
const T& LinkedList<T>::At(uint32_t index) const {
	if (index >= mSizeByte) {
		return nullptr;
	}

	Node<T>* finded = nullptr;
	if (index < mSizeByte / 2) {
		finded = mHead;
		for (int i = 0; i < index; i++)
			finded = finded->pNext;
	}
	else {
		finded = mTail;
		for (int i = 0; i < mSizeByte - 1 - index; i++)
			finded = finded->pPrev;
	}

	return finded->value;
}

template<typename T>
inline Node<T>* LinkedList<T>::GetHead() const {
	return mHead;
}

template<typename T>
inline  Node<T>* LinkedList<T>::GetTail() const {
	return mTail;
}

template<typename T>
inline uint32_t LinkedList<T>::GetSize() const {
	return mSizeByte;
}

template<typename T>
inline T LinkedList<T>::GetFirst() const {	
	return mHead->value;
}

template<typename T>
inline T LinkedList<T>::GetLast() const {
	return mTail->value;
}

#endif