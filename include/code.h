#pragma once

#include <stdint.h>

#pragma pack(1)
struct code
{
    struct 
    {
        uint32_t        id      : 12;
        uint32_t        type    : 4;
        uint32_t        reg     : 8;
        uint32_t        ext     : 8;
    };
    union
    {
        int64_t         num;
        double          dbl;
        const char*     str;
    }                   value;
};
#pragma pack()

typedef struct code     code_page[1024];

struct src_location
{
    int                 line;
    const char*         file;
    const char*         func;
};

enum CODE_TYPE
{
    NONE        = 0,
    INTEGER,
    DOUBLE,
    STRING,
    BOOL,
    MAP,
    LIST,
};

enum CODE_ID
{
//+---------+-------+-------+-------+-------------------+
//| NAME    | TYPE  | REG   | EXT   | VALUE             |
    NOP         = 0,
//+---------+-------+-------+-------+-------------------+
//| SET     | type  | to    | N/A   | value             |
    SET,        //set $type:$value to $reg
//+---------+-------+-------+-------+-------------------+
//| LOAD    | N/A   | to    | N/A   | var name          |
    LOAD,       //load var as $name to $reg
//+---------+-------+-------+-------+-------------------+
//| STORE   | N/A   | from  | N/A   | N/A               |
    STORE,       //store $reg to it's source var
//+---------+-------+-------+-------+-------------------+
//| STOREN  | N/A   | from  | N/A   | var name          |
    STOREN,      //store $reg to var as $name
};


