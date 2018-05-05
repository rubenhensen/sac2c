# This Makefile is a wrapper around cmake that allows to quickly
# make default build and install of product and development versions
# of the compiler.

BUILD_DIR_DEVEL  := build_d
BUILD_DIR_RELEASE := build_r
MAKE_NUMTHREADS ?= 4

define BUILD =
  if [ -d $(1) ]; then \
    cd $(1); make -j$(MAKE_NUMTHREADS); cd -; \
  else \
    mkdir -p $(1); \
    cd $(1); \
    cmake -DCMAKE_BUILD_TYPE=$(2) ..; \
    make -j$(MAKE_NUMTHREADS); \
    cd -; \
  fi
endef



all: release devel

release:
	$(call BUILD,$(BUILD_DIR_RELEASE),RELEASE)

devel:
	$(call BUILD,$(BUILD_DIR_DEVEL),DEBUG)

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


