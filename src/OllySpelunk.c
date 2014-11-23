/*******************************************************************************
 * OllySpelunk - OllySpelunk.c
 *
 * Copyright (c) 2013, Austyn Krutsinger
 * All rights reserved.
 *
 * OllySpelunk is released under the New BSD license (see LICENSE.txt).
 *
 ******************************************************************************/

/*******************************************************************************
 * Things to change as I think of them...
 * [ ] = To do
 * [?] = Might be a good idea?
 * [!] = Implemented
 * [+] = Added
 * [-] = Removed
 * [*] = Changed
 * [~] = Almost there...
 *
 * Version 0.2.0 (21JUN2014)
 * [+] Added menu to code cave log window
 * [*] Changed naming convention of functions and variables
 * [*] Changed shortcut key for code cave search
 *
 * Version 0.1.0 (20OCT2013)
 * [+] Initial release
 *
 *
 * -----------------------------------------------------------------------------
 * TODO
 * -----------------------------------------------------------------------------
 *
 * [+] Find code caves in all loaded modules
 *
 *
 ******************************************************************************/

#include "OllySpelunk.h"
#include <stdio.h>
#include "settings.h"
#include "resource.h"
#include "plugin.h"

/* Module specific globals */
HINSTANCE	plugin_inst_g		= NULL;
t_table		logtable 			= {{0}};		/* log table used for code cave data */

/*
 * Plug-in menu that will appear in the main OllyDbg menu
 * and in pop-up menu.
 */
static t_menu plugin_main_menu[] = {
	{ L"Search For Caves",
		L"Search for code caves through the entire loaded module",
		KK_DIRECT|KK_CTRL|KK_ALT|'C', menu_handler, NULL, MENU_FIND_CAVES },
	{ L"|Settings",
		L"Configure Search Settings",
		K_NONE, menu_handler, NULL, MENU_SETTINGS },
	{ L"|About",
		L"About OllySpelunk",
		K_NONE, menu_handler, NULL, MENU_ABOUT },
	/* End of menu. */
	{ NULL, NULL, K_NONE, NULL, NULL, 0 }
};

/*
 * Plug-in menu that will appear in the pop-up menu.
 */
static t_menu plugin_popup_menu[] = {
	{ L"Search For Caves",
		L"Search for code caves through the entire loaded module",
		KK_DIRECT|KK_CTRL|KK_ALT|'C', menu_handler, NULL, MENU_FIND_CAVES },
	{ L"|Settings",
		L"Configure Search Settings",
		K_NONE, menu_handler, NULL, MENU_SETTINGS },
	/* End of menu. */
	{ NULL, NULL, K_NONE, NULL, NULL, 0 }
};

/* Pop-up menu for view address in CPU Disasm */
static t_menu log_window_popup_menu[] = {
	{ L"View In CPU Disasm",
		L"View Address In CPU Disasm",
		KK_DIRECT|KK_SHIFT|'W', menu_handler, NULL, MENU_VIEW_IN_CPU },
	{ L"|Settings",
		L"Configure Search Settings",
		K_NONE, menu_handler, NULL, MENU_SETTINGS },
	/* End of menu. */
	{ NULL, NULL, K_NONE, NULL, NULL, 0 }
};

/*
 *
 * Plugin specific functions
 *
 */
