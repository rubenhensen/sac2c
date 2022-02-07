# Tests explanation

The tests in this folder generally ensure that errors are thrown in a proper manner and that the correct output is displayed to the user.

Since our goal is to trigger errors at specific points, we can break the compiler early after a certain phase because either the error was thrown or the test failed because the error was not thrown. As an example, you may see the SAC flag `-b 1:prs` when an error is generated in the parsing steps.

To ensure that the messages are of the proper form, regexes are used with grep.
To keep the `GREP_COMMAND_OUTPUT` script simple, the decision was made to not add the ability to give it arbitrary arguments, but instead do regex matching as follows:

`SAC_TEST|<tab>$(CALL_SAC2C) $(SAC2C_FLAGS) $< 2>&1 | grep -Pa <perl_regex_pattern> | $(GREP_COMMAND_OUTPUT) "" <line_count>`

This redirects the stderr into the stdout so grep can match it. Stdout is fed into `grep -P <pattern>` to grab all lines that abide by the given regex pattern. The result of it is piped into `$(GREP_COMMAND_OUPUT) "" <line_count>` to count the number of lines we matched with the regex.

Multiline matches can't easily be achieved with this setup. It is preferred to avoid matching newlines if at all possible. If, however, it is absolutely necessary, this should work to match newlines:

`SAC_TEST|<tab>$(CALL_SAC2C) $(SAC2C_FLAGS) $< 2>&1 | grep -Pazo <perl_regex> | grep -Pazc "\n" | $(GREP_COMMAND_OUTPUT) <expected_number_of_matched_newlines> 1`

`P`: Enable perl regexes  
`a`: Force text formatting  
`z`: Treat input as a set of lines so we can match with `\n`  
`o`: Print only the parts that match  

Only matching with `grep -Pazc <perl_regex>` does not work for some reason, so they have to be chained.
