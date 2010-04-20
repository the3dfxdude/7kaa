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

// Filename   : OPLASMA.CPP
// Description: plasma map generator
// Ownership  : Gilbert

#include <stdlib.h>
#include <ALL.h>
#include <OCONFIG.h>
#include <OPLASMA.h>

// ---------- define constant ----------//

enum { SHIFT_VALUE=18,     // shift based on no. of colors
		 MAX_COLOR=256       // MAX. no. of colors
	  };


// --------- Begin of function Plasma::Plasma -----------//
Plasma::Plasma()
{
	max_x = max_y = 0;
	matrix = NULL;
}
// --------- End of function Plasma::Plasma -----------//


// --------- Begin of function Plasma::~Plasma -----------//
Plasma::~Plasma()
{
	deinit();
}
// --------- End of function Plasma::~Plasma -----------//


// --------- Begin of function Plasma::init -----------//
void Plasma::init(short x, short y)
{
	deinit();
	max_x = x;
	max_y = y;
	matrix = (short *) mem_add( (x+1)*(y+1)*sizeof(short) );
	memset( matrix, 0, (x+1)*(y+1)*sizeof(short) );
}
// --------- End of function Plasma::init -----------//


// --------- Begin of function Plasma::deinit -----------//
void Plasma::deinit()
{
	if( matrix)
		mem_del(matrix);
	matrix = NULL;
	max_x = max_y = 0;
}
// --------- End of function Plasma::deinit -----------//


// --------- Begin of function Plasma::get_pix -----------//
short Plasma::get_pix(short x, short y)
{
	return matrix[y * (max_x+1) + x];
}
// --------- End of function Plasma::get_pix -----------//


// --------- Begin of function Plasma::plot -----------//
void Plasma::plot(short x, short y, short value)
{
	matrix[y * (max_x+1) + x] = value;
}
// --------- End of function Plasma::plot -----------//


// --------- Begin of function Plasma::generate -----------//
void Plasma::generate(int genMethod, int grainFactor, int randomSeed)
{
   int i,k, n;
   U16 rnd[4];

   iparmx = grainFactor * 8;

   srand(randomSeed);

   for(n = 0; n < 4; n++)
      rnd[n] = 1+(((m.rand()/MAX_COLOR)*(MAX_COLOR-1))>>(SHIFT_VALUE-11));

   plot(    0,     0, rnd[0]);
   plot(max_x,     0, rnd[1]);
   plot(max_x, max_y, rnd[2]);
   plot(    0, max_y, rnd[3]);

   recur_level = 0;

   if ( genMethod == 0)         // use original method
   {
      sub_divide(0,0,max_x,max_y);
   }
   else         // use new method
   {
      recur1 = i = k = 1;
      while(new_sub_divide(0,0,max_x,max_y,i)==0)
      {
         k = k * 2;
         if (k  > max_x && k > max_y)
            break;
         i++;
		}
   }
}
// --------- End of function Plasma::generate -----------//


// --------- Begin of function Plasma::sub_divide -----------//
void Plasma::sub_divide(int x1,int y1,int x2,int y2)
{
   int x,y;
   S32 v,i;

   if(x2-x1<2 && y2-y1<2)
      return;

   recur_level++;
   recur1 = 320L >> recur_level;

   x = (x1+x2)>>1;
   y = (y1+y2)>>1;

   if((v=get_pix(x,y1)) == 0)
      v=adjust(x1,y1,x ,y1,x2,y1);
   i=v;

   if((v=get_pix(x2,y)) == 0)
      v=adjust(x2,y1,x2,y ,x2,y2);
   i+=v;

   if((v=get_pix(x,y2)) == 0)
      v=adjust(x1,y2,x ,y2,x2,y2);
   i+=v;

   if((v=get_pix(x1,y)) == 0)
      v=adjust(x1,y1,x1,y ,x1,y2);
   i+=v;

   if(get_pix(x,y) == 0)
      plot(x,y,(U16)((i+2)>>2));

   sub_divide(x1,y1,x ,y);
   sub_divide(x ,y1,x2,y);
   sub_divide(x ,y ,x2,y2);
	sub_divide(x1,y ,x ,y2);
   recur_level--;
}
// --------- End of function Plasma::sub_divide -----------//


