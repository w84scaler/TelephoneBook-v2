#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <windows.h>
#include <commctrl.h>
#include <vector>
#include <string>
#include <sstream>
#include <stdlib.h>
#include <mbstring.h>
#include <tchar.h> 

#include "DBCore.h"

#pragma comment(lib, "ComCtl32.Lib")

#define MAIN_CLASS_NAME L"MainWindow"
#define MAIN_WINDOW_NAME L"Phonebook"
#define IDC_LISTVIEW 1111

HWND hWndListView;
const std::vector<LPCWSTR> columnsName{ L"Телефон", L"Фамилия", L"Имя", L"Отчество", L"Улица", L"Дом", L"Корпус", L"Квартира" };
std::vector<HWND> Edits = std::vector<HWND>();
int currentPage = 0;

LPWSTR ConvertToLPWSTR(const std::string& s)
{
	LPWSTR ws = new wchar_t[s.size() + 1];
	copy(s.begin(), s.end(), ws);
	ws[s.size()] = 0;
	return ws;
}

std::string intToStr(int x) {
	std::stringstream ss;
	ss << x;
	std::string str = ss.str();
	return str;
}

VOID InsertListViewItems(HWND hWndListView, std::vector<Address*> items)
{
	ListView_DeleteAllItems(hWndListView);
	LVITEM lvI;

	lvI.pszText = LPSTR_TEXTCALLBACK;
	lvI.mask = LVIF_TEXT;
	lvI.iSubItem = 0;
	for (int i = 0; i < items.size(); i++) {
		lvI.iItem = i;

		ListView_InsertItem(hWndListView, &lvI);

		ListView_SetItemText(hWndListView, i, 0, ConvertToLPWSTR(items[i]->tel));
		ListView_SetItemText(hWndListView, i, 1, ConvertToLPWSTR(items[i]->surname));
		ListView_SetItemText(hWndListView, i, 2, ConvertToLPWSTR(items[i]->name));
		ListView_SetItemText(hWndListView, i, 3, ConvertToLPWSTR(items[i]->fatherName));
		ListView_SetItemText(hWndListView, i, 4, ConvertToLPWSTR(items[i]->street));
		ListView_SetItemText(hWndListView, i, 5, ConvertToLPWSTR(intToStr(items[i]->house)));
		ListView_SetItemText(hWndListView, i, 6, ConvertToLPWSTR(intToStr(items[i]->corp)));
		ListView_SetItemText(hWndListView, i, 7, ConvertToLPWSTR(intToStr(items[i]->flat)));
	}
}

VOID InitListViewColumns(HWND hWndListView)
{
	RECT rcListView;
	LVCOLUMN lvc;

	GetClientRect(hWndListView, &rcListView);
	lvc.mask = LVCF_WIDTH | LVCF_TEXT;
	lvc.cx = 80;
	for (int i = 0; i < columnsName.size(); i++) {
		lvc.pszText = const_cast<LPWSTR>(columnsName[i]);
		ListView_InsertColumn(hWndListView, i, &lvc);
	}
}

HWND CreateEdit(HWND parentWnd, int x, int y, int width) {
	HWND hEdit = CreateWindow(
		L"Edit",
		NULL,
		WS_BORDER | WS_CHILD | WS_VISIBLE | NULL | NULL,
		x,
		y,
		width,
		20,
		parentWnd,
		NULL,
		NULL,
		0);
	ShowWindow(hEdit, SW_SHOWDEFAULT);
	return hEdit;
}

HWND CreateListView(HWND hWndParent)
{
	HWND hWndListView;
	INITCOMMONCONTROLSEX icex;
	RECT rcClient;

	icex.dwICC = ICC_LISTVIEW_CLASSES;
	InitCommonControlsEx(&icex);
	GetClientRect(hWndParent, &rcClient);

	hWndListView = CreateWindowEx(NULL, WC_LISTVIEW, L"", WS_CHILD | LVS_REPORT | LVS_EDITLABELS, 0, 40, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top, hWndParent, (HMENU)IDC_LISTVIEW, GetModuleHandle(NULL), NULL);
	return hWndListView;
}

HMODULE hmd = LoadLibrary(L"DBCore.dll");

typedef std::vector<Address*>(*lDB)(int);
lDB loadDB = (lDB)GetProcAddress(hmd, "?loadDB@@YA?AV?$vector@PAVAddress@@V?$allocator@PAVAddress@@@std@@@std@@H@Z");