void about_message(void)
{
	wchar_t szMessage[TEXTLEN] = { 0 };
	wchar_t szBuffer[SHORTNAME];
	int n;

	/* Debuggee should continue execution while message box is displayed. */
	Resumeallthreads();
	/* In this case, swprintf() would be as good as a sequence of StrcopyW(), */
	/* but secure copy makes buffer overflow impossible. */
	n = StrcopyW(szMessage, TEXTLEN, L"OllySpelunk v");
	n += StrcopyW(szMessage + n, TEXTLEN - n, OLLYSPELUNK_VERSION);
	n += StrcopyW(szMessage + n, TEXTLEN - n, L"\n\nCoded by Austyn Krutsinger <akrutsinger@gmail.com>");
	n += StrcopyW(szMessage + n, TEXTLEN - n, L"\n\nCompiled on ");
	Asciitounicode(__DATE__, SHORTNAME, szBuffer, SHORTNAME);
	n += StrcopyW(szMessage + n, TEXTLEN - n, szBuffer);
	n += StrcopyW(szMessage + n, TEXTLEN - n, L" ");
	Asciitounicode(__TIME__, SHORTNAME, szBuffer, SHORTNAME);
	n += StrcopyW(szMessage + n, TEXTLEN - n, szBuffer);
	n += StrcopyW(szMessage + n, TEXTLEN - n, L" with ");
	#if defined(__BORLANDC__)
		n += StrcopyW(szMessage + n, TEXTLEN - n, L"Borland (R) ");
	#elif defined(_MSC_VER)
		n += StrcopyW(szMessage + n, TEXTLEN - n, L"Microsoft (R) ");
	#elif defined(__MINGW32__)
		n += StrcopyW(szMessage + n, TEXTLEN - n, L"MinGW32 ");
	#elif defined(__GNUC__)
		StrcopyW(szMessage + n, TEXTLEN - n, L"GCC ");
	#else
		n += StrcopyW(szMessage + n, TEXTLEN - n, L"\n\nCompiled with ");
	#endif
	#ifdef __cplusplus
		StrcopyW(szMessage + n, TEXTLEN - n, L"C++ compiler");
	#else
		StrcopyW(szMessage + n, TEXTLEN - n, L"C compiler");
	#endif
	MessageBox(hwollymain, szMessage, L"About OllySpelunk", MB_OK|MB_ICONINFORMATION);
	/* Suspendallthreads() and Resumeallthreads() must be paired, even if they */
	/* are called in inverse order! */
	Suspendallthreads();
}

int menu_handler(t_table* pTable, wchar_t* pszName, DWORD dwIndex, int iMode)
{
	UNREFERENCED_PARAMETER(pTable);
	UNREFERENCED_PARAMETER(pszName);

	LPLOGDATA pLogData;

	switch (iMode) {
	case MENU_VERIFY:
		return MENU_NORMAL;

	case MENU_EXECUTE:
		switch (dwIndex) {
		case MENU_FIND_CAVES:
			/* only execute if a module is loaded */
			if (Findmainmodule() == NULL) {
				Resumeallthreads();
				MessageBox(hwollymain, L"No module loaded", L"OllySpelunk", MB_OK | MB_ICONINFORMATION);
				Suspendallthreads();
			} else {
				if (logtable.hw == NULL) {
					Createtablewindow(&logtable, 0, logtable.bar.nbar, NULL, L"ICO_PLUGIN", OLLYSPELUNK_NAME);
				} else {
					Activatetablewindow(&logtable);
				}
				FindCodeCaves();
				return MENU_REDRAW;	/* force redrawing of the table after data is added to it */
			}
			break;
		case MENU_VIEW_IN_CPU:
			//View address in CPU Disasm
            pLogData = (LPLOGDATA)Getsortedbyselection(&pTable->sorted, pTable->sorted.selected);
            if (pLogData != NULL) {
                Setcpu(0, pLogData->dwAddress, 0, 0, 0, CPU_ASMHIST|CPU_ASMCENTER|CPU_ASMFOCUS);
            }
			Activatetablewindow(&logtable);
			break;
		case MENU_SETTINGS:
			DialogBox(plugin_inst_g,
						MAKEINTRESOURCE(IDD_SETTINGS),
						hwollymain,
						(DLGPROC)DlgProc);
			break;
		case MENU_ABOUT:
			about_message();
			break;
		}
		return MENU_NOREDRAW;
	}

	return MENU_ABSENT;
}

