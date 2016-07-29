#pragma once
#include <TinyAvBase.h>
#include "kmp.h"

StringW AnsiToUnicode(__in StringA * str);
StringW AnsiToUnicode(__in StringA& str);
StringA UnicodeToAnsi(__in StringW * str);
StringA UnicodeToAnsi(__in StringW& str);