typedef std::vector<Address*>(*search)(std::vector<int>, std::vector<std::string>);
search searchAddresses = (search)GetProcAddress(hmd, "?searchAddresses@@YA?AV?$vector@PAVAddress@@V?$allocator@PAVAddress@@@std@@@std@@V?$vector@HV?$allocator@H@std@@@2@V?$vector@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@V?$allocator@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@2@@2@@Z");

typedef int(*hash)(std::string);
hash hashcode = (hash)GetProcAddress(hmd, "?hashcode@@YAHV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@Z");

typedef node* (*tel)();
tel getTelIndex = (tel)GetProcAddress(hmd, "?getTelIndex@@YAPAUnode@@XZ");

typedef node* (*street)();
street getStreetIndex = (street)GetProcAddress(hmd, "?getStreetIndex@@YAPAUnode@@XZ");

typedef node* (*surname)();
surname getSurnameIndex = (surname)GetProcAddress(hmd, "?getSurnameIndex@@YAPAUnode@@XZ");

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

	if (uMsg == WM_CREATE) {

		std::vector<Address*> result = loadDB(currentPage);

		hWndListView = CreateListView(hWnd);
		InitListViewColumns(hWndListView);
		InsertListViewItems(hWndListView, result);
		ShowWindow(hWndListView, SW_SHOWDEFAULT);

		for (int i = 0; i < columnsName.size(); i++) {
			Edits.push_back(CreateEdit(hWnd, i * 90 + 10, 10, 80));
		}

		HWND hwndButton = CreateWindow(
			L"BUTTON",
			L"Find",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
			750,
			10,
			40,
			20,
			hWnd,
			(HMENU)10000,
			(HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
			NULL);
		ShowWindow(hwndButton, SW_SHOWDEFAULT);

		HWND hwndright = CreateWindow(
			L"BUTTON",
			L">",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
			840,
			10,
			20,
			20,
			hWnd,
			(HMENU)10001,
			(HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
			NULL);
		ShowWindow(hwndButton, SW_SHOWDEFAULT);

		HWND hwndleft = CreateWindow(
			L"BUTTON",
			L"<",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
			810,
			10,
			20,
			20,
			hWnd,
			(HMENU)10002,
			(HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
			NULL);
		ShowWindow(hwndButton, SW_SHOWDEFAULT);
	}

	if (uMsg == WM_COMMAND) {

		std::vector<int> editIndexes;
		std::vector<std::string> editValues;
		if (LOWORD(wParam) == 10000)
		{
			for (int i = 0; i < Edits.size(); i++) {
				char str[255];
				ZeroMemory(str, sizeof(str));
				wchar_t wtext[255];
				std::mbstowcs(wtext, str, strlen(str));
				LPWSTR ptr = wtext;
				SendMessage(Edits[i], WM_GETTEXT, (WPARAM)255, (LPARAM)ptr);

				char buffer[500];

				wcstombs(buffer, ptr, 500);
				if (strcmp(buffer, "") != 0) {
					editIndexes.push_back(i + 1);
					editValues.push_back(std::string(buffer));
				}

			}
			std::vector<Address*> result;
			result = searchAddresses(editIndexes, editValues);
			ListView_DeleteAllItems(hWndListView);


			InsertListViewItems(hWndListView, result);
			UpdateWindow(hWndListView);
		}

		if (LOWORD(wParam) == 10001) {
			currentPage++;
			std::vector<Address*> result = loadDB(currentPage);
			ListView_DeleteAllItems(hWndListView);


			InsertListViewItems(hWndListView, result);
			UpdateWindow(hWndListView);
		}

		if (LOWORD(wParam) == 10002) {
			if (currentPage > 0) {
				currentPage--;
			}
			std::vector<Address*> result = loadDB(currentPage);
			ListView_DeleteAllItems(hWndListView);

			InsertListViewItems(hWndListView, result);
			UpdateWindow(hWndListView);
		}
	}

	switch (uMsg) {
	case WM_DESTROY:
		PostQuitMessage(NULL);
		return NULL;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, INT nCmdShow)
{
	HWND hMainWindow;
	MSG msg;
	WNDCLASSEX wc;

	ZeroMemory(&wc, sizeof(wc));
	wc.cbSize = sizeof(wc);
	wc.lpszClassName = MAIN_CLASS_NAME;
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;

	RegisterClassEx(&wc);
	hMainWindow = CreateWindowEx(NULL, MAIN_CLASS_NAME, MAIN_WINDOW_NAME, WS_OVERLAPPED | WS_SYSMENU, 400, 400, 1100, 600, NULL, NULL, hInstance, NULL);

	ShowWindow(hMainWindow, nCmdShow);
	UpdateWindow(hMainWindow);

	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}
}