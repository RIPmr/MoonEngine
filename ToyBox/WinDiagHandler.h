#pragma once
#include <windows.h>
#include <Commdlg.h>
#include <Shlobj.h>
#include <string>
#pragma comment(lib,"Shell32.lib")

class WinDiagMgr {
public:

	static std::string APIENTRY FileDialog() {
	loop:
		OPENFILENAME ofn = { 0 };
		TCHAR strFilename[MAX_PATH] = { 0 };//用于接收文件名
		ofn.lStructSize = sizeof(OPENFILENAME);//结构体大小
		ofn.hwndOwner = NULL;//拥有窗口句柄，为NULL表示对话框是非模态的，实际应用中一般都要有这个句柄
		ofn.lpstrFilter = TEXT("All Files\0*.*\0C/C++ Flie\0*.cpp;*.c;*.h\0\0");//设置过滤
		ofn.nFilterIndex = 1;//过滤器索引
		ofn.lpstrFile = strFilename;//接收返回的文件名，注意第一个字符需要为NULL
		ofn.nMaxFile = sizeof(strFilename);//缓冲区长度
		ofn.lpstrInitialDir = NULL;//初始目录为默认
		ofn.lpstrTitle = TEXT("Choose a file");//使用系统默认标题留空即可
		ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;//文件、目录必须存在，隐藏只读选项
		if (GetOpenFileName(&ofn)) {
			//MessageBox(NULL, strFilename, TEXT("selected file"), 0);
			return strFilename;
		} else {
			//MessageBox(NULL, TEXT("please select a file!"), NULL, MB_ICONERROR);
			//goto loop;
		}

		return "";
	}

	// mini folder select dialog
	static std::string OpenFile() {
		TCHAR szBuffer[MAX_PATH] = { 0 };
		BROWSEINFO bi;
		ZeroMemory(&bi, sizeof(BROWSEINFO));
		bi.hwndOwner = NULL;
		bi.pszDisplayName = szBuffer;
		bi.lpszTitle = "Select model file:";
		bi.ulFlags = BIF_BROWSEINCLUDEFILES;
		LPITEMIDLIST idl = SHBrowseForFolder(&bi);
		if (NULL == idl) return NULL;
		SHGetPathFromIDList(idl, szBuffer);
		return szBuffer;
	}

	static std::string OpenFolder() {
		TCHAR szBuffer[MAX_PATH] = { 0 };
		BROWSEINFO bi;
		ZeroMemory(&bi, sizeof(BROWSEINFO));
		bi.hwndOwner = NULL;
		bi.pszDisplayName = szBuffer;
		bi.lpszTitle = "Select export folder:";
		bi.ulFlags = BIF_RETURNFSANCESTORS;
		LPITEMIDLIST idl = SHBrowseForFolder(&bi);
		if (NULL == idl) return NULL;
		SHGetPathFromIDList(idl, szBuffer);
		return szBuffer;
	}

};
