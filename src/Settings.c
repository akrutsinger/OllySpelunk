/*******************************************************************************
 * OllySpelunk - Settings.c
 *
 * Copyright (c) 2013, Austyn Krutsinger
 * All rights reserved.
 *
 * OllySpelunk is released under the New BSD license (see LICENSE.txt).
 *
 ******************************************************************************/

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <ShellAPI.h>

#include "Settings.h"
#include "OllySpelunk.h"
#include "Resource.h"

/* Globals Definitions - Program specific */
extern int	g_iMinimumCaveSize;

BOOL bMinimumChanged	= FALSE;	/* module specific indicator if values changed */

INT_PTR CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
			case IDC_OK:
				/* Save the settings */
				SaveSettings(hDlg);
				EndDialog(hDlg, 1);
				return TRUE;
			case IDC_CANCEL:
				/* End dialog without saving anything */
				EndDialog(hDlg, 0);
				return TRUE;
			case IDC_MIN_CAVE_SIZE:
				if (HIWORD(wParam) == EN_CHANGE)
					bMinimumChanged = TRUE;
				return TRUE;
		}
		return TRUE;
	case WM_CLOSE:
		EndDialog(hDlg, 0);
		return TRUE;
	case WM_INITDIALOG:
		/* Load settings from ollydbg.ini. If there is no
		 * setting already in the ollydbg.ini, set the default
		 * values so we can save them if we want
		 */
		LoadSettings(hDlg);
		SetFocus(GetDlgItem(hDlg, IDC_CANCEL));
		return TRUE;
	}
	return FALSE;
}

void SaveSettings(HWND hDlg)
{
	if (bMinimumChanged == TRUE) {
		g_iMinimumCaveSize = GetDlgItemInt(hDlg, IDC_MIN_CAVE_SIZE, 0, FALSE);
		if (g_iMinimumCaveSize <= 0) {
			g_iMinimumCaveSize = 1;
		}
		Writetoini(NULL, OLLYSPELUNK_NAME, L"Minimum cave size", L"%u", g_iMinimumCaveSize);
	}
}

void LoadSettings(HWND hDlg)
{
	/* Min / Max values */
	g_iMinimumCaveSize = 32;
	Getfromini(NULL, OLLYSPELUNK_NAME, L"Minimum cave size", L"%u", &g_iMinimumCaveSize);

	SetDlgItemInt(hDlg, IDC_MIN_CAVE_SIZE, g_iMinimumCaveSize, FALSE);

	/* when the dialog item is set it receives a EN_CHANGE notification.
	 * set the changed bools to FALSE since we're looking for manual changed
	 */
	bMinimumChanged = FALSE;
}
