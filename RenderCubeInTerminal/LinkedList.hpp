//#pragma once

#ifndef LINKEDLIST_HPP
#define LINKEDLIST_HPP
#include <cstdint>

template<typename T>
class Node {
public:
	Node(const T& value, Node<T>* pNext, Node<T>* pPrev) : value(value), pNext(pNext), pPrev(pPrev) {};
	~Node() {};

public:
	T value;
	Node<T>* pNext = nullptr;
	Node<T>* pPrev = nullptr;
};

template<typename T>
class LinkedList {

public:
	LinkedList();
	~LinkedList();

	void AddFirst(const T& value);
	void AddLast(const T& value);
	bool RemoveFirst();
	bool RemoveLast();

	const T& At(uint32_t index) const;

	inline Node<T>* GetHead() const;
	inline Node<T>* GetTail() const;
	inline T GetFirst() const;
	inline T GetLast() const;
	inline uint32_t GetSize() const;
		

private:	
	Node<T>* mHead = nullptr;
	Node<T>* mTail = nullptr;
	uint32_t mSizeByte = 0;
	
};

#include "LinkedList.ipp"
#endif