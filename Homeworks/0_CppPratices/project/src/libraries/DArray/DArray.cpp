// implementation of class DArray
#include <DArray.h>
#include <exception>
#include <iostream>
#include <cstdio>
#include <stdexcept>

// default constructor
DArray::DArray() : m_nSize(0), m_pData(nullptr) {
	//Init();
}

// set an array with default values
DArray::DArray(int nSize, double dValue) {
	if (nSize < 0) {
		throw std::out_of_range("nSize < 0");
	}
	m_nSize = nSize;
	if (nSize > 0)
		m_pData = new double[nSize];
	else
		m_pData = nullptr;

	for (int i = 0; i < nSize; i++) {
		m_pData[i] = dValue;
	}
}

DArray::DArray(const DArray& arr) {
	if (arr.m_nSize == 0) {
		this->m_nSize = 0;
		this->m_pData = nullptr;
	}
	else {
		this->m_nSize = arr.m_nSize;
		this->m_pData = new double[arr.m_nSize];
		for (int i = 0; i < arr.m_nSize; i++) {
			m_pData[i] = arr.m_pData[i];
		}
	}
}

// deconstructor
DArray::~DArray() {
	Free();
}

// display the elements of the array
void DArray::Print() const {
	printf("[");
	for (int i = 0; i < m_nSize; i++) {
		printf("%lf", m_pData[i]);
		if (i != m_nSize - 1)
			printf(",");
	}
	printf("]\n");
}

// initilize the array
void DArray::Init()  {
	// Discard
}

// free the array
void DArray::Free() {
	if (m_nSize == 0)
		return;

	delete [] m_pData;
}

// get the size of the array
int DArray::GetSize() const {
	return m_nSize;
}

// set the size of the array
// expanded area has *uninitializd value*
void DArray::SetSize(int nSize) {
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
	} else {
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
	}
		
}

// get an element at an index
const double& DArray::GetAt(int nIndex) const {
	if (nIndex >= m_nSize || nIndex < 0) {
		throw std::out_of_range("nIndex out of range");
	}

	return m_pData[nIndex];
}

// set the value of an element 
void DArray::SetAt(int nIndex, double dValue) {
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
	this->SetSize(m_nSize + 1);
	this->SetAt(m_nSize - 1, dValue);
}

// delete an element at some index
void DArray::DeleteAt(int nIndex) {
	if (m_nSize == 0 || nIndex < 0 || nIndex >= m_nSize) {
		throw std::out_of_range("nIndex invalid or DArray already empty");
		return;
	}

	if (m_nSize == 1) {
		m_nSize = 0;
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
}

// insert a new element at some index
// **insert before nIndex**
// for nIndex == m_nSize it means pushback
void DArray::InsertAt(int nIndex, double dValue) {
	if (nIndex < 0 || nIndex > m_nSize) {
		throw std::out_of_range("nIndex invalid");
	}
	if (nIndex == m_nSize) {
		this->PushBack(dValue);
		return;
	}

	double* new_p = new double[(size_t)m_nSize + 1];
	for (int i = 0; i < nIndex; i++) {
		new_p[i] = m_pData[i];
	}
	new_p[nIndex] = dValue;
	for (int i = nIndex + 1; i < m_nSize + 1; i++) {
		new_p[i] = m_pData[i - 1];
	}
	m_nSize++;
	m_pData = new_p;
}

// overload operator '='
DArray& DArray::operator = (const DArray& arr) {
	Free();
	if (arr.m_nSize == 0) {
		this->m_nSize = 0;
		this->m_pData = nullptr;
	} else {
		this->m_nSize = arr.m_nSize;
		this->m_pData = new double[arr.m_nSize];
		for (int i = 0; i < arr.m_nSize; i++) {
			m_pData[i] = arr.m_pData[i];
		}
	}
	return *this;
}
