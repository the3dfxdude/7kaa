/*
 * Seven Kingdoms: Ancient Adversaries
 *
 * Copyright 1997,1998 Enlight Software Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

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


