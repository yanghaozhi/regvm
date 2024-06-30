#pragma once

#include <stdint.h>
//#include <type_traits>


///////////////////////////////////////////////////////

#define MLIB_CAT(a, b)             MLIB_CAT_HELPER(a, b)
#define MLIB_CAT_HELPER(a, b)      a ## b

#define MLIB_CAT2(a, b)            MLIB_CAT2_HELPER(a, b)
#define MLIB_CAT2_HELPER(a, b)     a ## b

#define MLIB_CAT3(a, b)            MLIB_CAT3_HELPER(a, b)
#define MLIB_CAT3_HELPER(a, b)     a ## b

//use this to workaround a bug of msvc : https://stackoverflow.com/questions/5134523/msvc-doesnt-expand-va-args-correctly
#define MLIB_EXPAND(x)             x

#define MLIB_DEC(n) MLIB_DEC_##n
#define MLIB_INC(n) MLIB_INC_##n

#define MLIB_DEC_1     0
#define MLIB_DEC_2     1
#define MLIB_DEC_3     2
#define MLIB_DEC_4     3
#define MLIB_DEC_5     4
#define MLIB_DEC_6     5
#define MLIB_DEC_7     6
#define MLIB_DEC_8     7
#define MLIB_DEC_9     8
#define MLIB_DEC_10    9

#define MLIB_INC_0     1
#define MLIB_INC_1     2
#define MLIB_INC_2     3
#define MLIB_INC_3     4
#define MLIB_INC_4     5
#define MLIB_INC_5     6
#define MLIB_INC_6     7
#define MLIB_INC_7     8
#define MLIB_INC_8     9
#define MLIB_INC_9     10

///////////////////////////////////////////////////////

//max count is 10 !!!
#define MLIB_MULTI_0_EXT(op, count, ...)                   MLIB_MULTI_IMPL(MLIB_MULTI_0_EXT_, op, count, __VA_ARGS__)

#define MLIB_MULTI_1_EXT(op, ext1, count, ...)             MLIB_MULTI_IMPL(MLIB_MULTI_1_EXT_, op, count, ext1, __VA_ARGS__)

#define MLIB_MULTI_2_EXT(op, ext1, ext2, count, ...)       MLIB_MULTI_IMPL(MLIB_MULTI_2_EXT_, op, count, ext1, ext2, __VA_ARGS__)

#define MLIB_MULTI_3_EXT(op, ext1, ext2, ext3, count, ...) MLIB_MULTI_IMPL(MLIB_MULTI_3_EXT_, op, count, ext1, ext2, ext3, __VA_ARGS__)



#define MLIB_MULTI_IMPL_0(pre, op, count, ...) MLIB_EXPAND(MLIB_CAT(pre, MLIB_DEC(count))(MLIB_DEC(count), pre, op, __VA_ARGS__))
#define MLIB_MULTI_IMPL_1(pre, op, count, ...) 

#define MLIB_MULTI_IMPL(pre, op, count, ...)   MLIB_CAT(MLIB_MULTI_IMPL_, MLIB_BOOL(MLIB_CHECK_ZERO_, count))(pre, op, count, __VA_ARGS__)

///////////////////////////////////////////////////////

