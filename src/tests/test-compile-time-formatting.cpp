#include <sys/types.h>
#include <dirent.h>
#include "gtest/gtest.h"

extern "C" {
#define DBUG_PREFIX "TEST-CTF"
#include "debug.h"
#include "memory.h"
#include "str.h"
#include "str_buffer.h"
#include "ctformatting.h"
}

TEST (CTF, testCreateMessageBegin)
{
    // Empty for now
}
