// Filename    : OLIGHTN2.CPP
// Description : class YLightning
// Ownership   : Gilbert

#include	<math.h>
#include <stdlib.h>
#include <ALL.H>
#include <OVGABUF.H>
#include <OLIGHTN.H>

//------------ Define constant ----------//

#define	PI 3.141592654

//--------- Begin of function YLightning::YLightning --------//
YLightning::YLightning()
{
	used_branch = 0;
	for( int i = 0; i < MAX_BRANCH; ++i)
	{
		branch[i] = 0;
	}
}
//--------- End of function YLightning::YLightning --------//

//--------- Begin of function YLightning::~YLightning --------//
YLightning::~YLightning()
{
	for( int i = 0; i < MAX_BRANCH; ++i)
	{
		delete branch[i];
		branch[i] = 0;
	}
}
//--------- End of function YLightning::~YLightning --------//

//--------- Begin of function YLightning::init --------//
void YLightning::init(double fromX, double fromY, double toX, double toY,
							 char energy)
{
	for( int i = 0; i < MAX_BRANCH; ++i)
	{
		delete branch[i];
		branch[i] = 0;
	}
	used_branch = 0;
	Lightning::init(fromX, fromY, toX, toY, energy);
	branch_prob = 1000 * MAX_BRANCH / expect_steps;
	if( branch_prob < 10 )
		branch_prob = 10;
	if( branch_prob > 300 )
		branch_prob = 300;
	branch_left = 0;
}
//--------- End of function YLightning::init --------//

//--------- Begin of function YLightning::move_particle --------//
void YLightning::move_particle()
{
	Lightning::move_particle();

	// determine if branching occurs
	if( (rand_seed() % 1000) <= branch_prob  && used_branch < MAX_BRANCH)
	{
		char branch_energy;
		if(energy_level > 4)
		{
			branch[used_branch] = new YLightning;
			branch_energy = 4;
		}
		else
		{
			branch[used_branch] = new Lightning;
			branch_energy = 1;
		}

		//------ determine new location ------//
		// angle : attraction angle + or - PI/8 to PI*3/8
		// distant : 1/2 to 3/4 from dist(destx-x, desty-y);
		double branchDist = Lightning::dist(destx-x, desty-y) 
			* ( 32 + rand_seed() % 16) / 64.0;
		double branchAngle= atan2(desty-y, destx-x) + 
			( 4 + rand_seed() % 8 ) * (branch_left ? PI / -32.0 : PI / 32.0);
		branch_left = !branch_left;

		branch[used_branch]->init(x, y, x+branchDist*cos(branchAngle),
			y+branchDist*sin(branchAngle), branch_energy);

		used_branch++;
	}
}
//--------- End of function YLightning::move_particle --------//

//--------- Begin of function YLightning::draw_step --------//
void YLightning::draw_step(VgaBuf *vgabuf)
{
	Lightning::draw_step(vgabuf);

	for( int i = 0; i < used_branch; ++i)
	{
		branch[i]->draw_step(vgabuf);
	}
}
//--------- End of function YLightning::draw_step --------//

//--------- Begin of function YLightning::draw_whole --------//
void YLightning::draw_whole(VgaBuf *vgabuf)
{
	while(!goal())
	{
		draw_step(vgabuf);
	}
}
//--------- End of function YLightning::draw_whole --------//
