#ifndef _TEXT_H_
#define _TEXT_H_

#include <windows.h>

#include "typ.h"
#include "ex.h"
#include "nt.h"

NAMESPACE_BEGIN(text)

const char * Format(char * buffer, size_t size,
	char const * format, ...);

struct AnsiBase {
	DEFINE_EXCEPTION(ex::Error, AnsiError)

	NTSTATUS result;

	AnsiBase();
};

struct Ansi : protected AnsiBase {
	ANSI_STRING data;

	Ansi(UNICODE_STRING & ustr);
	Ansi(LPCSTR text);
	virtual ~Ansi();

	size_t GetLength() const;
	char const * GetText() const;
};

NAMESPACE_END

#endif
