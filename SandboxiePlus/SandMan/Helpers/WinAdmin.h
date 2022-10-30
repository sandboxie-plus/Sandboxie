#pragma once


bool IsElevated();
int RunElevated(const std::wstring& Params, bool bGetCode = false);
int RunElevated(const std::wstring& binaryPath, const std::wstring& Params, bool bGetCode = false);
int RestartElevated(int &argc, char **argv);

bool IsAdminUser(bool OnlyFull = false);

bool IsAutorunEnabled();
bool AutorunEnable(bool is_enable);
