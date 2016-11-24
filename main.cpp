#define WIN32_LEAN_AND_MEAN
#define STRICT
#include <windows.h>
#include <stdio.h>
#include <stdexcept>

#include <handles/typ.h>
#include <handles/ex.h>
#include <handles/sys.h>


int main(int argc, char const ** argv)
{
	STANDARD_TRY_BEGIN

	typ::strings ss;
	ss.push_back("hello, how do you do?");
	ss.push_back("good bye.");
	ss.push_back("see you soon.");

	typ::string s;
	sys::JoinStrings(s, ss, "|");

	OutputDebugString(s.c_str());

	sys::Dll dll("kernel32.dll");
	using odsw_t = VOID(WINAPI*)(LPCWSTR);
	auto f = dll.find<odsw_t>("OutputDebugStringW");
	f(L"hi, again!");

	STANDARD_TRY_END

	return 0;
}