// --------- Begin of function Plasma::new_sub_divide -----------//
int Plasma::new_sub_divide(int x1,int y1,int x2,int y2, int recur)
{
   int x,y;
   int nx1;
   int nx;
   int ny1, ny;
   S32 i, v;

   struct sub
   {
      BYTE t;      // top of stack
      int  v[16];  // subdivided value
      BYTE r[16];  // recursion level
   };

   static struct sub subx, suby;

   /*
   recur1=1;
   for (i=1;i<=recur;i++)
      recur1 = recur1 * 2;
   recur1=320/recur1;
   */

   recur1 = 320L >> recur;
   suby.t = 2;
   ny     = suby.v[0] = y2;
   ny1    = suby.v[2] = y1;
   suby.r[0] = suby.r[2] = 0;
   suby.r[1] = 1;
   y      = suby.v[1] = (ny1 + ny) >> 1;

   while (suby.t >= 1)
   {
      while (suby.r[suby.t-1] < recur)
      {
			/*     1.  Create new entry at top of the stack  */
         /*     2.  Copy old top value to new top value.  */
         /*            This is largest y value.           */
         /*     3.  Smallest y is now old mid point       */
         /*     4.  Set new mid point recursion level     */
         /*     5.  New mid point value is average        */
         /*            of largest and smallest            */

         suby.t++;
         ny1  = suby.v[suby.t] = suby.v[suby.t-1];
         ny   = suby.v[suby.t-2];
         suby.r[suby.t] = suby.r[suby.t-1];
         y    = suby.v[suby.t-1]   = (ny1 + ny) >> 1;
         suby.r[suby.t-1]   = (int)MAX(suby.r[suby.t], suby.r[suby.t-2])+1;
      }

      subx.t = 2;
      nx  = subx.v[0] = x2;
      nx1 = subx.v[2] = x1;
      subx.r[0] = subx.r[2] = 0;
      subx.r[1] = 1;
      x = subx.v[1] = (nx1 + nx) >> 1;

      while (subx.t >= 1)
      {
         while (subx.r[subx.t-1] < recur)
         {
            subx.t++; /* move the top ofthe stack up 1 */
            nx1  = subx.v[subx.t] = subx.v[subx.t-1];
            nx   = subx.v[subx.t-2];
            subx.r[subx.t] = subx.r[subx.t-1];
            x    = subx.v[subx.t-1]   = (nx1 + nx) >> 1;
            subx.r[subx.t-1]   = (int)MAX(subx.r[subx.t],
                subx.r[subx.t-2])+1;
         }

         if ((i = get_pix(nx, y)) == 0)
            i = adjust(nx,ny1,nx,y ,nx,ny);

         v = i;

         if ((i = get_pix(x, ny)) == 0)
            i = adjust(nx1,ny,x ,ny,nx,ny);

			v += i;

         if(get_pix(x,y) == 0)
         {
            if ((i = get_pix(x, ny1)) == 0)
               i = adjust(nx1,ny1,x ,ny1,nx,ny1);
            v += i;
            if ((i = get_pix(nx1, y)) == 0)
               i = adjust(nx1,ny1,nx1,y ,nx1,ny);
            v += i;
            plot(x,y,(U16)((v + 2) >> 2));
         }

         if (subx.r[subx.t-1] == recur) subx.t = subx.t - 2;
      }

      if (suby.r[suby.t-1] == recur) suby.t = suby.t - 2;
   }

   return(0);
}
// --------- End of function Plasma::new_sub_divide -----------//