#define MLIB_MULTI_0_EXT_0(id, pre, op, x, ...)   op(id, x)
#define MLIB_MULTI_0_EXT_1(id, pre, op, x, ...)   op(id, x) MLIB_EXPAND(MLIB_CAT(pre, MLIB_DEC(id))(MLIB_DEC(id), pre, op, __VA_ARGS__))
#define MLIB_MULTI_0_EXT_2(id, pre, op, x, ...)   op(id, x) MLIB_EXPAND(MLIB_CAT(pre, MLIB_DEC(id))(MLIB_DEC(id), pre, op, __VA_ARGS__))
#define MLIB_MULTI_0_EXT_3(id, pre, op, x, ...)   op(id, x) MLIB_EXPAND(MLIB_CAT(pre, MLIB_DEC(id))(MLIB_DEC(id), pre, op, __VA_ARGS__))
#define MLIB_MULTI_0_EXT_4(id, pre, op, x, ...)   op(id, x) MLIB_EXPAND(MLIB_CAT(pre, MLIB_DEC(id))(MLIB_DEC(id), pre, op, __VA_ARGS__))
#define MLIB_MULTI_0_EXT_5(id, pre, op, x, ...)   op(id, x) MLIB_EXPAND(MLIB_CAT(pre, MLIB_DEC(id))(MLIB_DEC(id), pre, op, __VA_ARGS__))
#define MLIB_MULTI_0_EXT_6(id, pre, op, x, ...)   op(id, x) MLIB_EXPAND(MLIB_CAT(pre, MLIB_DEC(id))(MLIB_DEC(id), pre, op, __VA_ARGS__))
#define MLIB_MULTI_0_EXT_7(id, pre, op, x, ...)   op(id, x) MLIB_EXPAND(MLIB_CAT(pre, MLIB_DEC(id))(MLIB_DEC(id), pre, op, __VA_ARGS__))
#define MLIB_MULTI_0_EXT_8(id, pre, op, x, ...)   op(id, x) MLIB_EXPAND(MLIB_CAT(pre, MLIB_DEC(id))(MLIB_DEC(id), pre, op, __VA_ARGS__))
#define MLIB_MULTI_0_EXT_9(id, pre, op, x, ...)   op(id, x) MLIB_EXPAND(MLIB_CAT(pre, MLIB_DEC(id))(MLIB_DEC(id), pre, op, __VA_ARGS__))

///////////////////////////////////////////////////////

#define MLIB_MULTI_1_EXT_0(id, pre, op, e1, x, ...)   op(e1, id, x)
#define MLIB_MULTI_1_EXT_1(id, pre, op, e1, x, ...)   op(e1, id, x) MLIB_EXPAND(MLIB_CAT(pre, MLIB_DEC(id))(MLIB_DEC(id), pre, op, e1, __VA_ARGS__))
#define MLIB_MULTI_1_EXT_2(id, pre, op, e1, x, ...)   op(e1, id, x) MLIB_EXPAND(MLIB_CAT(pre, MLIB_DEC(id))(MLIB_DEC(id), pre, op, e1, __VA_ARGS__))
#define MLIB_MULTI_1_EXT_3(id, pre, op, e1, x, ...)   op(e1, id, x) MLIB_EXPAND(MLIB_CAT(pre, MLIB_DEC(id))(MLIB_DEC(id), pre, op, e1, __VA_ARGS__))
#define MLIB_MULTI_1_EXT_4(id, pre, op, e1, x, ...)   op(e1, id, x) MLIB_EXPAND(MLIB_CAT(pre, MLIB_DEC(id))(MLIB_DEC(id), pre, op, e1, __VA_ARGS__))
#define MLIB_MULTI_1_EXT_5(id, pre, op, e1, x, ...)   op(e1, id, x) MLIB_EXPAND(MLIB_CAT(pre, MLIB_DEC(id))(MLIB_DEC(id), pre, op, e1, __VA_ARGS__))
#define MLIB_MULTI_1_EXT_6(id, pre, op, e1, x, ...)   op(e1, id, x) MLIB_EXPAND(MLIB_CAT(pre, MLIB_DEC(id))(MLIB_DEC(id), pre, op, e1, __VA_ARGS__))
#define MLIB_MULTI_1_EXT_7(id, pre, op, e1, x, ...)   op(e1, id, x) MLIB_EXPAND(MLIB_CAT(pre, MLIB_DEC(id))(MLIB_DEC(id), pre, op, e1, __VA_ARGS__))
#define MLIB_MULTI_1_EXT_8(id, pre, op, e1, x, ...)   op(e1, id, x) MLIB_EXPAND(MLIB_CAT(pre, MLIB_DEC(id))(MLIB_DEC(id), pre, op, e1, __VA_ARGS__))
#define MLIB_MULTI_1_EXT_9(id, pre, op, e1, x, ...)   op(e1, id, x) MLIB_EXPAND(MLIB_CAT(pre, MLIB_DEC(id))(MLIB_DEC(id), pre, op, e1, __VA_ARGS__))

