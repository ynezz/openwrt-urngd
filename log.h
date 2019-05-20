/*
 * Copyright (C) 2013 Felix Fietkau <nbd@openwrt.org>
 * Copyright (C) 2013 John Crispin <blogic@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 2.1
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __LOG_H
#define __LOG_H

#include <libubox/ulog.h>

#ifdef URNGD_DEBUG
#define DEBUG(level, fmt, ...) do { \
	if (debug >= level) { \
		ulog(LOG_DEBUG, fmt, ## __VA_ARGS__); \
	} } while (0)
#else
#define DEBUG(level, fmt, ...)
#endif

#define LOG   ULOG_INFO
#define ERROR ULOG_ERR

extern unsigned int debug;

#endif
