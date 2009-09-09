// Filename    : OBLOB.CPP
// Description : fixed size binary block


#include <OBLOB.H>
#include <ALL.H>


Blob::Blob() : ptr(NULL), size(0)
{
}


Blob::Blob(int s) : ptr(mem_add(s)), size(s)
{
}


Blob::Blob(Blob &b) : ptr(mem_add(b.size)), size(b.size)
{
	memcpy(ptr, b.ptr, b.size);
}

Blob::~Blob()
{
	mem_del(ptr);
}

void Blob::resize(int s)
{
	ptr = (char *)mem_resize(ptr, s);
	size = s;
}


Blob& Blob::operator= (Blob& b)
{
	ptr = mem_resize(ptr, b.size);
	size = b.size;
	memcpy( ptr, b.ptr, b.size);
	return *this;
}