///////////////////////////////////////////////////////

#define MLIB_MULTI_2_EXT_0(id, pre, op, e1, e2, x, ...)   op(e1, e2, id, x)
#define MLIB_MULTI_2_EXT_1(id, pre, op, e1, e2, x, ...)   op(e1, e2, id, x) MLIB_EXPAND(MLIB_CAT(pre, MLIB_DEC(id))(MLIB_DEC(id), pre, op, e1, e2, __VA_ARGS__))
#define MLIB_MULTI_2_EXT_2(id, pre, op, e1, e2, x, ...)   op(e1, e2, id, x) MLIB_EXPAND(MLIB_CAT(pre, MLIB_DEC(id))(MLIB_DEC(id), pre, op, e1, e2, __VA_ARGS__))
#define MLIB_MULTI_2_EXT_3(id, pre, op, e1, e2, x, ...)   op(e1, e2, id, x) MLIB_EXPAND(MLIB_CAT(pre, MLIB_DEC(id))(MLIB_DEC(id), pre, op, e1, e2, __VA_ARGS__))
#define MLIB_MULTI_2_EXT_4(id, pre, op, e1, e2, x, ...)   op(e1, e2, id, x) MLIB_EXPAND(MLIB_CAT(pre, MLIB_DEC(id))(MLIB_DEC(id), pre, op, e1, e2, __VA_ARGS__))
#define MLIB_MULTI_2_EXT_5(id, pre, op, e1, e2, x, ...)   op(e1, e2, id, x) MLIB_EXPAND(MLIB_CAT(pre, MLIB_DEC(id))(MLIB_DEC(id), pre, op, e1, e2, __VA_ARGS__))
#define MLIB_MULTI_2_EXT_6(id, pre, op, e1, e2, x, ...)   op(e1, e2, id, x) MLIB_EXPAND(MLIB_CAT(pre, MLIB_DEC(id))(MLIB_DEC(id), pre, op, e1, e2, __VA_ARGS__))
#define MLIB_MULTI_2_EXT_7(id, pre, op, e1, e2, x, ...)   op(e1, e2, id, x) MLIB_EXPAND(MLIB_CAT(pre, MLIB_DEC(id))(MLIB_DEC(id), pre, op, e1, e2, __VA_ARGS__))
#define MLIB_MULTI_2_EXT_8(id, pre, op, e1, e2, x, ...)   op(e1, e2, id, x) MLIB_EXPAND(MLIB_CAT(pre, MLIB_DEC(id))(MLIB_DEC(id), pre, op, e1, e2, __VA_ARGS__))
#define MLIB_MULTI_2_EXT_9(id, pre, op, e1, e2, x, ...)   op(e1, e2, id, x) MLIB_EXPAND(MLIB_CAT(pre, MLIB_DEC(id))(MLIB_DEC(id), pre, op, e1, e2, __VA_ARGS__))

///////////////////////////////////////////////////////

