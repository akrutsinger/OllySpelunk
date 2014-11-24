/*******************************************************************************
 * OllySpelunk - Settings.c
 *
 * Written by Austyn Krutsinger
 *
 ******************************************************************************/

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <wchar.h>
#include <math.h>

#include "Settings.h"
#include "OllySpelunk.h"
#include "Resource.h"

/* Globals Definitions - Program specific */
extern unsigned int	g_iMinimumCaveSize;
extern unsigned int g_cave_search_type;
extern unsigned int g_custom_search_value;

BOOL bSettingsChanged	= FALSE;	/* module specific indicator if values changed */
HWND	hCaveTypeNull;
HWND	hCaveTypeNOP;
HWND	hCaveTypeINT3;
HWND	hCaveTypeCustom;
HWND	hCustomCaveValue;
wchar_t custom_search_hex[3];	/* buffer for custom cave search value */


LRESULT CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
					bSettingsChanged = TRUE;
				return TRUE;
			case IDC_TYPE_NULL:
				if ((SendMessage(hCaveTypeNull, BM_GETCHECK, 0, 0)) == BST_CHECKED) {
					g_cave_search_type |= CAVE_TYPE_NULL;
				} else {
					g_cave_search_type ^= CAVE_TYPE_NULL;
				}
				bSettingsChanged = TRUE;
				return TRUE;
			case IDC_TYPE_NOP:
				if ((SendMessage(hCaveTypeNOP, BM_GETCHECK, 0, 0)) == BST_CHECKED) {
					g_cave_search_type |= CAVE_TYPE_NOP;
				} else {
					g_cave_search_type ^= CAVE_TYPE_NOP;
				}
				bSettingsChanged = TRUE;
				return TRUE;
			case IDC_TYPE_INT3:
				if ((SendMessage(hCaveTypeINT3, BM_GETCHECK, 0, 0)) == BST_CHECKED) {
					g_cave_search_type |= CAVE_TYPE_INT3;
				} else {
					g_cave_search_type ^= CAVE_TYPE_INT3;
				}
				bSettingsChanged = TRUE;
				return TRUE;
			case IDC_TYPE_CUSTOM:
				if ((SendMessage(hCaveTypeCustom, BM_GETCHECK, 0, 0)) == BST_CHECKED) {
					EnableWindow(hCustomCaveValue, TRUE);
					g_cave_search_type |= CAVE_TYPE_CUSTOM;
				} else {
					EnableWindow(hCustomCaveValue, FALSE);
					g_cave_search_type ^= CAVE_TYPE_CUSTOM;
				}
				bSettingsChanged = TRUE;
				return TRUE;
			case IDC_CUSTOM_CAVE_VALUE:
				if (HIWORD(wParam) == EN_UPDATE) {
					bSettingsChanged = TRUE;
				}
				return TRUE;
		}
		return TRUE;
	case WM_CLOSE:
		EndDialog(hDlg, 0);
		return TRUE;
	case WM_INITDIALOG:
		/* save handles to checkbox items */
		hCaveTypeNull = GetDlgItem(hDlg, IDC_TYPE_NULL);
		hCaveTypeNOP = GetDlgItem(hDlg, IDC_TYPE_NOP);
		hCaveTypeINT3 = GetDlgItem(hDlg, IDC_TYPE_INT3);
		hCaveTypeCustom = GetDlgItem(hDlg, IDC_TYPE_CUSTOM);
		hCustomCaveValue = GetDlgItem(hDlg, IDC_CUSTOM_CAVE_VALUE);

		/* limit custom cave search value to 2 characters */
		SendDlgItemMessage(hDlg, IDC_CUSTOM_CAVE_VALUE, EM_LIMITTEXT, 2, 0);

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

