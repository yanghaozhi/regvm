#pragma once

#include <stdint.h>

#pragma pack(1)
typedef struct
{
    uint16_t            id      : 9;
    uint16_t            type    : 3;
    uint16_t            reg     : 4;
} code_base_t;

typedef struct
{
    code_base_t         base;
} code0_t;

typedef struct
{
    code_base_t         base;
    union
    {
        uint16_t        num;
        struct 
        {
            uint16_t    v1     : 8;
            uint16_t    v2     : 8;
        };
        struct
        {
            uint16_t    r1     : 4;
            uint16_t    r2     : 4;
            uint16_t    r3     : 4;
            uint16_t    r4     : 4;
        };
    };
} code2_t;

typedef struct
{
    code_base_t         base;
    union
    {
        int32_t         num;
        float           f32;
    };
} code4_t;

typedef struct
{
    code_base_t         base;
    union
    {
        int64_t         num;
        double          dbl;
        const char*     str;
        const void*     other;
    };
} code8_t;
#pragma pack()

typedef code2_t         code_page[1024];

struct src_location
{
    int                 line;
    const char*         file;
    const char*         func;
};

enum CODE_TYPE
{
    OTHER        = 0,
    INTEGER,
    DOUBLE,
    STRING,
    MAP,
    LIST,
};

struct regvm;
typedef int (*trap_callback)(struct regvm* vm, int type, int reg);

//code name with L suffix means it need codex_t

enum CODE_ID
{
//+---------+-------+-------+-------+-------+-------+-------------------+
//| NAME    | TYPE  | REG   | v16   | v1    | v2    | VALUE             |
    NOP     = 0,
//+---------+-------+-------+-------+-------+-------+-------------------+
//| SET     | type  | reg   | value | N/A   | N/A  | callbak function   |
    TRAP,   //extra for debugger, func type is trap_callback
//+---------+-------+-------+-------+-------+-------+-------------------+
//| SET     | type  | reg   | value | N/A   | N/A  | NONE               |
    SET2,   //set short int value $type:$value to $reg
    SET4,   //set short int value $type:$value to $reg
    SET8,   //set short int value $type:$value to $reg
//+---------+-------+-------+-------+-------+-------+-------------------+
//| LOAD    | N/A   | dest  | N/A   | N/A   | N/A  | var name           |
    LOAD,   //load var as $name to reg $dest
//+---------+-------+-------+-------+-------+-------+-------------------+
//| STORE   | N/A   | from  | N/A   | N/A   | N/A  | NONE               |
    STORE,  //writeback reg $from to var which it comes from
//+---------+-------+-------+-------+-------+-------+-------------------+
//| STOREL  | N/A   | from  | N/A   | N/A   | N/A  | var name           |
    STORE8, //store $reg to var as $name
//+---------+-------+-------+-------+-------+-------+-------------------+
//| BLOCK   | N/A   | N/A   | N/A   | op    | N/A  | var name           |
    BLOCK,  //v1 0 means enter block, 1 means exit block
};


