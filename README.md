μrngd
=====

μrngd is OpenWrt's micro non-physical true random number generator based on
timing jitter.

Using the Jitter RNG core, the rngd provides an entropy source that feeds into
the Linux /dev/random device if its entropy runs low. It updates the /dev/random
entropy estimator such that the newly provided entropy unblocks /dev/random.

The seeding of /dev/random also ensures that /dev/urandom benefits from entropy.
Especially during boot time, when the entropy of Linux is low, the Jitter RNGd
provides a source of sufficient entropy.