void create_log_window(void)
{
	StrcopyW(logtable.name, SHORTNAME, OLLYSPELUNK_NAME);
	logtable.mode = TABLE_SAVEPOS|TABLE_AUTOUPD;
	logtable.bar.visible = 1;

	logtable.bar.name[0] = L"Address";
	logtable.bar.expl[0] = L"";
	logtable.bar.mode[0] = BAR_SORT;
	logtable.bar.defdx[0] = 9;

	logtable.bar.name[1] = L"Cave Size";
	logtable.bar.expl[1] = L"";
	logtable.bar.mode[1] = BAR_SORT;
	logtable.bar.defdx[1] = 64;

	logtable.bar.nbar = 2;
	logtable.tabfunc = (TABFUNC*)log_window_proc;
	logtable.custommode = 0;
	logtable.customdata = NULL;
	logtable.updatefunc = NULL;
	logtable.drawfunc = (DRAWFUNC*)log_window_draw;
	logtable.tableselfunc = NULL;
	logtable.menu = (t_menu*)log_window_popup_menu;
}

long log_window_draw(wchar_t *pszBuffer, uchar *pMask, int *pSelect, t_table *pTable, t_drawheader *pHeader, int iColumn, void *pCache)
{
	int	str_len = 0;
	LPLOGDATA pLogData = (LPLOGDATA)pHeader;

	switch (iColumn)
	{
	case 0:	/* address of cave */
		str_len = Simpleaddress(pszBuffer, pLogData->dwAddress, pMask, pSelect);
		break;
	case 1: /* size of code cave */
        str_len = swprintf(pszBuffer, L"%lu", pLogData->dwCaveSize);
		break;
	default:
		break;
	}
	return str_len;
}

long log_window_proc(t_table *pTable, HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LPLOGDATA pLogData;
	switch (uMsg)
	{
	case WM_USER_UPD:		/* autoupdate contents of the window */
		InvalidateRect(pTable->hw, NULL, FALSE);
		break;
	case WM_USER_DBLCLK:
		pLogData = (LPLOGDATA)Getsortedbyselection(&pTable->sorted, pTable->sorted.selected);
		if (pLogData != NULL)
			Setcpu(0, pLogData->dwAddress, 0, 0, 0, CPU_ASMHIST|CPU_ASMCENTER|CPU_ASMFOCUS);
		return 1;
	case WM_USER_CREATE:
		Setautoupdate(&logtable, 1);
		break;
	default:
		break;
	}
	return 0;
}

int log_window_sort_proc(const t_sorthdr *pSortHeader1, const t_sorthdr *pSortHeader2, const int iSort)
{
	int i = 0;
    LPLOGDATA pLogData1	= NULL;
    LPLOGDATA pLogData2	= NULL;

    pLogData1 = (LPLOGDATA)pSortHeader1;
    pLogData2 = (LPLOGDATA)pSortHeader2;

	if (iSort == 1) {	/* sort by size of code cave */
		if (pLogData1->dwCaveSize > pLogData2->dwCaveSize) {
			i = -1;
		} else if (pLogData1->dwCaveSize < pLogData2->dwCaveSize){
			i = 1;
		}
	}
	if (i == 0) {	/* additionally sort by address of code cave */
		if (pLogData1->dwAddress < pLogData2->dwAddress) {
			i = -1;
		} else if (pLogData1->dwAddress > pLogData2->dwAddress) {
			i = 1;
		}
	}
	return i;
}

