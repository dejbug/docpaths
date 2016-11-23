#ifndef _EX_H_
#define _EX_H_

#include <windows.h>
#include <stdexcept>

#define STANDARD_TRY_BEGIN try {
#define STANDARD_TRY_END } catch(std::runtime_error &e) { \
	OutputDebugString(e.what()); }

#define DECLARE_EXCEPTION(base, name) class name;

#define DEFINE_EXCEPTION(base, name) \
	struct name : public base { \
		name(const char * msg) : base(msg) {} \
	};

#endif
