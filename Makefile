# This Makefile is a wrapper around cmake that allows to quickly
# make default build and install of product and development versions
# of the compiler.

BUILD_DIR_DEVEL  := build_d
BUILD_DIR_RELEASE := build_r
MAKE_NUMTHREADS ?= 4

all: release devel

release:
	if [ -d $(BUILD_DIR_RELEASE) ]; then \
          cd $(BUILD_DIR_RELEASE); make -j$(MAKE_NUMTHREADS); cd -; \
        else \
          mkdir -p $(BUILD_DIR_RELEASE); \
          cd $(BUILD_DIR_RELEASE); \
          cmake -DCMAKE_BUILD_TYPE=RELEASE ..; \
          make -j$(MAKE_NUMTHREADS); \
          cd -; \
        fi

devel:
	if [ -d $(BUILD_DIR_DEVEL) ]; then \
          cd $(BUILD_DIR_DEVEL); make -j$(MAKE_NUMTHREADS); cd -; \
        else \
          mkdir -p $(BUILD_DIR_DEVEL); \
          cd $(BUILD_DIR_DEVEL); \
          cmake -DCMAKE_BUILD_TYPE=DEBUG ..; \
          make -j$(MAKE_NUMTHREADS); \
          cd -; \
        fi

clean: clean-release clean-devel

clean-release:
	$(RM) -rf $(BUILD_DIR_RELEASE)

clean-devel:
	$(RM) -rf  $(BUILD_DIR_DEVEL)

install: install-devel install-release

install-release: release
	cd $(BUILD_DIR_RELEASE); make install; cd -
	@echo "Please run $(BUILD_DIR_RELEASE)/scripts/sac2c-version-manager " \
              "now to set symlinks correctly."

install-devel: devel
	cd $(BUILD_DIR_DEVEL); make install; cd -
	@echo "Please run $(BUILD_DIR_DEVEL)/scripts/sac2c-version-manager " \
              "now to set symlinks correctly."


