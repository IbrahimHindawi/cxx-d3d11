#pragma once
#include "common.h"
#include <string>

std::wstring ToWide(const std::string& narrow);
std::string ToNarrow(const std::wstring& wide);
