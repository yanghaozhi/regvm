#include "mem_run.h"

#include <regvm.h>
#include <debug.h>
#include <irq.h>
#include <log.h>

using namespace vasm;

mem_run::mem_run() : parser()
{
}

mem_run::~mem_run()
{
}


bool mem_run::comment(const char* line, int size)
{
    return true;
}

bool mem_run::line(const char* str, inst* orig)
{
    auto r = parser::line(str, orig);
    if (orig->id != CODE_SET)
    {
        return r;
    }

    auto v = static_cast<instv<CODE_SET>*>(insts.back());
    switch (v->type)
    {
    case TYPE_STRING:
        {
            const char* p = strchr(v->ex.str, '\n');
            auto it = (p != NULL) ? strs.emplace(std::string(v->ex.str, p - v->ex.str)) : strs.emplace(std::string(v->ex.str));
            return v->change_str(it.first->c_str());
        }
        break;
    default:
        break;
    }
    return r;
}

//bool mem_run::finish()
//{
//    const int line = 16;
//    int j = 0;
//    const unsigned char* p = (const unsigned char*)buf;
//    LOGI("total %lld bytes of codes", (long long)code_bytes);
//    if LOG_IS_ENBALE(DEBUG)
//    {
//        for (int i = 0; i < code_bytes; i++)
//        {
//            if (j++ >= line)
//            {
//                printf("\n");
//                j = 1;
//            }
//            printf("%02X ", p[i]);
//        }
//        printf("\n\n");
//    }
//
//    auto vm = regvm_init();
//
//    if (dbg != NULL)
//    {
//        dbg->start(vm, debugger::VAR | debugger::REG);
//    }
//
//    int64_t exit = 0;
//    bool r = regvm_exec(vm, codes, code_bytes >> 1, (dbg != NULL) ? &dbg->exit : &exit);
//    LOGI("run result : %d and exit code : %lld", r, (long long)((dbg != NULL) ? dbg->exit : exit));
//
//    regvm_exit(vm);
//    return r;
//}

//mem_run::pass1::pass1(mem_run& o) : strs::pass1(o), data(o)
//{
//}
//
//int mem_run::pass1::write_code(const code_t* code, int bytes)
//{
//    data.code_bytes += bytes;
//    return bytes;
//}
//
//mem_run::pass2::pass2(mem_run& o) : strs::pass2(o), data(o)
//{
//    data.buf = malloc(data.code_bytes);
//    cur = data.codes;
//}
//
//int mem_run::pass2::write_code(const code_t* code, int bytes)
//{
//    memcpy(cur, code, bytes);
//    cur += (bytes >> 1);
//    return bytes;
//}

//bool mem_run::debug_reg(struct regvm* vm, void* arg, code_t code, int offset, void* extra)
//{
//    //regvm_reg_info info;
//    //memset(&info, 0, sizeof(info));
//    //cb(arg, NULL);
//    //error::reg_info(vm->reg, [cb, arg](const regvm_reg_info* info)
//    //        {
//    //            cb(arg, info);
//    //        }, &info);
//    //cb(arg, (regvm_reg_info*)(intptr_t)-1);
//    return true;
//}
//
//bool mem_run::debug_var(struct regvm* vm, void* arg, code_t code, int offset, void* extra)
//{
//    //regvm_var_info info;
//    //memset(&info, 0, sizeof(info));
//    //cb(arg, NULL);
//    //error::ctx_vars(*vm->ctx, [cb, arg](const regvm_var_info* info)
//    //        {
//    //            cb(arg, info);
//    //        }, &info);
//    //cb(arg, (regvm_var_info*)(intptr_t)-1);
//    return true;
//}
