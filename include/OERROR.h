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

//Filename    : OERROR.h
//Description : Header file for the Error class

#ifndef __OERROR_H
#define __OERROR_H

//------- Define Class Error ------------//

typedef void (*ExtraHandler)();

class Error
{
private:
   ExtraHandler extra_handler;          // extra error handler

public:
   Error();

   void internal(char*,const char*,int);
   void mem();
        void msg(const char*,...);
   void run(const char*,...);

   void set_extra_handler(ExtraHandler extraHandler) { extra_handler = extraHandler; }
};

extern Error err;

//-------- error handling functions ----------//

#ifdef DEBUG
   #define err_when(cond)   if(cond) err.internal(NULL,__FILE__, __LINE__)
   #define err_here()       err.internal(NULL,__FILE__, __LINE__)
   #define err_if(cond)     if(cond)
   #define err_else         else
   #define err_now(msg)     err.run(msg)        // internal error

   // always use err_if(cond) together with err_now(), so when debug is turn off, these two statement will turn off
#else
   #define err_when(cond)
   #define err_here()
   #define err_if(cond)
   #define err_else
   #define err_now(msg)
#endif

#endif // __OERROR_H
