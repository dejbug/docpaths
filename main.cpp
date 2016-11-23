#define WIN32_LEAN_AND_MEAN
#define STRICT
#include <windows.h>
#include <stdio.h>
#include <stdexcept>

#include <handles/ex.h>

DEFINE_EXCEPTION(std::runtime_error, Error)

int main(int argc, char const ** argv)
{
	STANDARD_TRY_BEGIN
	throw Error("hello");
	STANDARD_TRY_END
	return 0;
}
