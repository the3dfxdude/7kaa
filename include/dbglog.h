/*
 * Seven Kingdoms: Ancient Adversaries
 *
 * Copyright 2010 Jesse Allen
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

#ifndef _DBGLOG_H
#define _DBGLOG_H

#define DBGLOG_ALL_OFF   0x00
#define DBGLOG_ERR_ON    0x01
#define DBGLOG_MSG_ON    0x02
#define DBGLOG_NEED_INIT 0xff

enum __dbglog_class
{
	__DBGLOG_MSG,
	__DBGLOG_ERR
};

struct __dbglog_channel
{
	unsigned char flags;
	char name[32];
};

extern void dbglog_printf(enum __dbglog_class msg_class, struct __dbglog_channel *c, char *format, ...);

#ifdef DEBUG

#define MSG(...) dbglog_printf(__DBGLOG_MSG, &__dbglog_default_channel, __VA_ARGS__)
#define ERR(...) dbglog_printf(__DBGLOG_ERR, &__dbglog_default_channel, __VA_ARGS__)

#define DBGLOG_DEFAULT_CHANNEL(s) static struct __dbglog_channel __dbglog_default_channel = { DBGLOG_NEED_INIT, #s }

#else /* !DEBUG */

#define ERR(...)
#define MSG(...)

#define DBGLOG_DEFAULT_CHANNEL(s)

#endif /* DEBUG */

#endif /* _DBGLOG_H */
