/*
 * This module is rewrite from c4 project :https://github.com/rswier/c4
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>

#include <map>
#include <list>
#include <vector>
#include <string>
#include <iterator>
#include <string_view>
#include <unordered_map>

#include "inst.h"
#include "common.h"
#include "parser.h"
#include "statements.h"


struct var
{
    DATA_TYPE           type;
    uv                  init;
    std::string_view    name;
};

struct func
{
    DATA_TYPE           ret;
    std::vector<var>    args;
};

std::unordered_map<std::string_view, func> funcs;


bool grammar(std::vector<inst>& insts, const char* src)
{
    printf("%s\n", src);

    parser par;
    decl_var_only   dvo(&par);
    decl_var_init   dvi(&par);

    while ((src != NULL) && (*src != '\0'))
    {
        src = par.statement(src);
    }

    insts.swap(par.insts);
    return (src != NULL) ? true : false;
}



[[maybe_unused]]
static char t1[] = R"(
//int a;
//double b = 12.34;
//int c = -64898;
//double d = b;
//int e = 2 + 6 - 5;
//int f = 2 + (6 - 5) + 9;
//int g = 1 + 2 * 3;
//int h = 10 * 20 - 30;
int i = 6 + 10 * (87 + 51) - 30;
)";


[[maybe_unused]]
static char t2[] = R"(
#include <stdio.h>
int main()
{
    int a;
    a = 1 + 2;
    printf("%d\n", a);
    return 0;
}
)";


int main(int argc, char** argv)
{
    std::vector<inst> insts;
    auto r = grammar(insts, t1);
    printf("grammar : %d\n", r);
    if (r == true)
    {
        for (auto& it : insts)
        {
            //it.print(stdout);
            it.print_txt(stdout);
        }
    }

    //auto r = grammar(t2);
    //printf("grammar : %d\n", r);


    return 0;
}

