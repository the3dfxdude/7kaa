// Filename    : OBLOB.H
// Description : header file for fixed size binary block
// Owner       : Gilbert

#ifndef __OBLOB_H
#define __OBLOB_H

class Blob
{
public:
	char *ptr;				// more exactly char const *ptr
	int	size;				// const int

public:
	Blob();
	Blob(int);
	Blob( Blob &);
	~Blob();

	void	resize(int);
	Blob& operator=(Blob &);

	char *p()			{ return ptr; }
};


#endif
