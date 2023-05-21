#pragma once
#include "common.h"
#include <exception>
#include <string>

class XExceptionBase: public std::exception 
{
private:
	int Line;
	std::string File;
protected:
	mutable std::string WhatBuffer;
public:
	XExceptionBase(int Line, const char* File) noexcept;
	const char* what() const noexcept override;
	virtual const char* GetType() const noexcept;
	int GetLine() const noexcept;
	const std::string& GetFile() const noexcept;
	std::string GetOriginString() const noexcept;

};