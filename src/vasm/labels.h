#pragma once

#include "parser.h"

namespace vasm
{

class labels : public parser
{
public:
    struct pass1 : public parser::pass
    {
        int64_t     code_size       = 0;
        int64_t     cur_label_id    = 0;

        pass1(labels& o);

        virtual int write_code(const code_t* code, int bytes) {return bytes;};

        virtual void comment(const char* line);
        virtual bool setc(code_t& code, intptr_t* next, const char* str);
        virtual bool line(const code_t* code, int max_bytes, const char* orig);

        int64_t label_id(const std::string& label);
    private:
        labels& data;
    };

    struct pass2 : public parser::pass
    {
        pass2(labels& o);

        virtual int write_code(const code_t* code, int bytes)     = 0;

        virtual void comment(const char* line);
        virtual bool setc(code_t& code, intptr_t* next, const char* str);
        virtual bool line(const code_t* code, int max_bytes, const char* orig);

    private:
        labels& data;
    };


protected:
    struct label_info
    {
        int64_t     pos;
        int64_t     line;
        std::string file;
        std::string label;
    };
    std::map<int64_t, label_info>                   label_infos;
    std::unordered_map<std::string, int64_t>        label_ids;

    bool find_label(const char* str, std::string& label);
};

}