unsigned int hex_str_to_int(const wchar_t *str)
{
	unsigned int i = 0;
	unsigned int c = 0;
	unsigned int result = 0;
	unsigned int len = (unsigned int)StrlenW(str, 3);

	for (i = 0; i < len; i++) {
		/* get integer value of i'th digit */
        if (str[i] >= '0' && str[i] <= '9') {
			c = str[i] - '0';
        } else if (str[i] >= 'a' && str[i] <= 'f') {
			c = (str[i] - 'a') + 10;
        } else if (str[i] >= 'A' && str[i] <= 'F') {
			c = (str[i] - 'A') + 10;
        }

		/* start at highest bit, convert to base 16, and accumulate the result */
		result += (c * (unsigned int)pow((double)16, (double)(len - 1 - i)));
	}

	return result;
}

void SaveSettings(HWND hDlg)
{
	if (bSettingsChanged == TRUE) {
		g_iMinimumCaveSize = GetDlgItemInt(hDlg, IDC_MIN_CAVE_SIZE, 0, FALSE);
		if (g_iMinimumCaveSize <= 0) {
			g_iMinimumCaveSize = 1;
		}
		Writetoini(NULL, OLLYSPELUNK_NAME, L"Minimum cave size", L"%u", g_iMinimumCaveSize);
		Writetoini(NULL, OLLYSPELUNK_NAME, L"Cave search types", L"%02X", g_cave_search_type);

		GetDlgItemText(hDlg, IDC_CUSTOM_CAVE_VALUE, custom_search_hex, 3);
		g_custom_search_value = hex_str_to_int(custom_search_hex);
		Writetoini(NULL, OLLYSPELUNK_NAME, L"Cave search value", L"%u", g_custom_search_value);
	}
}

void LoadSettings(HWND hDlg)
{
	/* Min / Max values */
	g_iMinimumCaveSize = 32;
	Getfromini(NULL, OLLYSPELUNK_NAME, L"Minimum cave size", L"%u", &g_iMinimumCaveSize);

	SetDlgItemInt(hDlg, IDC_MIN_CAVE_SIZE, g_iMinimumCaveSize, FALSE);

	/* default cave type to search is 0x00 and 0x90 */
	g_cave_search_type = CAVE_TYPE_NULL|CAVE_TYPE_NOP;

	SendDlgItemMessage(hDlg, IDC_TYPE_NULL, BM_SETCHECK, BST_CHECKED, 0);
	SendDlgItemMessage(hDlg, IDC_TYPE_NOP, BM_SETCHECK, BST_CHECKED, 0);
	SendDlgItemMessage(hDlg, IDC_TYPE_INT3, BM_SETCHECK, BST_UNCHECKED, 0);
	SendDlgItemMessage(hDlg, IDC_TYPE_CUSTOM, BM_SETCHECK, BST_UNCHECKED, 0);
	Getfromini(NULL, OLLYSPELUNK_NAME, L"Cave search types", L"%02X", &g_cave_search_type);

	/* set check boxes based off value retrieved from ini file */
	SendDlgItemMessage(hDlg, IDC_TYPE_NULL, BM_SETCHECK, (g_cave_search_type & CAVE_TYPE_NULL), 0);
	SendDlgItemMessage(hDlg, IDC_TYPE_NOP, BM_SETCHECK, (g_cave_search_type & CAVE_TYPE_NOP), 0);
	SendDlgItemMessage(hDlg, IDC_TYPE_INT3, BM_SETCHECK, (g_cave_search_type & CAVE_TYPE_INT3), 0);
	SendDlgItemMessage(hDlg, IDC_TYPE_CUSTOM, BM_SETCHECK, (g_cave_search_type & CAVE_TYPE_CUSTOM), 0);

	/* if custom is checked, enable the text box */
	EnableWindow(hCustomCaveValue, (g_cave_search_type & CAVE_TYPE_CUSTOM));

	/* set value of custom cave search */
	Getfromini(NULL, OLLYSPELUNK_NAME, L"Cave search value", L"%i", &g_custom_search_value);
	snwprintf(custom_search_hex, 2, L"%02X", g_custom_search_value);
	SetWindowText(hCustomCaveValue, custom_search_hex);

	/* when the dialog item is set it receives a EN_CHANGE notification.
	 * set the changed boolean to FALSE since we're looking for manual changed
	 */
	bSettingsChanged = FALSE;
}
