// Filename    : OTARRAY.H
// Description : template for temporary array


#ifndef __OTARRAY_H
#define __OTARRAY_H

// to create a temp. array of char[10], 
// TArray<char> charArray(10);

// --------- define template class TArray --------//

template TArray <class T>
class TArray
{
private:
	T* array_ptr;
	int array_size;

public:
	TArray( int s );
	~TArray();

	T& operator[](int i)		{ return array_ptr[i]; }
	int size();					{ return array_size; }
};


// --------- begin of function TArray::TArray --------//
template TArray <class T>
TArray<T>::TArray(int s) : array_ptr(new T[s]), array_size(s)
{
}
// --------- end of function TArray::TArray --------//


// --------- begin of function TArray::~TArray --------//
template TArray <class T>
TArray<T>::~TArray()
{
	delete[] array_ptr;
}
// --------- end of function TArray::~TArray --------//


#endif


