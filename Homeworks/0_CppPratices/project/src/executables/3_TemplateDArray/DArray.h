#pragma once
#ifndef _DYNAMICARRAY_H_
#define _DYNAMICARRAY_H_

// interfaces of Dynamic Array class DArray
template <class T>
class DArray {
public:
	DArray(); // default constructor
	DArray(int nSize, T dValue = 0); // set an array with default values
	DArray(const DArray<T>& arr); // copy constructor
	~DArray(); // deconstructor

	void Print() const; // print the elements of the array

	int GetSize() const; // get the size of the array
	void SetSize(int nSize); // set the size of the array

	const T& GetAt(int nIndex) const; // get an element at an index
	void SetAt(int nIndex, T dValue); // set the value of an element

	T& operator[](int nIndex); // overload operator '[]'
	const T& operator[](int nIndex) const; // overload operator '[]'

	void PushBack(T dValue); // add a new element at the end of the array
	void DeleteAt(int nIndex); // delete an element at some index
	void InsertAt(int nIndex, T dValue); // insert a new element at some index

	DArray<T>& operator = (const DArray<T>& arr); //overload operator '='

private:
	T* m_pData; // the pointer to the array memory
	int m_nSize; // the size of the array
	int m_nMax;

private:
	void Init(); // initilize the array
	void Free(); // free the array
	void Reserve(int nSize); // allocate enough memory
};


// implementation of class DArray
#include "DArray.h"
#include <exception>
#include <iostream>
#include <cstdio>
#include <stdexcept>

/* Policy:
   PushBack & InsertAt - reserve m_nSize when full
   Copy - equiv. to the other part
   DeleteAt - no shrink, until explicit called so
   */


   // default constructor

template<typename T>
DArray<T>::DArray() : m_nSize(0), m_pData(nullptr), m_nMax(0) {
	//Init();
}

// set an array with default values

template<typename T>
DArray<T>::DArray(int nSize, T dValue) {
	if (nSize < 0) {
		throw std::out_of_range("nSize < 0");
	}
	m_nSize = nSize;
	m_nMax = nSize;
	if (nSize > 0)
		m_pData = new T[nSize];
	else
		m_pData = nullptr;

	for (int i = 0; i < nSize; i++) {
		m_pData[i] = dValue;
	}
}


template<typename T>
DArray<T>::DArray(const DArray<T>& arr) {
	if (arr.m_nSize == 0) {
		this->m_nSize = 0;
		this->m_pData = nullptr;
		m_nMax = 0;
	}
	else {
		this->m_nSize = arr.m_nSize;
		this->m_pData = new T[arr.m_nMax];
		m_nMax = arr.m_nMax;
		for (int i = 0; i < arr.m_nSize; i++) {
			m_pData[i] = arr.m_pData[i];
		}
	}
}

// deconstructor

template<typename T>
DArray<T>::~DArray() {
	Free();
}

// allocate enough memory

template<typename T>
void DArray<T>::Reserve(int nSize) {
	if (nSize <= m_nMax)
		return;

	T* new_p = new T[nSize];
	for (int i = 0; i < m_nSize; i++) {
		new_p[i] = m_pData[i];
	}
	m_nMax = nSize;
	delete[] m_pData;
	m_pData = new_p;
}

// display the elements of the array

template<typename T>
void DArray<T>::Print() const {
	printf("[");
	for (int i = 0; i < m_nSize; i++) {
		std::cout <<  m_pData[i];
		if (i != m_nSize - 1)
			printf(",");
	}
	printf("]\n");
}

// initilize the array

template<typename T>
void DArray<T>::Init() {
	// Discard
}

// free the array

template<typename T>
void DArray<T>::Free() {
	if (m_nSize == 0)
		return;

	delete[] m_pData;
}

// get the size of the array

template<typename T>
int DArray<T>::GetSize() const {
	return m_nSize;
}

// set the size of the array
// expanded area has *uninitializd value*
// always shrink/expand

