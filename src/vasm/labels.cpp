#include "labels.h"

#include <regvm.h>

using namespace vasm;

bool labels::find_label(const char* str, std::string& label)
{
    char buf[256];
    if (sscanf(str, "#LABEL: %255[^\n]", buf) == 0) return false;
    buf[sizeof(buf) - 1] = '\0';
    label = buf;
    return true;
}

int64_t labels::pass1::label_id(const std::string& label)
{
    auto it = data.label_ids.find(label);
    if (it != data.label_ids.end()) return it->second;
    return data.label_ids.emplace(label, ++cur_label_id).first->second;
}

labels::pass1::pass1(labels& o) : pass(o), data(o)
{
}

void labels::pass1::comment(const char* line)
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
        r.first->second.pos = code_size >> 1;
        r.first->second.file = src.file;
        r.first->second.label = label;
        r.first->second.line = cur_line;

        LOGD("find label : %lld - %lld - %s", (long long)code_size, (long long)cur_line, label.c_str());
    }
}

bool labels::pass1::setc(code_t& code, intptr_t* next, const char* str)
{
    //std::string label;
    //if (data.find_label(str, label) == false)
    //{
    //    return false;
    //}
    //code.ex = TYPE_ADDR;
    //code.id = CODE_SETL;
    return true;
}

bool labels::pass1::line(const code_t* code, int max_bytes, const char* orig)
{
    return true;
    //LOGD("%d - %d - %d", code->id, code->reg, code->ex);
    //int bytes = regvm_code_len(*code) << 1;
    //if (bytes > max_bytes)
    //{
    //    LOGE("not enough bytes of code %d, want %d, got %d", code->id, bytes, max_bytes);
    //    return false;
    //}
    //code_size += bytes;
    //return (bytes == write_code(code, bytes)) ? true : false;
}

labels::pass2::pass2(labels& o) : pass(o), data(o)
{
    //int64_t adjust = 0;
    //for (auto& it : data.label_infos)
    //{
    //    if (adjust != 0)
    //    {
    //        DEBUG("label adjust : {} - {} -> {}", label.c_str(), it.second.pos, it.second.pos - adjust);
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

void labels::pass2::comment(const char* line)
{
}

bool labels::pass2::setc(code_t& code, intptr_t* next, const char* str)
{
    //std::string label;
    //if (data.find_label(str, label) == true)
    //{
    //    code.ex = TYPE_ADDR;
    //    code.id = CODE_SETL;
    //    auto it = data.label_ids.find(label);
    //    if (it == data.label_ids.end())
    //    {
    //        LOGE("Can not find id of label : %s", label.c_str());
    //        return false;
    //    }
    //    int64_t id = it->second;
    //    auto it2 = data.label_infos.find(id);
    //    if (it2 == data.label_infos.end())
    //    {
    //        LOGE("Can not find label info of label : %lld - %s", (long long)id, label.c_str());
    //        return false;
    //    }
    //    *(uint64_t*)next= (uint64_t)it2->second.pos;
    //    LOGD("write %lld as %s ", (long long)it2->second.pos, label.c_str());
    //    return true;
    //}
    return false;
}

bool labels::pass2::line(const code_t* code, int max_bytes, const char* orig)
{
    int bytes = regvm_code_len(*code) << 1;
    return (bytes == write_code(code, bytes)) ? true : false;
}

