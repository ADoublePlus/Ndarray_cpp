#pragma once

#include <iostream>
#include <exception>
#include <string>

class IndexError : public std::exception
{
	public:
		IndexError(std::string _msg) : msg(_msg) {}

		const char* what() const throw() { return msg.c_str(); }

	private:
		std::string msg;
};

class DimensionError : public std::exception
{
	public:
		DimensionError(std::string _msg) : msg(_msg) {}

		const char* what() const throw() { return msg.c_str(); }

	private:
		std::string msg;
};