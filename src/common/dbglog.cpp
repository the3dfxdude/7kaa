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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <dbglog.h>

#define DEBUG_LOG_MAX_CHANNELS 256

static struct __dbglog_channel dbglog_settings[DEBUG_LOG_MAX_CHANNELS];

static int debug_log_initialized = 0;

int dbglog_init()
{
	const char *debug_env_var;
	int i, j, len;

	if (debug_log_initialized) return 1;

	debug_env_var = getenv("SKDEBUG");
	if (debug_env_var)
		len = strlen(debug_env_var);
	else
		len = 0;

	for (i = 0, j = 0; i < DEBUG_LOG_MAX_CHANNELS; i++)
	{
		int copied = 0;

		while (j < len && copied < 30)
		{
			if (debug_env_var[j] == '\0' ||
			    debug_env_var[j] == ',')
			{
				j++;
				break;
			}
			dbglog_settings[i].name[copied] = debug_env_var[j];
			j++;
			copied++;
		}

		dbglog_settings[i].name[copied] = '\0';

		if (copied > 0)
		{
			dbglog_settings[i].flags = DBGLOG_ERR_ON|DBGLOG_MSG_ON;
		} else {
			dbglog_settings[i].flags = DBGLOG_NEED_INIT;
		}
	}

	debug_log_initialized = 1;

	return 1;
}

unsigned char dbglog_get_flags(char *name)
{
	int i;

	if (!debug_log_initialized && !dbglog_init())
		return DBGLOG_NEED_INIT;

	for (i = 0; i < DEBUG_LOG_MAX_CHANNELS; i++)
	{
		if (strncmp(name, dbglog_settings[i].name, 32)==0)
		{
			return dbglog_settings[i].flags;
		}
		if (dbglog_settings[i].flags == DBGLOG_NEED_INIT)
		{
			strncpy(dbglog_settings[i].name, name, 32);
			dbglog_settings[i].flags = DBGLOG_ERR_ON;
			return dbglog_settings[i].flags;
		}
	}
	return DBGLOG_NEED_INIT;
}

void dbglog_printf(enum __dbglog_class msg_class, struct __dbglog_channel *c, const char *format, ...)
{
	va_list valist;

	if (c->flags == DBGLOG_NEED_INIT)
	{
		c->flags = dbglog_get_flags(c->name);
		if (c->flags == DBGLOG_NEED_INIT) return;
	}

	if (c->flags & DBGLOG_ERR_ON && msg_class == __DBGLOG_ERR)
	{
		printf("err:%s: ", c->name);
		va_start(valist, format);
		vprintf(format, valist);
		va_end(valist);
	}

	if (c->flags & DBGLOG_MSG_ON && msg_class == __DBGLOG_MSG)
	{
		printf("msg:%s: ", c->name);
		va_start(valist, format);
		vprintf(format, valist);
		va_end(valist);
	}
}
