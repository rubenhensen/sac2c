#!/usr/bin/osascript
set vol to POSIX file "/Volumes/@CPACK_BUNDLE_NAME@/usr"
set root to POSIX file "/"

tell application "Finder"
    display dialog "Install SAC @SAC2C_VERSION@?\n\nThis will copy all files from the DMG to '/' while maintaining the directory hierarchy." buttons {"Cancel", "Install"} default button 2 with icon file "Volumes:@CPACK_BUNDLE_NAME@:@CPACK_BUNDLE_NAME@.app:Contents:Resources:@CPACK_BUNDLE_NAME@.icns"
    set rbutton to button returned of result
    -- If the user doesnt want to install then we close
    if rbutton = "Install" then
        try
            do shell script "cp -r " & quoted form of POSIX path of vol & " " & quoted form of POSIX path of root with administrator privileges
            on error emsg number eno
            display alert "Install failed with error code '" & eno & "' and message:\n\n" & emsg
            return
        end try
            display alert "Install was succesful!"
    end if
end tell
