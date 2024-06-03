#pragma once

#include <stdbool.h>

#include "code.h"

struct regvm;

struct regvm* regvm_init();

bool regvm_exit(struct regvm* vm);

bool regvm_exe_one(struct regvm* vm, const struct code* inst);