template<typename T>
void DArray<T>::SetSize(int nSize) {
	if (nSize < 0)
		throw std::out_of_range("nSize < 0");

	if (m_nSize == nSize)
		return;

	if (m_nSize > nSize) {
		// shrink
		T* new_p = nullptr;
		if (nSize != 0) {
			new_p = new T[nSize];
			for (int i = 0; i < nSize; i++) {
				new_p[i] = m_pData[i];
			}
		}
		delete[] m_pData;

		m_pData = new_p;
		m_nSize = nSize;
		m_nMax = nSize;
	}
	else {
		// expand
		T* new_p = nullptr;
		new_p = new T[nSize];
		for (int i = 0; i < m_nSize; i++) {
			new_p[i] = m_pData[i];
		}
		if (m_nSize != 0)
			delete[] m_pData;
		m_pData = new_p;
		m_nSize = nSize;
		m_nMax = nSize;
	}

}

// get an element at an index

template<typename T>
const T& DArray<T>::GetAt(int nIndex) const {
	if (nIndex >= m_nSize || nIndex < 0) {
		throw std::out_of_range("nIndex out of range");
	}

	return m_pData[nIndex];
}

// set the value of an element 

template<typename T>
void DArray<T>::SetAt(int nIndex, T dValue) {
	if (nIndex >= m_nSize || nIndex < 0) {
		throw std::out_of_range("nIndex out of range");
	}

	m_pData[nIndex] = dValue;
}

// overload operator '[]'

template<typename T>
const T& DArray<T>::operator[](int nIndex) const {
	return this->GetAt(nIndex);
}

// add a new element at the end of the array

template<typename T>
void DArray<T>::PushBack(T dValue) {
	if (m_nMax == m_nSize) {
		this->Reserve(m_nSize * 2 + 1);
	}
	m_nSize++;
	this->SetAt(this->m_nSize - 1, dValue);
}

// delete an element at some index

template<typename T>
void DArray<T>::DeleteAt(int nIndex) {
	if (m_nSize == 0 || nIndex < 0 || nIndex >= m_nSize) {
		throw std::out_of_range("nIndex invalid or DArray already empty");
	}



	if (m_nSize == 1) {
		m_nSize = 0;
		m_nMax = 0;
		m_pData = nullptr;
		return;
	}

	T* new_p = new T[(size_t)m_nSize - 1];
	for (int i = 0; i < nIndex; i++) {
		new_p[i] = m_pData[i];
	}
	for (int i = nIndex + 1; i < m_nSize; i++) {
		new_p[i - 1] = m_pData[i];
	}
	m_nSize--;
	m_pData = new_p;
	m_nMax = m_nSize;
}

// insert a new element at some index
// **insert before nIndex**
// for nIndex == m_nSize it means pushback

template<typename T>
void DArray<T>::InsertAt(int nIndex, T dValue) {
	if (nIndex < 0 || nIndex > m_nSize) {
		throw std::out_of_range("nIndex invalid");
	}
	if (nIndex == m_nSize) {
		this->PushBack(dValue);
		return;
	}

	T* new_p = m_pData;
	if (m_nSize == m_nMax) { // already full, expand
		new_p = new T[m_nSize * 2 + 1];
		m_nMax = m_nSize * 2 + 1;

		for (int i = 0; i < nIndex; i++) {
			new_p[i] = m_pData[i];
		}
	}
	for (int i = m_nSize; i >= nIndex + 1; i--) {
		new_p[i] = m_pData[i - 1];
	}
	new_p[nIndex] = dValue;

	m_nSize++;
	m_pData = new_p;
}

// overload operator '='
template<typename T>
DArray<T>& DArray<T>::operator = (const DArray& arr) {
	Free();
	if (arr.m_nSize == 0) {
		this->m_nSize = 0;
		this->m_pData = nullptr;
		this->m_nMax = 0;
	}
	else {
		this->m_nSize = arr.m_nSize;
		this->m_pData = new T[arr.m_nMax];
		this->m_nMax = arr.m_nMax;
		for (int i = 0; i < arr.m_nSize; i++) {
			m_pData[i] = arr.m_pData[i];
		}
	}
	return *this;
}


#endif // !_DYNAMICARRAY_H_