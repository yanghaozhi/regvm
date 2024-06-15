#include "compile.h"

using namespace vasm;

bool compile::find_label(const char* str, std::string& label)
{
    char buf[256];
    if (sscanf(str, "#LABEL: %255[^\n]", buf) == 0) return false;
    buf[sizeof(buf) - 1] = '\0';
    label = buf;
    return true;
}

int64_t compile::label_id(const std::string& label)
{
    auto it = label_ids.find(label);
    if (it != label_ids.end()) return it->second;
    return label_ids.emplace(label, ++cur_label_id).first->second;
}

void compile::comment(const char* line)
{
}

uint32_t compile::setc(code_t& code, intptr_t* next, const char* str)
{
    return 0;
}

int compile::line(const code_t* code, int max_bytes, const char* orig)
{
    int bytes = 2;
    switch (code->id)
    {
    case CODE_SETS:
        bytes += 2;
        break;
    case CODE_SETI:
        bytes += 4;
        break;
    case CODE_SETL:
        bytes += 8;
        break;
    case CODE_NOP:
        bytes += (code->ex << 1);
        break;
    default:
        break;
    }
    if (bytes > max_bytes)
    {
        printf("\e[31m --- not enough bytes of code %d, want %d, got %d\e[0m\n", code->id, bytes, max_bytes);
        return 0;
    }
    return write_code(code, bytes);
}

