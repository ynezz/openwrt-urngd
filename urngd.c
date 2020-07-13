/*
 * Non-physical true random number generator based on timing jitter.
 *
 * Copyright Stephan Mueller <smueller@chronox.de>, 2014
 * Copyright Petr Štetiar <ynezz@true.cz>, 2019
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, and the entire permission notice in its entirety,
 *    including the disclaimer of warranties.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * ALTERNATIVELY, this product may be distributed under the terms of
 * the GNU General Public License, in which case the provisions of the GPL are
 * required INSTEAD OF the above restrictions.  (This clause is
 * necessary due to a potential bad interaction between the GPL and
 * the restrictions contained in a BSD-style copyright.)
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, ALL OF
 * WHICH ARE HEREBY DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF NOT ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <linux/random.h>

#include <libubox/uloop.h>

#include "log.h"
#include "jitterentropy.h"

#define ENTROPYBYTES 32
#define OVERSAMPLINGFACTOR 2
#define DEV_RANDOM "/dev/random"
#define ENTROPYPOOLBYTES \
		(ENTROPYBYTES * OVERSAMPLINGFACTOR * sizeof(char))

#ifdef URNGD_DEBUG
unsigned int debug;
#endif

struct urngd {
	struct uloop_fd rnd_fd;
	struct rand_data *ec;
};

static struct urngd urngd_service;

static inline void memset_secure(void *s, int c, size_t n)
{
	memset(s, c, n);
	__asm__ __volatile__("" : : "r" (s) : "memory");
}

static size_t write_entropy(struct urngd *u, struct rand_pool_info *rpi)
{
	int ret;
	ret =  ioctl(u->rnd_fd.fd, RNDADDENTROPY, rpi);
	if (0 > ret) {
		ERROR("error injecting entropy: %s\n", strerror(errno));
		return 0;
	} else {
		DEBUG(1, "injected %ub (%ub of entropy)\n",
			rpi->buf_size, rpi->entropy_count/8);
		ret = rpi->buf_size;
	}

	return ret;
}

static size_t gather_entropy(struct urngd *u)
{
	ssize_t ent;
	size_t ret = 0;
	struct rand_pool_info *rpi = alloca(sizeof(*rpi) + ENTROPYPOOLBYTES);

	ent = jent_read_entropy(u->ec, (char *)&rpi->buf[0], ENTROPYPOOLBYTES);
	if (ent < 0) {
		ERROR("cannot read entropy\n");
		return 0;
	}

	rpi->buf_size = ENTROPYPOOLBYTES;
	rpi->entropy_count = 8 * ENTROPYBYTES;

	ret = write_entropy(u, rpi);

	memset_secure(&rpi->buf, 0, ENTROPYPOOLBYTES);

	return ret;
}

static void low_entropy_cb(struct uloop_fd *ufd, unsigned int events)
{
	struct urngd *u = container_of(ufd, struct urngd, rnd_fd);

	DEBUG(2, DEV_RANDOM " signals low entropy\n");
	gather_entropy(u);
}

static void urngd_done(struct urngd *u)
{
	if (u->ec) {
		jent_entropy_collector_free(u->ec);
		u->ec = NULL;
	}

	if (u->rnd_fd.fd) {
		close(u->rnd_fd.fd);
		u->rnd_fd.fd = 0;
	}
}

static bool urngd_init(struct urngd *u)
{
	int ret = jent_entropy_init();
	if (ret) {
		ERROR("jent-rng init failed, err: %d\n", ret);
		return false;
	}

	u->ec = jent_entropy_collector_alloc(1, 0);
	if (!u->ec) {
		ERROR("jent-rng alloc failed\n");
		return false;
	}

	u->rnd_fd.cb = low_entropy_cb;
	u->rnd_fd.fd = open(DEV_RANDOM, O_WRONLY);
	if (u->rnd_fd.fd < 1) {
		ERROR(DEV_RANDOM " open failed: %s\n", strerror(errno));
		return false;
	}

	uloop_fd_add(&u->rnd_fd, ULOOP_WRITE);

	return true;
}

static int usage(const char *prog)
{
	fprintf(stderr, "Usage: %s [options]\n"
		"Options:\n"
#ifdef URNGD_DEBUG
		"	-d <level>	Enable debug messages\n"
#endif
		"	-S		Print messages to stdout\n"
		"\n", prog);
	return 1;
}

int main(int argc, char **argv)
{
	int ch;
	int ulog_channels = ULOG_KMSG;
#ifdef URNGD_DEBUG
	char *dbglvl = getenv("DBGLVL");

	if (dbglvl) {
		debug = atoi(dbglvl);
		unsetenv("DBGLVL");
	}
#endif

	while ((ch = getopt(argc, argv, "d:S")) != -1) {
		switch (ch) {
#ifdef URNGD_DEBUG
		case 'd':
			debug = atoi(optarg);
			break;
#endif
		case 'S':
			ulog_channels = ULOG_STDIO;
			break;
		default:
			return usage(argv[0]);
		}
	}

	ulog_open(ulog_channels, LOG_DAEMON, "urngd");

	uloop_init();
	if (!urngd_init(&urngd_service)) {
		uloop_done();
		return -1;
	}

	LOG("v%s started.\n", URNGD_VERSION);

	gather_entropy(&urngd_service);

	uloop_run();
	uloop_done();

	urngd_done(&urngd_service);

	return 0;
}
