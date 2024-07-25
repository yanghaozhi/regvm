#pragma once

#include "mlib.h"

#ifdef REGVM_EXT
#define REGVM_IMPL              ext::MLIB_CAT(regvm_, REGVM_EXT)
#else
#define REGVM_IMPL              regvm
#endif

