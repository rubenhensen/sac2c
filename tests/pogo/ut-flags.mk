// TEMPORARY LOCATION UNTIL ARTEM GETS SUBDIR SUPPORT WORKING.
//  THIS IS FOR POGO ONLY... other subdirs need different flags

//  gitlab unit test flag definitions for POGO

// Caller must define GREP_STR and GREP_COUNT;
// GREP_STR is the string to look for in the compiler output,
// e.g., GREP_STR="val_l"
// GREP_COUNT is the number of occurrences of that string if all goes well,
// e.g., GREP_COUNT=42
//echo GREP_STR is $GREP_STR and GREP_COUNT is $GREP_COUNT

// SAC_TEST|include common.mk
// Compile and execute unit test; save exit code
// SAC_TEST|SAC2C_FLAGS += -check c -dopogo -noggs -dolacsi -dowlf -nopwlf -noctz -norelcf -noainl -doplur
// SAC_TEST|SAC2C_BREAKPOINT := $(SAC2C_FLAGS) -bopt:wlbscnf2  -printfun main
// SAC_TEST|CALL_SAC2C := $(SAC2C) $(SAC2C_FLAGS)
// SAC_TEST|ifeq ($(shell ./scripts/isalloptionson.sh $(SAC2C) "ISL" "BARVINOK"),yes)
// SAC_TEST|all: <file-name>
// SAC_TEST|<tab>@$(SAC2C) $(SAC2C_FLAGS) $< -o <file-name-we>
// SAC_TEST|<tab>$(CHECK_RETURN_STATUS) <file-name-we> 0
// Compile unit test with breakpoint
// SAC_TEST|<tab>@$(CALL_SAC2C) $(SAC2C_BREAKPOINT) $< | $(GREP_COMMAND_OUTPUT) $(GREP_STR) $(GREP_COUNT)
// SAC_TEST|else
// SAC_TEST|all:
// SAC_TEST|<tab>echo "test skipped. Unsupported sac2c configuration"
// SAC_TEST|endif