// --------- Begin of function Plasma::adjust -----------//
U16 Plasma::adjust(int xa,int ya,int x,int y,int xb,int yb)
{
   S32 pseudoRandom;

   pseudoRandom = ((S32)iparmx)*((m.rand()-16383));
//   pseudoRandom = pseudoRandom*(abs(xa-xb)+abs(ya-yb));

   pseudoRandom = pseudoRandom * recur1;
   pseudoRandom = pseudoRandom >> SHIFT_VALUE;
   pseudoRandom = (((S32)get_pix(xa,ya)+(S32)get_pix(xb,yb)+1)>>1)+pseudoRandom;

   if (pseudoRandom >= MAX_COLOR)
      pseudoRandom = MAX_COLOR-1;

   if(pseudoRandom < 1)
      pseudoRandom = 1;

   plot(x,y,(U16)pseudoRandom);
	return((U16)pseudoRandom);
}
// --------- End of function Plasma::adjust -----------//


// --------- Begin of function Plasma::add_base_level -----------//
void Plasma::add_base_level(short baseLevel)
{
	//---------- adjust map baseLevel -----------//

	int		i;
	int		totalLoc=(max_x +1) * (max_y +1);
	short		*locPtr;
	err_when( max_x == 0 || max_y == 0 || matrix == NULL);

	for( i=0, locPtr=matrix ; i<totalLoc ; i++, locPtr++ )
	{
		*locPtr += baseLevel;
		if(*locPtr < 1)
			*locPtr = 1;
		if(*locPtr >= MAX_COLOR)
			*locPtr = MAX_COLOR-1;
	}
}
// --------- End of function Plasma::add_base_level -----------//


// --------- Begin of function Plasma::calc_tera_base_level -----------//
int Plasma::calc_tera_base_level(short minHeight)
{
   //------- count the percentage of sea and land -------//

   int totalLoc=(max_x+1) * (max_y+1);
	int i, locHeight, landCount=0, baseLevel=0;

   for( i=0 ; i<totalLoc ; i++ )
   {
		if( matrix[i] >= minHeight )
         landCount++;
   }

   // ensure that percentage of land won't be less than 1/3

   if( landCount < 2*totalLoc/3 )   // if land is less than 2/3 of the map
      baseLevel = 50L * (totalLoc-landCount) / totalLoc;

   //-------- ensure availability of sea in the map -----------//

   int xLoc, yLoc, seaCount=0, lowestGrassHeight=0xFFFF;
	err_when(max_x == 0 || max_y == 0);

   //---------- scan top & bottom side -----------//

	for( xLoc=0 ; xLoc<=max_x ; xLoc++ )
	{
		//----------- top side ------------//

		locHeight = get_pix(xLoc,0) + baseLevel;

		if( locHeight < minHeight )        // it's a sea
         seaCount++;
      else
		{
         if( locHeight < lowestGrassHeight )
            lowestGrassHeight = locHeight;
      }

      //--------- bottom side -----------//

		locHeight = get_pix(xLoc,max_y) + baseLevel;

		if( locHeight < minHeight )        // it's a sea
         seaCount++;
		else
		{
         if( locHeight < lowestGrassHeight )
            lowestGrassHeight = locHeight;
      }
   }

   //---------- scan left & right side -----------//

   for( yLoc=0 ; yLoc<=max_y ; yLoc++ )
	{
      //----------- top side ------------//

		locHeight = get_pix(0,yLoc) + baseLevel;

		if( locHeight < minHeight)        // it's a sea
         seaCount++;
      else
      {
         if( locHeight < lowestGrassHeight )
            lowestGrassHeight = locHeight;
      }

      //--------- bottom side -----------//

		locHeight = get_pix(max_x,yLoc) + baseLevel;

		if( locHeight < minHeight )        // it's a sea
			seaCount++;
		else
		{
			if( locHeight < lowestGrassHeight )
				lowestGrassHeight = locHeight;
		}
	}


	//------- If there is not enough sea --------//

	static short min_sea_count_array[] = { 1600, 700, 200 };

	int minSeaCount = min_sea_count_array[config.land_mass-OPTION_LOW];

	if( seaCount < minSeaCount )
		baseLevel -= lowestGrassHeight - minHeight + (minSeaCount-seaCount) / 15;

	return baseLevel;
}
// --------- End of function Plasma::calc_tera_base_level -----------//


