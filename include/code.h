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
//  重置寄存器的变量为默认值，类型为type
//      type == TYPE_STRING :   $reg = NULL
//           == TYPE_DICT :     $reg = DICT{}
//           == TYPE_LIST :     $reg = LIST{}
//+---------+---------------+-----------------------+-----------------------+
//| LOAD    | CODE_LOAD     | reg                   | reg stores var name   |
//  把变量的值加载到$reg中，变量名存于$ex里
//+---------+---------------+-----------------------+-----------------------+
//| STORE   | CODE_STORE    | reg                   | reg stores var name   |
//  把数据回写变量中
//      如果ex == reg，则把reg的值回写到原始加载的变量中
//      如果ex != reg，则新变量名存于ex内
//+---------+---------------+-----------------------+-----------------------+
//| GLOBAL  | CODE_STORE    | reg                   | reg stores var name   |
//  把数据写入全局变量中
//      变量名存于ex内
//+---------+---------------+-----------------------+-----------------------+
//| NEW     | CODE_NEW      | reg                   | type                  |
//  在当前scope中创建新的变量，变量名位于$reg内，ex则为变量类型
//  变量的值为默认值，具体情况和CLEAR一样
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
//| MOD     | CODE_MOD      | reg                   | ex                    |
//  $reg %= $ex
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
//      0   : 清零
//      1   : 负数
//      2   : 倒数
//      3   : NOT（按位取反）
//      4   : malloc（本地保存字符串副本）
//      5   : $reg == 0
//      6   : $reg != 0
//      7   : $reg > 0
//      8   : $reg >= 0
//      9   : $reg < 0
//      10  : $reg <= 0
//+---------+---------------+-----------------------+-----------------------+
//| JUMP    | CODE_JUMP     | N/A                   | dest                  |
//  无条件跳转，跳转目的地址存于$dest中
//      相对偏移，跳转地址的类型为TYPE_SIGNED（正负表示跳转方向）
//      绝对偏移，跳转地址的类型为TYPE_ADDR
//+---------+---------------+-----------------------+-----------------------+
//| JNZ      | CODE_JNZ       | reg                   | dest                |
//  if ($reg != 0) jump
//+---------+---------------+-----------------------+-----------------------+

//扩展指令（32bit长度）
//+---------+---------------+-----------------------+-----------------------+
//| CMD     | CODE_CMD      | return value          | cmd id                |
//| 调用内置命令            | a1        | a2        | a3        | a4        |
//  id  0   : 打印变量的值 | 变量个数  | 依次为存放变量的寄存器            |
//+---------+---------------+-----------------------+-----------------------+
//| STR     | CODE_STR      | return value          | op code               |
//| 字符串操作              | a1        | a2        | a3        | a4        |
//          | 0 : len       | str       | N/A       | N/A       | N/A       |
//          | 1 : substr    | str       | $start    | $len      | N/A       |
//+---------+---------------+-----------------------+-----------------------+
//| LIST    | CODE_LIST     | return value          | op code               |
//| 字符串操作              | a1        | a2        | a3        | a4        |
//          | 0 : len       | list      | N/A       | N/A       | N/A       |
//          | 1 : at        | list      | $idx      | N/A       | N/A       |
//          type 0 : back，1 : front
//          | 2 : push      | list      | type      | $value    | N/A       |
//          | 3 : pop       | list      | type      | N/A       | N/A       |
//          | 4 : insert    | list      | $idx      | $value    | N/A       |
//          | 5 : erase     | list      | $idx      | N/A       | N/A       |
//          | 6 : set       | list      | $idx      | $value    | N/A       |
//          cmp 0 : greater, 1 : lesser
//          | 7 : sort      | list      | cmp       | N/A       | N/A       |
//+---------+---------------+-----------------------+-----------------------+
//| DICT    | CODE_DICT     | return value          | op code               |
//| 字符串操作              | a1        | a2        | a3        | a4        |
//          | 0 : len       | dict      | N/A       | N/A       | N/A       |
//          | 1 : set       | dict      | $key      | $value    | N/A       |
//          | 2 : get       | dict      | $key      | N/A       | N/A       |
//          | 3 : del       | dict      | $key      | N/A       | N/A       |
//          | 4 : has       | dict      | $key      | N/A       | N/A       |
//          type 0 : only keys, 1 : only values, 2 : keys and values
//          output two lists, a3 store keys, a4 store values, 
//          they are in same order, means dict[$keys[i]] == $values[i]
//          return len of dict, also the len of keys / values
//          | 5 : items     | dict      | type      | keys      | values    |
//+---------+---------------+-----------------------+-----------------------+

//+---------+---------------+-----------------------+-----------------------+
//| EXIT    | CODE_EXIT     | reg                   | ex                    |
//  CODE_EXIT   = 255,  //退出执行, exitcode = (ex == 0) ? 0 : $reg
//+---------+---------------+-----------------------+-----------------------+


//指令长度计算规则
//指令为0的，是NOP指令，根据其内容决定长度
//指令最高位为0的，其长度为2 byte（共64条）
//指令最高位为1的，则其后带有扩展数据
//最高位为10xxxxx的，带有2 byte扩展（共64条）
//最高位为110xxxx的，带有4 byte扩展（共32条）
//最高位为111xxxx的，带有8 byte扩展（共32条）

enum CODE_ID
{
    CODE_NOP    = 0,
    CODE_TRAP,
    CODE_MOVE,
    CODE_CLEAR,
    CODE_LOAD,
    CODE_STORE,
    CODE_GLOBAL,
    CODE_NEW,
    CODE_BLOCK,
    CODE_CALL,
    CODE_RET,
    CODE_INC,
    CODE_DEC,
    CODE_ADD,
    CODE_SUB,
    CODE_MUL,
    CODE_DIV,
    CODE_MOD,
    CODE_AND,
    CODE_OR,
    CODE_XOR,
    CODE_SHL,
    CODE_SHR,
    CODE_CONV,
    CODE_TYPE,
    CODE_CHG,
    CODE_CMP,
    CODE_JUMP,
    CODE_JNZ,

    CODE_SETS   = 128,      //2 byte
    CODE_CMD,
    CODE_STR,
    CODE_LIST,
    CODE_DICT,

    CODE_SETI   = 192,      //4 byte

    CODE_SETL   = 224,      //8 byte

    CODE_EXIT   = 255,
};


