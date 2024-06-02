#pragma once


#pragma pack(1)
struct code
{
    struct 
    {
        uint32_t        code    : 11;
        uint32_t        type    : 3;
        uint32_t        reg     : 8;
        uint32_t        unused  : 8;
    };
    union
    {
        int64_t         num;
        double          dbl;
        const char*     str;
    };
};
#pragma pack()

typedef code[1024]      code_page;

