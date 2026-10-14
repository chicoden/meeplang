#pragma once

#include "string.h"

enum meep_token_kind_t {
};

struct meep_token_t {
    enum meep_token_kind_t kind;
    struct meep_string_t value;
};
