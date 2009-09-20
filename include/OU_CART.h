// Filename    : OU_CART.H
// Description : Header file for Explosive Cart

#ifndef __OU_CART_H
#define __OU_CART_H

#include <OUNIT.h>

class UnitExpCart : public Unit
{
public:
	char	triggered;

public:
	UnitExpCart();
	~UnitExpCart();

	int	process_die();
	void	trigger_explode();

	//-------------- multiplayer checking codes ---------------//
	virtual	UCHAR crc8();
	virtual	void	clear_ptr();
};

#endif