/*******************************************************************************
 * OllySpelunk - Settings.h
 *
 * Copyright (c) 2013, Austyn Krutsinger
 * All rights reserved.
 *
 * OllySpelunk is released under the New BSD license (see LICENSE.txt).
 *
 ******************************************************************************/

#ifndef __OLLYSPELUNK_SETTINGS_H__
#define __OLLYSPELUNK_SETTINGS_H__

/* Global Declarations */
int	g_iMinimumCaveSize;

/* Prototypes */
INT_PTR CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
void SaveSettings(HWND hDlg);
void LoadSettings(HWND hDlg);

#endif	/* __OLLYSPELUNK_SETTINGS_H__ */