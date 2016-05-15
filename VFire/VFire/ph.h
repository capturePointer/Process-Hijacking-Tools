#pragma once
#include "stdafx.h"

#ifndef PROC_HOLLOW_H
#define PROC_HOLLOW_H

#include <stddef.h>
#include <tchar.h>

/* call this first to initialize the pointers before
* you call create_hollowed_proc
*/
int ph_init(void);

/*
* NOTE: src isn't a pointer to const since the contents
* of src CAN BE modified if the executable is going to be relocated.
* This is to minimize the number of WriteProcessMemory calls
*/
int create_hollowed_proc(const TCHAR *name, TCHAR *cmd_line, void *map);
#endif
#pragma once
