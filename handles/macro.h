#ifndef _MACRO_H_
#define _MACRO_H_

#include <stdio.h>

#define NAMESPACE_BEGIN(name) namespace name {
#define NAMESPACE_END }

#define STANDARD_TRY_BEGIN try {
#define STANDARD_TRY_END } catch(std::runtime_error & e) { \
	fprintf(stderr, "! %s\n", e.what()); }
#define STANDARD_TRY_END_DBG } catch(std::runtime_error & e) { \
	OutputDebugString(e.what()); }

#define DECLARE_EXCEPTION(base, name) class name;
#define DEFINE_EXCEPTION(base, name) \
	struct name : public base { \
		name(const char * msg) : base(msg) {} \
	};

#endif
