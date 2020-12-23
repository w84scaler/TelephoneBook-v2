#include "pch.h"
#include <sstream>
#include <string>
#include "DBCore.h"
#include <vector>
#include <fstream>
#include <windows.h>
#pragma warning(disable : 4251)

std::string dbPath = "db.txt";

node* surnameIndexTreeCurrNode;

node* streetIndexTreeCurrNode;

node* telIndexTreeCurrNode;

__declspec(dllexport) node* getSurnameIndex() {
	return surnameIndexTreeCurrNode;
}

__declspec(dllexport) node* getStreetIndex() {
	return streetIndexTreeCurrNode;
}

__declspec(dllexport) node* getTelIndex() {
	return telIndexTreeCurrNode;
}

std::vector<std::string> explode(std::string& s, char delim)
{
	if (s[0] != '1') {
		s = std::string("1 " + s);
	}
	std::vector<std::string> result;
	std::istringstream iss(s);

	for (std::string token; std::getline(iss, token, delim); )
	{
		result.push_back(std::move(token));
	}

	return result;
}

bool isMathcedRecord(std::vector<std::string> adr, std::vector<int> indexes, std::vector<std::string> values) {
	bool isMatched = true;
	for (int i = 0; i < indexes.size(); i++) {
		if (adr[indexes[i]] != values[i]) {
			isMatched = false;
			break;
		}
	}
	return isMatched;
}

DWORD fOffsetHigh = 0;
DWORD fOffsetLow = 0;
SIZE_T fBufferSize = 255;
std::string buffStr = "";
std::vector<Address*> records;
std::vector<std::string> values;
std::vector<int> indexes;

void addAddress(std::vector<std::string> data) {
	Address* adr = new Address(
		data[1],
		data[2],
		data[3],
		data[4],
		data[5],
		atoi(data[6].c_str()),
		atoi(data[7].c_str()),
		atoi(data[8].c_str())
	);
	records.push_back(adr);
}

void ReadInChunks(const char* pszFileName, bool isSeaerchMode, int page) {
	records = std::vector<Address*>();
	SYSTEM_INFO sysinfo = { 0 };
	::GetSystemInfo(&sysinfo);
	DWORD cbView = sysinfo.dwAllocationGranularity;

	HANDLE hfile = CreateFileA(pszFileName, GENERIC_READ, FILE_SHARE_READ,
		NULL, OPEN_EXISTING, 0, NULL);
	if (hfile != INVALID_HANDLE_VALUE) {
		LARGE_INTEGER file_size = { 0 };
		::GetFileSizeEx(hfile, &file_size);
		const unsigned long long cbFile =
			static_cast<unsigned long long>(file_size.QuadPart);

		HANDLE hmap = CreateFileMappingA(hfile, NULL, PAGE_READONLY, 0, 0, NULL);
		if (hmap != NULL) {
			int currentPage = 0;
			for (unsigned long long offset = 0; offset < cbFile; offset += cbView) {
				DWORD high = static_cast<DWORD>((offset >> 32) & 0xFFFFFFFFul);
				DWORD low = static_cast<DWORD>(offset & 0xFFFFFFFFul);
				if (offset + cbView > cbFile) {
					cbView = static_cast<int>(cbFile - offset);
				}
				if (page == currentPage) {
					const char* pView = static_cast<const char*>(
						MapViewOfFile(hmap, FILE_MAP_READ, high, low, cbView));
					if (pView != NULL) {
						char test;
						for (int i = 0; i < cbView; i++) {
							test = pView[i];
							if (test == '\n' || test == '\r') {
								if (buffStr != "") {
									std::vector<std::string> parsedAddress = explode(buffStr, ' ');
									if (isSeaerchMode) {
										if (isMathcedRecord(parsedAddress, indexes, values)) {
											addAddress(parsedAddress);
										}
									}
									else {
										addAddress(parsedAddress);
									}
								}
								buffStr = "";
								continue;
							}

							buffStr += test;
						}
						std::vector<std::string> tempParsedAddress = explode(buffStr, ' ');
						if (tempParsedAddress.size() == 9) {
							if (isSeaerchMode) {
								if (isMathcedRecord(tempParsedAddress, indexes, values)) {
									addAddress(tempParsedAddress);
								}
							}
							else {
								addAddress(tempParsedAddress);
							}
							buffStr = "";
						}
					}
				}
				currentPage++;
			}
			CloseHandle(hmap);
		}
		CloseHandle(hfile);
	}
}

__declspec(dllexport) std::vector<Address*> loadDB(int cP) {
	ReadInChunks(dbPath.c_str(), false, cP);

	return records;
}

__declspec(dllexport) std::vector<Address*> searchAddresses(std::vector<int> _indexes, std::vector<std::string> _values) {
	indexes = _indexes;
	values = _values;
	ReadInChunks(dbPath.c_str(), true, 0);

	return records;
}