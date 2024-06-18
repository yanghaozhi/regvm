#pragma once

#include <stdint.h>

#pragma pack(1)
typedef struct
{
    uint8_t         id;
    struct
    {
        uint8_t     ex      : 4;
        uint8_t     reg     : 4;
    };
} code_t;

#pragma pack()

enum DATA_TYPE
{
    TYPE_NULL       = 0,
    TYPE_SIGNED     = 1,
    TYPE_UNSIGNED   = 2,
    TYPE_DOUBLE     = 3,
    TYPE_STRING     = 4,
    TYPE_DICT       = 5,
    TYPE_LIST       = 6,
    TYPE_ADDR       = 7,
};


//+---------+---------------+-----------------------+-----------------------+
//| NAME    | ID            | REG                   | EX                    |
//+---------+---------------+-----------------------+-----------------------+
//| NOP     | CODE_NOP      | N/A                   | ignore number of data |
//  空指令，ex表示跳过接下来的指令数目
//+---------+---------------+-----------------------+-----------------------+
//| TRAP    | CODE_TRAP     | N/A                   | EX                    |
//  产生一个中断，reg和ex会带入回调参数中
//+---------+---------------+-----------------------+-----------------------+
//| SETS    | CODE_SETS     | reg                   | type of data          |
//| SETI    | CODE_SETI     | reg                   | type of data          |
//| SETL    | CODE_SETL     | reg                   | type of data          |
//  把随后跟随的指令作为立即数存入$reg中
//      S: 2byte(16bit)
//      I: 4byte(32bit)
//      L: 8byte(64bit)
//+---------+---------------+-----------------------+-----------------------+
//| MOVE    | CODE_MOVE     | reg                   | ex                    |
//  $reg = $ex
//+---------+---------------+-----------------------+-----------------------+
//| CLEAR   | CODE_CLEAR    | reg                   | type                  |
//  $reg = 0
//+---------+---------------+-----------------------+-----------------------+
//| LOAD    | CODE_LOAD     | reg                   | reg stores var name   |
//  把变量的值加载到$reg中，变量名存于$ex里
//+---------+---------------+-----------------------+-----------------------+
//| STORE   | CODE_STORE    | reg                   | reg stores var name   |
//  把数据回写变量中
//      如果ex == reg，则把reg的值回写到原始加载的变量中
//      如果ex != reg，则新变量名存于ex内
//      type==4是局部变量，type==0x0C是全局变量enum CODE_ID
//+---------+---------------+-----------------------+-----------------------+
//| BLOCK   | CODE_BLOCK    | N/A                   | enter or leave        |
//  ex == 0 means enter block, ex == 1 means exit block
//+---------+---------------+-----------------------+-----------------------+
//| CALL    | CODE_CALL     | function entry        | arg count             |
//  调用函数，函数入口存于$reg，
//      ex表示参数个数（从0开始依次存在于寄存器中，最多16个）
//      如果$reg的类型为TYPE_ADDR，则它直接是跳转地址
//      如果$reg的类型为TYPE_SIGNED，则表明它是函数id
//+---------+---------------+-----------------------+-----------------------+
//| RET     | CODE_RET      | N/A                   | return value count    |
//  退出函数，ex表示返回值个数（方法如同CALL的参数）
//+---------+---------------+-----------------------+-----------------------+
//| CMD     | CODE_CMD      | reg                   | ex                    |
//  执行内置命令，命令id为：reg << 4 + ex（一共255）
//+---------+---------------+-----------------------+-----------------------+
//| INC     | CODE_INC      | reg                   | ex                    |
//  $reg += ex
//+---------+---------------+-----------------------+-----------------------+
//| DEC     | CODE_DEC      | reg                   | ex                    |
//  $reg -= ex
//+---------+---------------+-----------------------+-----------------------+
//| ADD     | CODE_ADD      | reg                   | ex                    |
//  $reg += $ex
//+---------+---------------+-----------------------+-----------------------+
//| SUB     | CODE_SUB      | reg                   | ex                    |
//  $reg -= $ex
//+---------+---------------+-----------------------+-----------------------+
//| MUL     | CODE_MUL      | reg                   | ex                    |
//  $reg *= $ex
//+---------+---------------+-----------------------+-----------------------+
//| DIV     | CODE_DIV      | reg                   | ex                    |
//  $reg /= $ex
//+---------+---------------+-----------------------+-----------------------+
//| AND     | CODE_AND      | reg                   | ex                    |
//  $reg &= $ex
//+---------+---------------+-----------------------+-----------------------+
//| OR      | CODE_OR       | reg                   | ex                    |
//  $reg |= $ex
//+---------+---------------+-----------------------+-----------------------+
//| XOR     | CODE_XOR      | reg                   | ex                    |
//  $reg ^= $ex
//+---------+---------------+-----------------------+-----------------------+
//| SHL     | CODE_SHL      | reg                   | ex                    |
//  $reg <<= $ex
//+---------+---------------+-----------------------+-----------------------+
//| SHR     | CODE_SHR      | reg                   | ex                    |
//  $reg >>= $ex
//+---------+---------------+-----------------------+-----------------------+
//| CONV    | CODE_CONV     | reg                   | type                  |
//  把$reg的类型转换为type
//+---------+---------------+-----------------------+-----------------------+
//| TYPE    | CODE_TYPE     | reg                   | ex                    |
//  把$ex的类型的值存入$reg中（类型为TYPE_UNSIGNED）
//+---------+---------------+-----------------------+-----------------------+
//| CHG     | CODE_CHG      | reg                   | op                    |
//  根据op，对$reg进行操作：
//      0: 清零
//      1: 负数
//      2: 倒数
//      3: NOT（按位取反）
//+---------+---------------+-----------------------+-----------------------+
//| JUMP    | CODE_JUMP     | N/A                   | dest                  |
//  无条件跳转，跳转目的地址存于$dest中
//      相对偏移，跳转地址的类型为TYPE_SIGNED（正负表示跳转方向）
//      绝对偏移，跳转地址的类型为TYPE_ADDR
//+---------+---------------+-----------------------+-----------------------+
//| JZ      | CODE_JZ       | reg                   | dest                  |
//  if ($reg == 0) jump
//+---------+---------------+-----------------------+-----------------------+
//| JNZ     | CODE_JNZ      | reg                   | dest                  |
//  if ($reg != 0) jump
//+---------+---------------+-----------------------+-----------------------+
//| JG      | CODE_JG       | reg                   | dest                  |
//  if ($reg > 0) jump
//+---------+---------------+-----------------------+-----------------------+
//| JL      | CODE_JL       | reg                   | dest                  |
//  if ($reg < 0) jump
//+---------+---------------+-----------------------+-----------------------+
//| JNG     | CODE_JNG      | reg                   | dest                  |
//  if ($reg <= 0) jump
//+---------+---------------+-----------------------+-----------------------+
//| JNL     | CODE_JNL      | reg                   | dest                  |
//  if ($reg >= 0) jump
//+---------+---------------+-----------------------+-----------------------+

