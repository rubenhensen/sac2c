These scripts have to be called from the sac2c directory.
They are shortcuts to build the following versions of sac2c:
 - make_distmem: distributed memory backend version
 - make_distmemcheck: distributed memory backend version with runtime checks
 - make_all: classic sac2c, distmem and distmemcheck
 - make_all_das: same as make_all, but uses " -j 8 ", for use on fast enough build servers like on the DAS-4
