#pragma once

#include <stdint.h>

#pragma pack(1)
typedef struct
{
    uint8_t             id;
    union
    {
        struct
        {
            uint8_t     type    : 4;
            uint8_t     reg     : 4;
        };
        struct
        {
            int8_t      a1      : 4;
            int8_t      a2      : 4;
        };
    };
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
            int16_t     v1     : 8;
            int16_t     v2     : 8;
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

enum CODE_ID
{
//+---------+-------+-------+---------------+---------------+---------------+
//| NAME    | TYPE  | REG   | value16       | value32       | value64       |
    EXIT    = 0,    //退出程序
//+---------+-------+-------+---------------+---------------+---------------+
//| SET     | type  | reg   | N/A           | N/A           | trap function |
    TRAP,   //extra for debugger, func type is trap_callback
//+---------+-------+-------+---------------+---------------+---------------+
//| SET     | type  | reg   | for SET2      | for SET4      | fot SET8      |
    SET2,   //set short int value $type:$value to $reg
    SET4,   //set short int value $type:$value to $reg
    SET8,   //set short int value $type:$value to $reg
//+---------+-------+-------+---------------+---------------+---------------+
//| LOAD    | N/A   | dest  | N/A           | N/A           | var name      |
    LOAD,   //把var name的值加载到reg中
//+---------+-------+-------+---------------+---------------+---------------+
//| STORE   | N/A   | from  | N/A           | N/A           | N/A           |
    STORE,  //把reg的值回写
//+---------+-------+-------+---------------+---------------+---------------+
//| STORE8  | N/A   | from  | N/A           | N/A           | var name      |
    STORE8, //把reg的值写入指定的var name
//+---------+-------+-------+---------------+---------------+---------------+
//| BLOCK   | N/A   | N/A   | N/A           | N/A           | N/A           |
    BLOCK,  //v1 0 means enter block, 1 means exit block

//+---------+-------+-------+---------------+---------------+---------------+
//| INC     | value | reg   | N/A           | N/A           | N/A           |
    INC,    //reg 的值增加 value @ a1（可正可负）
//+---------+-------+-------+---------------+---------------+---------------+
//| INC     | value | reg   | regs          | N/A           | N/A           |
    ADD,    //$reg = $r1 + $r2 + $r3 + $r4
//+---------+-------+-------+---------------+---------------+---------------+
//| MUL     | value | reg   | regs          | N/A           | N/A           |
    MUL,    //$reg = $r1 * $r2 * $r3 * $r4
//+---------+-------+-------+---------------+---------------+---------------+
//| CONV    | type  | reg   | N/A           | N/A           | N/A           |
    CONV,   //把$reg的类型转换为type
//+---------+-------+-------+---------------+---------------+---------------+
//| CHG     | value | reg   | N/A           | N/A           | N/A           |
    CHG,    //根据type，对$reg进行操作：0取反，1倒数

//+---------+-------+-------+---------------+---------------+---------------+
//| JUMP    | value | reg   | N/A           | dest          | N/A           |
    JUMP,   //根据type，对$reg与0进行判断后的结果进行跳转：
            //0无条件跳，1大于，2大于等于，3小于0，4小于等于0，5等于0，6不等于0


};


