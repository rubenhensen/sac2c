# This Makefile is a wrapper around cmake that allows to quickly
# make default build and install of product and development versions
# of the compiler.

BUILD_DIR_DEBUG  := build_d
BUILD_DIR_RELEASE := build_r
MAKE_NUMTHREADS ?= 4

define BUILD =
  if [ -d $(1) ]; then \
    cd $(1); \
    $(MAKE) -j$(MAKE_NUMTHREADS);
    cd -; \
  else \
    mkdir -p $(1); \
    cd $(1); \
    cmake -DCMAKE_BUILD_TYPE=$(2) ..; \
    $(MAKE) -j$(MAKE_NUMTHREADS); \
    cd -; \
  fi
endef

.PHONY: all release debug clean clean-release clean-debug install install-release install-debug

all: release debug

release:
	$(call BUILD,$(BUILD_DIR_RELEASE),RELEASE)

debug:
	$(call BUILD,$(BUILD_DIR_DEBUG),DEBUG)

clean: clean-release clean-debug

clean-release:
	$(RM) -rf $(BUILD_DIR_RELEASE)

clean-debug:
	$(RM) -rf  $(BUILD_DIR_DEBUG)

install: install-debug install-release

install-release: release
	cd $(BUILD_DIR_RELEASE); make install; cd -
	@echo "Please run $(BUILD_DIR_RELEASE)/scripts/sac2c-version-manager " \
              "now to set symlinks correctly."

install-debug: debug
	cd $(BUILD_DIR_DEBUG); make install; cd -
	@echo "Please run $(BUILD_DIR_DEBUG)/scripts/sac2c-version-manager " \
              "now to set symlinks correctly."