//扩展指令（32bit长度）
//+---------+---------------+-----------------------+-----------------------+
//| STR     | CODE_STR      | reg                   | op code               |
//| 字符串操作              | a1        | a2        | a3        | a4        |
//          | 0 : len       | result    | N/A       | N/A       | N/A       |
//          | 1 : substr    | result    | $start    | $len      | N/A       |
//+---------+---------------+-----------------------+-----------------------+

//+---------+---------------+-----------------------+-----------------------+
//| EXIT    | CODE_EXIT     | reg                   | ex                    |
//  CODE_EXIT   = 255,  //退出执行, exitcode = (ex == 0) ? 0 : $reg
//+---------+---------------+-----------------------+-----------------------+


enum CODE_ID
{
    CODE_NOP    = 0,
    CODE_TRAP,
    CODE_SETS,
    CODE_SETI,
    CODE_SETL,
    CODE_MOVE,
    CODE_CLEAR,
    CODE_LOAD,
    CODE_STORE,
    CODE_BLOCK,
    CODE_CALL,
    CODE_RET,
    CODE_CMD,
    CODE_INC,
    CODE_DEC,
    CODE_ADD,
    CODE_SUB,
    CODE_MUL,
    CODE_DIV,
    CODE_AND,
    CODE_OR,
    CODE_XOR,
    CODE_SHL,
    CODE_SHR,
    CODE_CONV,
    CODE_TYPE,
    CODE_CHG,
    CODE_JUMP,
    CODE_JZ,
    CODE_JNZ,
    CODE_JG,
    CODE_JL,
    CODE_JNG,
    CODE_JNL,

    CODE_EXIT   = 255,
};


