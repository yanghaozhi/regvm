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

    T get()     {return used(0);}

    bool add(T v)
    {
        if (size >= max) 
        {
            return false;
        }
        datas[size++] = v;
        return true;
    }

    T remove(void)
    {
        T v = used(0);
        if (v >= 0)
        {
            size -= 1;
        }
        return v;
    }

    void remove(T v)
    {
        if (active(v) >= 0)
        {
            size -= 1;
        }
    }

    T active(T v)
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

    T used(int id)
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

