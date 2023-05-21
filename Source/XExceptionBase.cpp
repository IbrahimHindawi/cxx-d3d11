#pragma once
#include "XExceptionBase.h"
#include <sstream>

XExceptionBase::XExceptionBase(int Line, const char* File) noexcept: Line(Line), File(File)
{

}

const char* XExceptionBase::what() const noexcept
{
	std::ostringstream outstringstream = std::ostringstream();
	outstringstream << GetType() << std::endl << GetOriginString();
	WhatBuffer = outstringstream.str();
	return WhatBuffer.c_str();
}

const char* XExceptionBase::GetType() const noexcept
{
	return "XExceptionBase";
}

int XExceptionBase::GetLine() const noexcept
{
	return Line;
}

const std::string& XExceptionBase::GetFile() const noexcept
{
	return File;
}

std::string XExceptionBase::GetOriginString() const noexcept
{
	std::ostringstream outstringstream;
	outstringstream << "[File]" << File << std::endl << "[Line]" << Line;
	return outstringstream.str();
}
