# This Makefile is a wrapper around cmake that allows to quickly
# make default build and install of product and development versions
# of the compiler.

BUILD_DIR_DEVEL  := build_d
BUILD_DIR_PROD := build_p
MAKE_NUMTHREAD ?= 4

all: prod devel

prod:
	if [ -d $(BUILD_DIR_PROD) ]; then \
          cd $(BUILD_DIR_PROD); make -j$(MAKE_NUMTHREADS); cd -; \
        else \
          mkdir -p $(BUILD_DIR_PROD); \
          cd $(BUILD_DIR_PROD); \
          cmake -DCMAKE_BUILD_TYPE=RELEASE ..; \
          make -j$(NUMTHREADS); \
          cd -; \
        fi

devel:
	if [ -d $(BUILD_DIR_DEVEL) ]; then \
          cd $(BUILD_DIR_DEVEL); make -j$(MAKE_NUMTHREADS); cd -; \
        else \
          mkdir -p $(BUILD_DIR_DEVEL); \
          cd $(BUILD_DIR_DEVEL); \
          cmake -DCMAKE_BUILD_TYPE=DEBUG ..; \
          make -j$(NUMTHREADS); \
          cd -; \
        fi

clean: clean-prod clean-devel

clean-prod:
	$(RM) -rf $(BUILD_DIR_PROD)

clean-devel:
	$(RM) -rf  $(BUILD_DIR_DEVEL)

install: install-dev install-prod

install-prod: prod
	cd $(BUILD_DIR_PROD); make install; cd -
	@echo "Please run $(BUILD_DIR_PROD)/scripts/sac2c-version-manager " \
              "now to set symlinks correctly."

install-devel: devel
	cd $(BUILD_DIR_DEVEL); make install; cd -
	@echo "Please run $(BUILD_DIR_DEVEL)/scripts/sac2c-version-manager " \
              "now to set symlinks correctly."


