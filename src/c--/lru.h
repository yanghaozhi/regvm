#pragma once

#include <string.h>

template <typename T, int max> class lru
{
public:
    lru()
    {
        memset(datas, 0xFF, sizeof(datas));
    }

    int     size = 0;

    int get()     {return used(0);}

    int add(T v)  //if return >= 0 means it swap this old one
    {
        if (size >= max) 
        {
            T r = datas[0];
            datas[0] = v;
            used(0);
            return r;
        }
        datas[size++] = v;
        return v;
    }

    int remove(void)
    {
        T v = used(0);
        if (v >= 0)
        {
            size -= 1;
        }
        return v;
    }

    bool remove(T v)
    {
        if (active(v) >= 0)
        {
            size -= 1;
            return true;
        }
        return false;
    }

    int active(T v)
    {
        if constexpr (sizeof(T) == 1)
        {
            T* p = (T*)memrchr(datas, v, size);
            return (p != NULL) ? used(p - datas) : -1;
        }
        else
        {
            for (int i = 0; i < size; i++)
            {
                if (datas[i] == v)
                {
                    return used(i);
                }
            }
            return -1;
        }
    }

private:
    T       datas[max];

    int used(int id)
    {
        if (size == 0) return -1;
        T v = datas[id];
        if (id != size - 1)
        {
            memmove(datas + id, datas + id + 1, (size - id - 1) * sizeof(T));
            datas[size - 1] = v;
        }
        return v;
    }
};

