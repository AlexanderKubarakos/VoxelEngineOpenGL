#pragma once

#include <atomic>
#include <vector>

// Atomic lock-free Queue, for one consumer, one producer
// Supports one thread pushing and one threading popping
template<typename T>
class AtomicQueue
{
public:
	AtomicQueue(int t_Size);
	void push(const T& t_Element);
	T pop();
	T peek();
private:
	std::atomic_int m_Length;
	std::atomic_int m_Head;
	std::atomic_int m_Tail;
	std::vector<T> m_Data;
};