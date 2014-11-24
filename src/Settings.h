/*******************************************************************************
 * OllySpelunk - Settings.h
 *
 * Written by Austyn Krutsinger
 *
 ******************************************************************************/

#ifndef __OLLYSPELUNK_SETTINGS_H__
#define __OLLYSPELUNK_SETTINGS_H__

#define CAVE_TYPE_NULL		0x01
#define CAVE_TYPE_NOP       0x02
#define CAVE_TYPE_INT3      0x04
#define CAVE_TYPE_CUSTOM    0x08

/* Global Declarations */
unsigned int	g_iMinimumCaveSize;
unsigned int	g_cave_search_type;
unsigned int	g_custom_search_value;

/* Prototypes */
LRESULT CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
void SaveSettings(HWND hDlg);
void LoadSettings(HWND hDlg);

#endif	/* __OLLYSPELUNK_SETTINGS_H__ */