void FindCodeCaves(void)
{
	t_dump		*pDump				= NULL;
	t_module	*pModule			= NULL;
	LPBYTE		lpBuffer			= NULL;
	DWORD		dwCaveStartAddress	= 0;
	DWORD		dwCaveSize			= 0;
	DWORD		nIndex				= 0;
	BOOL		bSaveAddress		= TRUE;
	LOGDATA		stLogData			= {0};
	wchar_t		szTitle[SHORTNAME]	= {0};
	int			n					= 0;

	/* clear log table */
	Deletesorteddatarange(&(logtable.sorted), 0x00000000, 0xFFFFFFFF);

	pDump = Getcpudisasmdump();
	/* use module based off code in CPU window */
	pModule = Findmodule(pDump->base);

	/* set title with name of module that was searched */
	n = StrcopyW(szTitle, SHORTNAME, OLLYSPELUNK_NAME);
	n += StrcopyW(szTitle + n, SHORTNAME - n, L" - ");
	n += StrcopyW(szTitle + n, SHORTNAME - n, pModule->modname);
	SetWindowText(logtable.hparent, (LPWSTR)szTitle);

	/* read module memory */
	lpBuffer = (LPBYTE)Memalloc(pModule->codesize, REPORT|ZEROINIT);
	DWORD dwBytesRead = Readmemory(lpBuffer, pModule->codebase, pModule->codesize, MM_SILENT);

	for (nIndex = 0; nIndex <= dwBytesRead; nIndex++) {
        /* find start of cave */
        if (lpBuffer[nIndex] == '\0') {
        	if (bSaveAddress == TRUE) {
				dwCaveStartAddress = pModule->codebase + nIndex;
				bSaveAddress = FALSE;
        	}
        	dwCaveSize++;
		} else {
			/* Cave is large enough to save */
			if (dwCaveSize >= g_iMinimumCaveSize) {
				stLogData.dwAddress = dwCaveStartAddress;
				stLogData.dwSize = 1;
				stLogData.dwType = 0;
				stLogData.dwCaveSize = dwCaveSize;
				Addsorteddata(&(logtable.sorted), &stLogData);
			}
			/* need to reset save flag and cave size to find the next cave start */
			bSaveAddress = TRUE;
			dwCaveSize = 0;
		}
	}

	/* free buffer */
	if (lpBuffer != NULL) {
		Memfree(lpBuffer);
	}
}

/**
 *
 * OllyDbg internal functions
 *
 */

BOOL WINAPI DllMain(HINSTANCE hinstDll, DWORD fdwReason, LPVOID lpReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH) {
		plugin_inst_g = hinstDll;		/* Save plugin instance */
	}
	return 1;					/* Report success */
};

extc int __cdecl ODBG2_Pluginquery(int iOllyDbgVersion, DWORD *dwFeatures, wchar_t szPluginName[SHORTNAME], wchar_t szPluginVersion[SHORTNAME])
{
	if (iOllyDbgVersion < 201) {
		return 0;
	}
	/* Report name and version to OllyDbg */
	StrcopyW(szPluginName, SHORTNAME, OLLYSPELUNK_NAME);
	StrcopyW(szPluginVersion, SHORTNAME, OLLYSPELUNK_VERSION);
	return PLUGIN_VERSION;			/* Expected API version */
};

extc int __cdecl ODBG2_Plugininit(void)
{
	if (Createsorteddata(&logtable.sorted, sizeof(LOGDATA), 1, (SORTFUNC *)log_window_sort_proc, NULL, 0) == -1) {
		Addtolist(0, DRAW_HILITE, L"%s: Unable to created sorted table data.", OLLYSPELUNK_NAME);
		return -1;
	}

	create_log_window();
	LoadSettings(NULL);

	Addtolist(0, DRAW_NORMAL, L"");
	Addtolist(0, DRAW_NORMAL, L"[*] %s v%s", OLLYSPELUNK_NAME, OLLYSPELUNK_VERSION);
	Addtolist(0, DRAW_NORMAL, L"[*] Coded by: Austyn Krutsinger <akrutsinger@gmail.com>");
	Addtolist(0, DRAW_NORMAL, L"");

	/* Report success. */
	return 0;
};

extc t_menu * __cdecl ODBG2_Pluginmenu(wchar_t *type)
{
	if (wcscmp(type, PWM_MAIN) == 0) {
		/* Main menu. */
		return plugin_main_menu;
	} else if (wcscmp(type, PWM_DISASM) == 0) {
		/* Disassembler pane of CPU window. */
		return plugin_popup_menu;
	}
	return NULL;                /* No menu */
};

extc void __cdecl ODBG2_Pluginreset(void)
{
	Deletesorteddatarange(&(logtable.sorted), 0x00000000, 0xFFFFFFFF);
}

extc void __cdecl ODBG2_Plugindestroy(void)
{
	Destroysorteddata(&(logtable.sorted));
}
