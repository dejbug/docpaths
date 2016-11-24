#ifndef _TYP_H_
#define _TYP_H_

#include <string>
#include <vector>
#include <map>

#include "macro.h"

NAMESPACE_BEGIN(typ)

using string = std::string;
using strings = std::vector<string>;
using stringmap = std::map<string,string>;

NAMESPACE_END

#endif
