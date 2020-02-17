// implementation of class DArray
#include "DArray.h"
#include <exception>
#include <iostream>
#include <cstdio>
#include <stdexcept>
#include <cassert>

/* Policy:
   PushBack & InsertAt - reserve m_nSize when full
   Copy - equiv. to the other part
   DeleteAt - no shrink, until explicit called so
   */


// default constructor
DArray::DArray() : m_nSize(0), m_pData(nullptr), m_nMax(0) {
	//Init();
}

// set an array with default values
DArray::DArray(int nSize, double dValue) {
	assert(this->m_nSize <= this->m_nMax);
	if (nSize < 0) {
		throw std::out_of_range("nSize < 0");
	}
	m_nSize = nSize;
	m_nMax = nSize;
	if (nSize > 0)
		m_pData = new double[nSize];
	else
		m_pData = nullptr;

	for (int i = 0; i < nSize; i++) {
		m_pData[i] = dValue;
	}
}

DArray::DArray(const DArray& arr) {
	assert(arr.m_nSize <= arr.m_nMax);
	if (arr.m_nSize == 0) {
		this->m_nSize = 0;
		this->m_pData = nullptr;
		m_nMax = 0;
	}
	else {
		this->m_nSize = arr.m_nSize;
		this->m_pData = new double[arr.m_nMax];
		m_nMax = arr.m_nMax;
		for (int i = 0; i < arr.m_nSize; i++) {
			m_pData[i] = arr.m_pData[i];
		}
	}
}

// deconstructor
DArray::~DArray() {
	Free();
}

// allocate enough memory
void DArray::Reserve(int nSize) {
	assert(this->m_nSize <= this->m_nMax);
	if (nSize <= m_nMax)
		return;

	double* new_p = new double[nSize];
	for (int i = 0; i < m_nSize; i++) {
		new_p[i] = m_pData[i];
	}
	m_nMax = nSize;
	delete[] m_pData;
	m_pData = new_p;
}

// display the elements of the array
void DArray::Print() const {
	assert(this->m_nSize <= this->m_nMax);
	printf("[");
	for (int i = 0; i < m_nSize; i++) {
		printf("%lf", m_pData[i]);
		if (i != m_nSize - 1)
			printf(",");
	}
	printf("]\n");
}

// initilize the array
void DArray::Init() {
	// Discard
}

// free the array
void DArray::Free() {
	assert(this->m_nSize <= this->m_nMax);
	if (m_nSize == 0)
		return;

	delete[] m_pData;
}

// get the size of the array
int DArray::GetSize() const {
	return m_nSize;
}

// set the size of the array
// expanded area has *uninitializd value*
// always shrink/expand
void DArray::SetSize(int nSize) {
	assert(this->m_nSize <= this->m_nMax);
	if (nSize < 0)
		throw std::out_of_range("nSize < 0");

	if (m_nSize == nSize)
		return;

	if (m_nSize > nSize) {
		// shrink
		double* new_p = nullptr;
		if (nSize != 0) {
			new_p = new double[nSize];
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
		double* new_p = nullptr;
		new_p = new double[nSize];
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
const double& DArray::GetAt(int nIndex) const {
	assert(this->m_nSize <= this->m_nMax);
	if (nIndex >= m_nSize || nIndex < 0) {
		throw std::out_of_range("nIndex out of range");
	}

	return m_pData[nIndex];
}

// set the value of an element 
void DArray::SetAt(int nIndex, double dValue) {
	assert(this->m_nSize <= this->m_nMax);
	if (nIndex >= m_nSize || nIndex < 0) {
		throw std::out_of_range("nIndex out of range");
	}

	m_pData[nIndex] = dValue;
}

// overload operator '[]'
const double& DArray::operator[](int nIndex) const {
	return this->GetAt(nIndex);
}

// add a new element at the end of the array
void DArray::PushBack(double dValue) {
	if (m_nMax == m_nSize) {
		this->Reserve(m_nSize * 2 + 1);
	}
	m_nSize++;
	this->SetAt(this->m_nSize - 1, dValue);
}

// delete an element at some index
void DArray::DeleteAt(int nIndex) {
	assert(this->m_nSize <= this->m_nMax);
	if (m_nSize == 0 || nIndex < 0 || nIndex >= m_nSize) {
		throw std::out_of_range("nIndex invalid or DArray already empty");
	}
	if (m_nSize == 1) {
		m_nSize = 0;
		m_nMax = 0;
		m_pData = nullptr;
		return;
	}
	double* new_p = new double[(size_t)m_nSize - 1];
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
void DArray::InsertAt(int nIndex, double dValue) {
	assert(this->m_nSize <= this->m_nMax);
	if (nIndex < 0 || nIndex > m_nSize) {
		throw std::out_of_range("nIndex invalid");
	}
	if (nIndex == m_nSize) {
		this->PushBack(dValue);
		return;
	}

	double* new_p = m_pData;
	if (m_nSize == m_nMax) { // already full, expand
		new_p = new double[(size_t)m_nSize * 2 + 1];
		m_nMax = m_nSize * 2 + 1;
			
		for (int i = 0; i < nIndex; i++) {
			new_p[i] = m_pData[i];
		}
	}
	for (int i = m_nSize ; i >= nIndex + 1; i--) {
		new_p[i] = m_pData[i - 1];
	}
	new_p[nIndex] = dValue;

	m_nSize++;
	m_pData = new_p;
}

// overload operator '='
DArray& DArray::operator = (const DArray& arr) {
	assert(this->m_nSize <= this->m_nMax);
	Free();
	if (arr.m_nSize == 0) {
		this->m_nSize = 0;
		this->m_pData = nullptr;
		this->m_nMax = 0;
	}
	else {
		this->m_nSize = arr.m_nSize;
		this->m_pData = new double[arr.m_nMax];
		this->m_nMax = arr.m_nMax;
		for (int i = 0; i < arr.m_nSize; i++) {
			m_pData[i] = arr.m_pData[i];
		}
	}
	return *this;
}
