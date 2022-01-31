// unit test template for POGO

// Caller must define GREP_STR and GREP_COUNT;
// GREP_STR
//    is the string to look for in the compiler
//    output, e.g., GREP_STR="val_l"
// GREP_COUNT
//    is the number of occurrences of that string
//    if all goes well, e.g., GREP_COUNT=42

// SAC_TEST|include common.mk

// Compile and execute unit test; save exit code
// SAC_TEST|SAC2C_FLAGS += -check c -dopogo -noggs -dolacsi -dowlf -nopwlf -noctz -norelcf -noainl -doplur
// SAC_TEST|SAC2C_BREAKPOINT := $(SAC2C_FLAGS) -bopt:wlbscnf2  -printfun main
// SAC_TEST|CALL_SAC2C := $(SAC2C) $(SAC2C_FLAGS)
// SAC_TEST|all: <file-name>
// SAC_TEST|<tab>@$(CALL_SAC2C) $< -o <file-name-we>
// SAC_TEST|<tab>$(CHECK_RETURN_STATUS) <file-name-we> 0
// Compile unit test with breakpoint
// SAC_TEST|<tab>@$(CALL_SAC2C) $(SAC2C_BREAKPOINT) $< | $(GREP_COMMAND_OUTPUT) $(GREP_STR) $(GREP_COUNT)