// --------- Begin of function Plasma::stat -----------//
//
// classify heights into groups and count the frequency of
// each group
//
// <int>	groups					(input) no. of groups
// <short *> minHeights			(input) lower limit of each group, must be strictly increasing
// <int *> freq					(output) frequency of each group
// <int> return value			total frequency
//
int Plasma::stat(int groups, short *minHeights, int *freq)
{
	int total = (max_x+1)*(max_y+1);
	int g;
	
	// -------- initialize freq count to zero ------//
	for( g = groups-1; g >= 0; --g)
		freq[g] = 0;

	// -------- classify and increase counter -------//
	for( int i = 0; i < total; ++i)
	{
		short v = matrix[i];
		for( g = groups-1; g >= 0; --g)
		{
			if( v >= minHeights[g])
			{
				freq[g]++;
				break;
			}
		}
	}
	return total;
}
// --------- End of function Plasma::stat -----------//


// --------- Begin of function Plasma::generate2 -----------//
//
// not initialize the corners, so some points should be
// initialized before calling this function
//
void Plasma::generate2(int genMethod, int grainFactor, int randomSeed)
{
   int i,k;

   iparmx = grainFactor * 8;

   srand(randomSeed);

   recur_level = 0;

   if ( genMethod == 0)         // use original method
   {
      sub_divide(0,0,max_x,max_y);
   }
   else         // use new method
   {
      recur1 = i = k = 1;
      while(new_sub_divide(0,0,max_x,max_y,i)==0)
      {
         k = k * 2;
         if (k  > max_x && k > max_y)
            break;
         i++;
		}
   }
}
// --------- End of function Plasma::generate2 -----------//


// --------- begin of function Plasma::shuffle_level --------//
// change height of a point, if this point and 8 point around it is within
// minHeight and maxHeight
void Plasma::shuffle_level(short minHeight, short maxHeight, short amplitude)
{
	// don't change boundary points
	int x,y;
	err_when( maxHeight - minHeight < 5 );
	for( y = 1; y < max_y; ++y)
	{
		for( x = 1; x < max_x; ++x)
		{
			short h;
			if( (h = get_pix(x,y)) >= minHeight && h <= maxHeight 
				&& (h = get_pix(x-1,y-1)) >= minHeight && h <= maxHeight
				&& (h = get_pix(x,y-1)) >= minHeight && h <= maxHeight
				&& (h = get_pix(x+1,y-1)) >= minHeight && h <= maxHeight
				&& (h = get_pix(x-1,y)) >= minHeight && h <= maxHeight
				&& (h = get_pix(x+1,y)) >= minHeight && h <= maxHeight
				&& (h = get_pix(x-1,y+1)) >= minHeight && h <= maxHeight
				&& (h = get_pix(x,y+1)) >= minHeight && h <= maxHeight
				&& (h = get_pix(x+1,y+1)) >= minHeight && h <= maxHeight
				)
			{
				h = get_pix(x,y);

				// method 1 - random amplitude
				//h += m.random(amplitude*2+1) - amplitude;
				//if( h > maxHeight )
				//	h = maxHeight;
				//if( h < minHeight )
				//	h = minHeight;

				// method 2 - folding (amplitude : +1, +3, +5... or -1, -3, -5...)
				if( amplitude >= 0)
					h = minHeight + (h - minHeight) * amplitude;
				else
					h = maxHeight + (maxHeight - h) * amplitude;
				int loopCount = 20;
				while( h < minHeight || h > maxHeight )
				{
					err_when( --loopCount <= 0 );
					if( h > maxHeight )
					{
						h = maxHeight - (h - maxHeight);
					}
					else if( h < minHeight )
					{
						h = minHeight + (minHeight - h);
					}
				}

				plot(x, y, h);
			}
		}
	}
}
// --------- end of function Plasma::shuffle_level --------//