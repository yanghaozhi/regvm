#include "compile.h"

#include <regvm.h>

using namespace vasm;

bool compile::find_label(const char* str, std::string& label)
{
    char buf[256];
    if (sscanf(str, "#LABEL: %255[^\n]", buf) == 0) return false;
    buf[sizeof(buf) - 1] = '\0';
    label = buf;
    return true;
}

int64_t compile::pass1::label_id(const std::string& label)
{
    auto it = data.label_ids.find(label);
    if (it != data.label_ids.end()) return it->second;
    return data.label_ids.emplace(label, ++cur_label_id).first->second;
}

compile::pass1::pass1(compile& o) : pass(o), data(o)
{
}

void compile::pass1::comment(const char* line)
{
    std::string label;
    if (data.find_label(line, label) == true)
    {
        auto id = label_id(label);
        auto r = data.label_infos.try_emplace(id, label_info());
        if (r.second == false)
        {
            //TODO
        }
        r.first->second.pos = code_size;
        r.first->second.file = src.file;
        r.first->second.label = label;
        r.first->second.line = cur_line;

        printf("find label : %ld - %ld - %s\n", code_size, cur_line, label.c_str());
    }
}

bool compile::pass1::setc(code_t& code, intptr_t* next, const char* str)
{
    std::string label;
    if (data.find_label(str, label) == true)
    {
        code.ex = 0x09;
        code.id = CODE_SETL;
    }
    return true;
}

bool compile::pass1::line(const code_t* code, int max_bytes, const char* orig)
{
    int bytes = regvm_code_len(*code);
    if (bytes > max_bytes)
    {
        printf("\e[31m --- not enough bytes of code %d, want %d, got %d\e[0m\n", code->id, bytes, max_bytes);
        return false;
    }
    code_size += bytes;
    return true;
}

compile::pass2::pass2(compile& o) : pass(o), data(o)
{
    //int64_t adjust = 0;
    //for (auto& it : data.label_infos)
    //{
    //    if (adjust != 0)
    //    {
    //        printf("label adjust : %s - %ld -> %ld\n", label.c_str(), it.second.pos, it.second.pos - adjust);
    //    }
    //    it.second.pos -= adjust;
    //    if (it.second.pos <= 0xFFFF)
    //    {
    //        adjust += (sizeof(uint64_t) - sizeof(uint16_t));
    //    }
    //    else if (it.second.pos <= 0xFFFFFFFF)
    //    {
    //        adjust += (sizeof(uint64_t) - sizeof(uint32_t));
    //    }
    //}
}

void compile::pass2::comment(const char* line)
{
}

bool compile::pass2::setc(code_t& code, intptr_t* next, const char* str)
{
    std::string label;
    if (data.find_label(str, label) == true)
    {
        code.ex = 0x09;
        code.id = CODE_SETL;
        auto it = data.label_ids.find(label);
        if (it == data.label_ids.end())
        {
            printf("\e[31m --- Can not find id of label : %s \e[0m\n", label.c_str());
            return false;
        }
        int64_t id = it->second;
        auto it2 = data.label_infos.find(id);
        if (it2 == data.label_infos.end())
        {
            printf("\e[31m --- Can not find label info of label : %ld - %s \e[0m\n", id, label.c_str());
            return false;
        }
        *(uint64_t*)next= (uint64_t)id;
        printf("write %ld as %s \n", id, label.c_str());
    }
    return true;
}

bool compile::pass2::line(const code_t* code, int max_bytes, const char* orig)
{
    int bytes = regvm_code_len(*code);
    return (bytes == write_code(code, bytes)) ? true : false;
}

