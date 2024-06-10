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
    TYPE_OTHER      = 0,
    TYPE_SIGNED     = 1,
    TYPE_UNSIGNED   = 2,
    TYPE_DOUBLE     = 3,
    TYPE_STRING     = 4,
};
//8 <= DATA_TYPE <= 15 means user defined

enum CODE_ID
{
//+-------------+---------------------------+---------------------------+
//| NAME        | REG                       | EX                        |
//| NOP         | N/A                       | ignore number of codes    |
    CODE_NOP    = 0,    //空指令，ex表示跳过接下来的指令数目
//+-------------+---------------------------+---------------------------+
//| TRAP        | N/A                       | EX                        |
    CODE_TRAP,          //产生一个中断，reg和ex会带入回调参数中
//+-------------+---------------------------+---------------------------+
//| SET         | reg                       | type of data              |
    CODE_SETS,          //把随后跟随的指令作为立即数存入$reg中
    CODE_SETI,          //S: 2byte(16bit), I: 4byte(32bit), L: 8byte(64bit)
    CODE_SETL,          //
//+-------------+---------------------------+---------------------------+
//| MOVE        | reg                       | ex                        |
    CODE_MOVE,          //$reg = $ex
//+-------------+---------------------------+---------------------------+
//| CLEAR       | reg                       | type                      |
    CODE_CLEAR,         //$reg = 0
//+-------------+---------------------------+---------------------------+
//| LOAD        | reg                       | reg stores var name       |
    CODE_LOAD,          //把变量的值加载到$reg中，变量名存于$ex里
//+-------------+---------------------------+---------------------------+
//| STORE       | reg                       | reg stores var name       |
    CODE_STORE,         //如果ex为0，则把reg的值回写到原始加载的变量中
                        //如果ex不为0，则新变量名存于ex内
                        //type==4是局部变量，type==0x0C是全局变量
//+-------------+---------------------------+---------------------------+
//| BLOCK       | N/A                       | type                      |
    CODE_BLOCK,         //ex 0 means enter block, 1 means exit block
//+-------------+---------------------------+---------------------------+
//| CALL        | function entry            | arg count                 |
    CODE_CALL,          //调用函数，函数入口存于$reg，
                        //ex则表示参数个数（从0开始依次存在于寄存器中，最多16个）
//+-------------+---------------------------+---------------------------+
//| RET         | N/A                       | return value count        |
    CODE_RET,           //退出函数，ex表示返回值个数（方法如同CALL的参数）

//+-------------+---------------------------+---------------------------+
//| CMD         | reg                       | ex                        |
    CODE_CMD,           //执行内置命令，命令id为：reg << 4 + ex（一共255）

//+-------------+---------------------------+---------------------------+
//| INC         | reg                       | ex                        |
    CODE_INC,           //$reg += ex
//+-------------+---------------------------+---------------------------+
//| DEC         | reg                       | ex                        |
    CODE_DEC,           //$reg -= ex
//+-------------+---------------------------+---------------------------+
//| ADD         | reg                       | ex                        |
    CODE_ADD,           //$reg += $ex
//+-------------+---------------------------+---------------------------+
//| SUB         | reg                       | ex                        |
    CODE_SUB,           //$reg -= $ex
//+-------------+---------------------------+---------------------------+
//| MUL         | reg                       | ex                        |
    CODE_MUL,           //$reg *= $ex
//+-------------+---------------------------+---------------------------+
//| DIV         | reg                       | ex                        |
    CODE_DIV,           //$reg /= $ex
//+-------------+---------------------------+---------------------------+
//| AND         | reg                       | ex                        |
    CODE_AND,           //$reg &= $ex
//+-------------+---------------------------+---------------------------+
//| OR          | reg                       | ex                        |
    CODE_OR,            //$reg |= $ex
//+-------------+---------------------------+---------------------------+
//| XOR         | reg                       | ex                        |
    CODE_XOR,           //$reg ^= $ex
//+-------------+---------------------------+---------------------------+
//| CONV        | reg                       | type                      |
    CODE_CONV,          //把$reg的类型转换为type
//+-------------+---------------------------+---------------------------+
//| CHG         | reg                       | op                        |
    CODE_CHG,           //根据op，对$reg进行操作：
                        //0: 清零
                        //1: 负数
                        //2: 倒数
                        //3: NOT（按位取反）

//+-------------+---------------------------+---------------------------+
//| JUMP        | N/A                       | dest                      |
    CODE_JUMP,          //无条件跳转，跳转目的地址存于$dest中
                        //相对偏移，跳转地址的类型为0x08（正负表示跳转方向）
                        //绝对偏移，跳转地址的类型为0x09
//+-------------+---------------------------+---------------------------+
//| JZ          | reg                       | dest                      |
    CODE_JZ,            //if ($reg == 0) jump
//+-------------+---------------------------+---------------------------+
//| JNZ         | reg                       | dest                      |
    CODE_JNZ,           //if ($reg != 0) jump
//+-------------+---------------------------+---------------------------+
//| JG          | reg                       | dest                      |
    CODE_JG,            //if ($reg > 0) jump
//+-------------+---------------------------+---------------------------+
//| JL          | reg                       | dest                      |
    CODE_JL,            //if ($reg < 0) jump
//+-------------+---------------------------+---------------------------+
//| JNG         | reg                       | dest                      |
    CODE_JNG,           //if ($reg <= 0) jump
//+-------------+---------------------------+---------------------------+
//| JNL         | reg                       | dest                      |
    CODE_JNL,           //if ($reg >= 0) jump


//+-------------+---------------------------+---------------------------+
//| EXIT         | reg                      | ex                        |
    CODE_EXIT   = 255,  //退出执行, exitcode = (ex == 0) ? 0 : $reg
};


