#pragma once

#include <stdexcept>
#include <atomic>
#include <vector>

//TODO: make dynamic size

// Atomic lock-free Queue, for one consumer, one producer
// Supports one thread pushing and one threading popping
template<typename T>
class AtomicQueue
{
public:
	AtomicQueue(const int t_Size)
	{
		m_Size = t_Size + 1;
		m_Data = new T[m_Size];
		m_Head = 0; // Head points to empty
		m_Tail = 0; // Tail points to full
	}

	~AtomicQueue()
	{
		delete[] m_Data;
	}

	void push(const T& t_Element)
	{
		if ((m_Head + 1) % m_Size == m_Tail)
			throw std::out_of_range("Atomic Queue is Full, can't push data onto it");

		m_Data[m_Head] = t_Element;
		m_Head = (m_Head + 1) % m_Size;
	}

	T pop()
	{
		if (m_Head == m_Tail)
			throw std::out_of_range("Atomic Queue is Empty"); // empty
		T copy = std::move(m_Data[m_Tail]);
		m_Tail = (m_Tail + 1) % m_Size;
		return copy;
	}
	T peek()
	{
		if (m_Head == m_Tail)
			throw std::out_of_range("Atomic Queue is Empty"); // empty
		return m_Data[m_Tail];
	}

	bool empty()
	{
		return m_Head == m_Tail;
	}

	int length()
	{
		if (m_Head == m_Tail)
			return 0;
		const int diff = m_Head - m_Tail;
		if (diff > 0)
		{
			return diff;
		}

		return m_Size + diff;
	}
private:
	std::atomic_int m_Head;
	std::atomic_int m_Tail;
	T* m_Data;
	int m_Size;
};