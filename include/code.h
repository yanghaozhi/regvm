#pragma once

#include <stdint.h>

#pragma pack(1)
struct code
{
    struct 
    {
        uint32_t        name    : 12;
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

enum CODE_NAME
{
//+---------+-------+-------+-------+-------------------+--------------------------------------+
//| NAME    | TYPE  | REG   | EXT   | VALUE             |  USAGE                               |
    NOP         = 0,
//+---------+-------+-------+-------+-------------------+--------------------------------------+
//| SET     | type  | to    | N/A   | value             |  set $type:$value to $reg            |
    SET,        //
//+---------+-------+-------+-------+-------------------+--------------------------------------+
//| LOAD    | N/A   | to    | N/A   | var name          |  load var to $reg                    |
    LOAD,       //
//+---------+-------+-------+-------+-------------------+--------------------------------------+
//| STORE   | N/A   | from  | N/A   | var name          |  store $reg to var                   |
    STORE,       //
};


