// Filename    : OVQUEUE.H
// Description : header file of variable length queue


#ifndef __OVQUEUE_H
#define __OVQUEUE_H

class VLenQueue
{
public:
	char*	queue_buf;
	int	queue_buf_size;
	int	queued_size;
	char* queue_ptr;

public:
	VLenQueue();
	VLenQueue(int);
	VLenQueue(VLenQueue &);
	~VLenQueue();

	VLenQueue& operator= (VLenQueue &);
	void	clear();
	char* reserve( int );
	void	append_queue(VLenQueue &);
	void	swap(VLenQueue &);
	int	length();

private:
	void	expand( int );
};

#endif
