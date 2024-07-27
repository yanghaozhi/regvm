#pragma once

#include <stdint.h>

#pragma pack(1)
typedef union
{
    uint32_t        value;
    struct
    {
        uint8_t     id      : 8;
        uint8_t     a       : 8;
        uint8_t     b       : 8;
        uint8_t     c       : 8;
    };
    struct
    {
        int8_t     ids      : 8;
        int8_t     as       : 8;
        int8_t     bs       : 8;
        int8_t     cs       : 8;
    };
    struct
    {
        int8_t     id2      : 8;
        int8_t     a2       : 8;
        int16_t    b2       : 16;
    };
    struct
    {
        int8_t     id3      : 8;
        int32_t    a3       : 24;
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

//每条指令长度为32bit
//系统内共有 0 - 255 共256个寄存器
//
//+---------+---------------+-----------------------+-----------------------+-----------------------+
//| NAME    | ID            | A                     | B                     | C                     |
//+---------+---------------+-----------------------+-----------------------+-----------------------+
//| NOP     | CODE_NOP      | ignore number of data |                       |                       |
//  空指令，ex表示跳过接下来的指令数目
//+---------+---------------+-----------------------+-----------------------+-----------------------+
//| TRAP    | CODE_TRAP     | N/A                   | N/A                   | N/A                   |
//  产生一个中断，reg和ex会带入回调参数中
//+---------+---------------+-----------------------+-----------------------+-----------------------+
//| DATA    | CODE_DATA     |                                                                       |
//  上一条非DATA指令的附加数据
//+---------+---------------+-----------------------+-----------------------+-----------------------+
//| SET     | CODE_SET      | reg                   | type of data          | lowest byte of data   |
//  设置$reg寄存器的值和类型, 超过1字节的数据则在后面附加DATA指令
//+---------+---------------+-----------------------+-----------------------+-----------------------+
//| MOVE    | CODE_MOVE     | dst                   | src                   |                       |
//  $dst = $src
//+---------+---------------+-----------------------+-----------------------+-----------------------+
//| CLEAR   | CODE_CLEAR    | reg                   | type                  |                       |
//  重置寄存器的变量为默认值，类型为type
//      type == TYPE_STRING :   $reg = ""
//           == TYPE_DICT :     $reg = DICT{}
//           == TYPE_LIST :     $reg = LIST{}
//+---------+---------------+-----------------------+-----------------------+-----------------------+
//| LOAD    | CODE_LOAD     | reg                   | reg stores var name   |                       |
//  把变量的值加载到$reg中，变量名存于$ex里
//+---------+---------------+-----------------------+-----------------------+-----------------------+
//| STORE   | CODE_STORE    | reg                   | name                  | attr                  |
//  把$reg的数据回写到变量。attr的值含义：
//      0   ：原始加载的变量（如无，则无操作）
//      1   ：写入到变量中，变量名存于$name中（如改变量不存在，则在当前scope内创建新的变量）
//      2   : 写入到新的变量中（不查询上级scope的同名变量，如本级scope找不到则新建）
//      3   ：只在顶层scope（全局变量）中查找或新建
//+---------+---------------+-----------------------+-----------------------+-----------------------+
//| BLOCK   | CODE_BLOCK    | N/A                   | enter or leave        |                       |
//  ex == 0 means enter block, ex == 1 means exit block
//+---------+---------------+-----------------------+-----------------------+-----------------------+
//| CALL    | CODE_CALL     | entry                 | arg                   |                       |
//  调用函数，函数入口存于$entry，
//      ex表示参数个数（从0开始依次存在于寄存器中，最多16个）
//      如果$reg的类型为TYPE_ADDR，则它直接是跳转地址
//      如果$reg的类型为TYPE_SIGNED，则表明它是函数id
//+---------+---------------+-----------------------+-----------------------+-----------------------+
//| RET     | CODE_RET      | N/A                   | return value count    |                       |
//  退出函数，ex表示返回值个数（方法如同CALL的参数）
//+---------+---------------+-----------------------+-----------------------+-----------------------+
//| INC     | CODE_INC      | a                     | b                     | result                |
//  $result = $a + b ( -128 <= b <= 127 )
//+---------+---------------+-----------------------+-----------------------+-----------------------+
//| ADD     | CODE_ADD      | a                     | b                     | result                |
//  $result = $a + $b                                                                                
//+---------+---------------+-----------------------+-----------------------+-----------------------+
//| SUB     | CODE_SUB      | a                     | b                     | result                |
//  $result = $a - $b                                                                                
//+---------+---------------+-----------------------+-----------------------+-----------------------+
//| MUL     | CODE_MUL      | a                     | b                     | result                |
//  $result = $a * $b                                                                                
//+---------+---------------+-----------------------+-----------------------+-----------------------+
//| DIV     | CODE_DIV      | a                     | b                     | result                |
//  $result = $a / $b                                                                                
//+---------+---------------+-----------------------+-----------------------+-----------------------+
//| MOD     | CODE_MOD      | a                     | b                     | result                |
//  $result = $a % $b                                                                                
//+---------+---------------+-----------------------+-----------------------+-----------------------+
//| AND     | CODE_AND      | a                     | b                     | result                |
//  $result = $a & $b                                                                                
//+---------+---------------+-----------------------+-----------------------+-----------------------+
//| OR     | CODE_OR        | a                     | b                     | result                |
//  $result = $a | $b                                                                                
//+---------+---------------+-----------------------+-----------------------+-----------------------+
//| XOR     | CODE_XOR      | a                     | b                     | result                |
//  $result = $a ^ $b
//+---------+---------------+-----------------------+-----------------------+-----------------------+
//| SHL     | CODE_SHL      | a                     | b                     | result                |
//  $result = $a << $b
//+---------+---------------+-----------------------+-----------------------+-----------------------+
//| SHR     | CODE_SHR      | a                     | b                     | result                |
//  $result = $a >> $b
//+---------+---------------+-----------------------+-----------------------+-----------------------+
//| SHIFT   | CODE_SHIFT    | a                     | bits                  | result                |
//  $result = (bits > 0) ? $a << bits : $a >> -(bits)
//+---------+---------------+-----------------------+-----------------------+-----------------------+
//| JUMP    | CODE_JUMP     | dest                                                                  |
//  无条件跳转，跳转目的地址为dest（正负表示跳转方向）
//+---------+---------------+-----------------------+-----------------------+-----------------------+
//| JEQ      | CODE_JEQ     | a                     | b                     | dest                  |
//  if ($a == $b) jump $dest                                                                         
//+---------+---------------+-----------------------+-----------------------+-----------------------+
//| JNE     | CODE_JNE      | a                     | b                     | dest                  |
//  if ($a != $b) jump $dest                                                                         
//+---------+---------------+-----------------------+-----------------------+-----------------------+
//| JGT     | CODE_JGT      | a                     | b                     | dest                  |
//  if ($a > $b) jump $dest                                                                          
//+---------+---------------+-----------------------+-----------------------+-----------------------+
//| JGE     | CODE_JGE      | a                     | b                     | dest                  |
//  if ($a >= $b) jump $dest                                                                         
//+---------+---------------+-----------------------+-----------------------+-----------------------+
//| JLT     | CODE_JLT      | a                     | b                     | dest                  |
//  if ($a < $b) jump $dest                                                                          
//+---------+---------------+-----------------------+-----------------------+-----------------------+
//| JLE     | CODE_JLE      | a                     | b                     | dest                  |
//  if ($a <= $b) jump $dest
//+---------+---------------+-----------------------+-----------------------+-----------------------+
//| CONV    | CODE_CONV     | result                | src                   | type                  |
//  把$src的类型转换为type后存入$result中
//+---------+---------------+-----------------------+-----------------------+-----------------------+
//| TYPE    | CODE_TYPE     | result                | ex                    |                       |
//  把$ex的类型的值存入$result中（类型为TYPE_SIGNED）
//+---------+---------------+-----------------------+-----------------------+-----------------------+
//| CHG     | CODE_CHG      | result                | src                   | op                    |
//  根据op，对$src进行操作：
//       0  : 负数
//       1  : 倒数
//       2  : NOT（按位取反）
//       3  : malloc（本地保存字符串副本）
//+---------+---------------+-----------------------+-----------------------+-----------------------+
//| CMP     | CODE_CMP      | result                | src                   | op                    |
//  根据op，对$reg进行操作，结果永远为1/0：
//      0  : $reg == 0
//      1  : $reg != 0
//      2  : $reg > 0
//      3  : $reg >= 0
//      4  : $reg < 0
//      5  : $reg <= 0
//+---------+---------------+-----------------------+-----------------------+-----------------------+


//扩展指令
//+---------+---------------+-----------------------+-----------------------+-----------------------+
//| ECHO    | CODE_ECHO     | a                     | b                     | c                     |
//  打印寄存器的值，$a, $b, $c 依次为需要打印的寄存器（遇到0结束）

//+---------+---------------+-----------------------+-----------------------+-----------------------+
//| SLEN    | CODE_SLEN     | result                | str                   |                       |
//  $result = strlen($str);
//
//+---------+---------------+-----------------------+-----------------------+-----------------------+
//| LLEN    | CODE_LLEN     | result                | list                  |                       |
// $result = $list.size()
//+---------+---------------+-----------------------+-----------------------+-----------------------+
//| LAT     | CODE_LAT      | result                | list                  | pos                   |
// $result = $list[$pos]
//+---------+---------------+-----------------------+-----------------------+-----------------------+
//| LSET    | CODE_LSET     | list                  | pos                   | value                 |
// $list[$pos] = $value
//+---------+---------------+-----------------------+-----------------------+-----------------------+
//| LPUSH   | CODE_LPUSH    | list                  | type                  | value                 |
// (type == 0) ? $list.push_back($value) : $list.push_front($value)
//+---------+---------------+-----------------------+-----------------------+-----------------------+
//| LPOP    | CODE_LPOP     | list                  | type                  |                       |
// (type == 0) ? $list.pop_back($value) : $list.pop_front($value)
//+---------+---------------+-----------------------+-----------------------+-----------------------+
//| LERASE  | CODE_LERASE   | list                  | pos                   |                       |
// $list.erase($pos)
//
//+---------+---------------+-----------------------+-----------------------+-----------------------+
//| DLEN    | CODE_DLEN     | result                | dict                  |                       |
// $result = $dict.size()
//+---------+---------------+-----------------------+-----------------------+-----------------------+
//| DGET    | CODE_DGET     | result                | dict                  | key                   |
// $result = $dict[$key]
//+---------+---------------+-----------------------+-----------------------+-----------------------+
//| DSET    | CODE_DSET     | dict                  | key                   | value                 |
// $dict[$key] = $value
//+---------+---------------+-----------------------+-----------------------+-----------------------+
//| DDEL    | CODE_DDEL     | dict                  | key                   |                       |
// $dict.erase($key)
//+---------+---------------+-----------------------+-----------------------+-----------------------+
//| DHAS    | CODE_DHAS     | result                | dict                  | key                   |
// $result = $dict.contains($key)
//+---------+---------------+-----------------------+-----------------------+-----------------------+
//| DITEMS  | CODE_DITEMS   | dict                  | keys                  | values                |
// $keys = $dict.keys()
// $values = $dict.values()

//+---------+---------------+-----------------------+-----------------------+-----------------------+
//| EXIT    | CODE_EXIT     | reg                   | code                                          |
//  CODE_EXIT   = 255,  //退出执行, exitcode = (reg != 255) ? $reg : code
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
    CODE_DATA,
    CODE_MOVE,
    CODE_CHG,
    CODE_CMP,
    CODE_TYPE,
    CODE_INC,
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
    CODE_SHIFT,
    CODE_JUMP,
    CODE_JEQ,
    CODE_JNE,
    CODE_JGT,
    CODE_JGE,
    CODE_JLT,
    CODE_JLE,



    CODE_TRAP   = 64,
    CODE_SET,
    CODE_LOAD,
    CODE_CLEAR,
    CODE_STORE,
    CODE_BLOCK,
    CODE_CONV,
    CODE_CALL,
    CODE_RET,

    CODE_ECHO,
    CODE_SLEN,

    CODE_LLEN,
    CODE_LAT,
    CODE_LSET,
    CODE_LPUSH,
    CODE_LPOP,
    CODE_LERASE,

    CODE_DLEN,
    CODE_DGET,
    CODE_DSET,
    CODE_DDEL,
    CODE_DHAS,
    CODE_DITEMS,

    CODE_EXIT   = 255,
};