#define MLIB_MULTI_3_EXT_0(id, pre, op, e1, e2, e3, x, ...)   op(e1, e2, e3, id, x)
#define MLIB_MULTI_3_EXT_1(id, pre, op, e1, e2, e3, x, ...)   op(e1, e2, e3, id, x) MLIB_EXPAND(MLIB_CAT(pre, MLIB_DEC(id))(MLIB_DEC(id), pre, op, e1, e2, e3, __VA_ARGS__))
#define MLIB_MULTI_3_EXT_2(id, pre, op, e1, e2, e3, x, ...)   op(e1, e2, e3, id, x) MLIB_EXPAND(MLIB_CAT(pre, MLIB_DEC(id))(MLIB_DEC(id), pre, op, e1, e2, e3, __VA_ARGS__))
#define MLIB_MULTI_3_EXT_3(id, pre, op, e1, e2, e3, x, ...)   op(e1, e2, e3, id, x) MLIB_EXPAND(MLIB_CAT(pre, MLIB_DEC(id))(MLIB_DEC(id), pre, op, e1, e2, e3, __VA_ARGS__))
#define MLIB_MULTI_3_EXT_4(id, pre, op, e1, e2, e3, x, ...)   op(e1, e2, e3, id, x) MLIB_EXPAND(MLIB_CAT(pre, MLIB_DEC(id))(MLIB_DEC(id), pre, op, e1, e2, e3, __VA_ARGS__))
#define MLIB_MULTI_3_EXT_5(id, pre, op, e1, e2, e3, x, ...)   op(e1, e2, e3, id, x) MLIB_EXPAND(MLIB_CAT(pre, MLIB_DEC(id))(MLIB_DEC(id), pre, op, e1, e2, e3, __VA_ARGS__))
#define MLIB_MULTI_3_EXT_6(id, pre, op, e1, e2, e3, x, ...)   op(e1, e2, e3, id, x) MLIB_EXPAND(MLIB_CAT(pre, MLIB_DEC(id))(MLIB_DEC(id), pre, op, e1, e2, e3, __VA_ARGS__))
#define MLIB_MULTI_3_EXT_7(id, pre, op, e1, e2, e3, x, ...)   op(e1, e2, e3, id, x) MLIB_EXPAND(MLIB_CAT(pre, MLIB_DEC(id))(MLIB_DEC(id), pre, op, e1, e2, e3, __VA_ARGS__))
#define MLIB_MULTI_3_EXT_8(id, pre, op, e1, e2, e3, x, ...)   op(e1, e2, e3, id, x) MLIB_EXPAND(MLIB_CAT(pre, MLIB_DEC(id))(MLIB_DEC(id), pre, op, e1, e2, e3, __VA_ARGS__))
#define MLIB_MULTI_3_EXT_9(id, pre, op, e1, e2, e3, x, ...)   op(e1, e2, e3, id, x) MLIB_EXPAND(MLIB_CAT(pre, MLIB_DEC(id))(MLIB_DEC(id), pre, op, e1, e2, e3, __VA_ARGS__))


///////////////////////////////////////////////////////

#define MLIB_NONE
#define MLIB_CONST                  const

#define MLIB_CHECK_ZERO_0           1, 1
#define MLIB_CHECK_VOID_void        1, 1

#define MLIB_CHECK_IMPL(x, n, ...)  n
#define MLIB_CHECK(...)             MLIB_EXPAND(MLIB_CHECK_IMPL(__VA_ARGS__))     //conv ('0, 0', 1) to (0, 0, 1)
#define MLIB_BOOL(m, x)             MLIB_CHECK(MLIB_CAT(m, x), 0)    //if (x == 0) {CHECK(0, 0, 1)} else {CHECK(CHECK_ZERO_x, 1)}

#define MLIB_NOT_0                  1
#define MLIB_NOT_1                  0
#define MLIB_NOT(x)                 MLIB_CAT(MLIB_NOT_, x)

#define MLIB_ZERO(x)                MLIB_BOOL(MLIB_CHECK_ZERO_, x)
#define MLIB_NOT_ZERO(x)            MLIB_NOT(MLIB_ZERO(x))

#define MLIB_COMMA_0
#define MLIB_COMMA_1                ,


///////////////////////////////////////////////////////

#define MLIB_DECL_GEN(id, x)       x v ##  id MLIB_CAT(MLIB_COMMA_, MLIB_NOT_ZERO(id))
#define MLIB_DECL_LIST(c, ...)     MLIB_MULTI_0_EXT(MLIB_DECL_GEN, c, __VA_ARGS__)

#define MLIB_CALL_GEN(id, x)       v ## id  MLIB_CAT(MLIB_COMMA_, MLIB_NOT_ZERO(id))
#define MLIB_CALL_LIST(c, ...)     MLIB_MULTI_0_EXT(MLIB_CALL_GEN, c, __VA_ARGS__)

///////////////////////////////////////////////////////

