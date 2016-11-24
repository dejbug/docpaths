#include "text.h"
#include <stdio.h>
#include <stdarg.h>

char const * text::Format(char * buffer, size_t size,
		char const * format, ...) {

	va_list list;
	va_start(list, format);
	_vsnprintf(buffer, size, format, list);
	va_end(list);
	return buffer;
}

text::AnsiBase::AnsiBase() : result(STATUS_SUCCESS) {
}

text::Ansi::Ansi(UNICODE_STRING & ustr) {
	memset(&data, 0, sizeof(ANSI_STRING));
	result = RtlUnicodeStringToAnsiString(&data, &ustr, TRUE);
	if(STATUS_SUCCESS != result)
		throw AnsiError("Ansi: failed converting UNICODE string");
}

text::Ansi::Ansi(LPCSTR text) {
	memset(&data, 0, sizeof(ANSI_STRING));
	RtlInitAnsiString(&data, text);
}

text::Ansi::~Ansi() {
	if(data.MaximumLength)
		RtlFreeAnsiString(&data);
}

size_t text::Ansi::GetLength() const {
	return data.Length;
}

char const * text::Ansi::GetText() const {
	return data.Buffer;
}
