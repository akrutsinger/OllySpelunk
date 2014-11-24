/*******************************************************************************
 * OllySpelunk - OllySpelunk.h
 *
 * Written by Austyn Krutsinger
 *
 ******************************************************************************/

#ifndef __OLLYSPELUNK_H__
#define __OLLYSPELUNK_H__

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "plugin.h"

/*  To use this exported function of dll, include this header
 *  in the project.
 */
#ifdef BUILD_DLL
    #define DLL_EXPORT __declspec(dllexport)
#else
    #define DLL_EXPORT __declspec(dllimport)
#endif

#define OLLYSPELUNK_NAME		L"OllySpelunk"		/* Unique plugin name */
#define OLLYSPELUNK_VERSION		L"0.2.0"			/* Plugin version (stable . update . patch  - status) */

#ifdef __cplusplus
extern "C" {
#endif

/* Menu items */
#define MENU_FIND_CAVES			1
#define	MENU_SETTINGS			2
#define	MENU_ABOUT				3
#define MENU_VIEW_IN_CPU		4

/* Global Declarations */


/**
 * Forward declarations
 */
 /* Menu functions */
int menu_handler(t_table* pTable, wchar_t* pszName, DWORD dwIndex, int iMode);
void about_message(void);

/* Log window functions */
void create_log_window(void);
long log_window_proc(t_table *pTable, HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
long log_window_draw(wchar_t *szBuffer, uchar *pMask, int *pSelect, t_table *pTable, t_drawheader *pHeader, int iColumn, void *pCache);
int log_window_sort_proc(const t_sorthdr *pSortHeader1, const t_sorthdr *pSortHeader2, const int iSort);

void FindCodeCaves(void);

#ifdef DEVELOPMENT_MODE
void test_code(void);
#endif

typedef struct _LOGDATA {
	/* Obligatory header, its layout _must_ coincide with t_sorthdr! */
	DWORD		dwAddress;	/* address of the call */
	DWORD		dwSize;		/* Size of index, always 1 in our case */
	DWORD		dwType;		/* Type of entry, TY_xxx */

	/* Custom data follows header. */
	DWORD		dwCaveType;			/* type of cave (e.g. 0x00, 0x90, 0xCC) */
	DWORD		dwCaveSize;			/* size of code cave */
} LOGDATA, *LPLOGDATA;

#ifdef __cplusplus
}
#endif

#endif	/* __OLLYSPELUNK_H__ */
