#!/usr/local/bin/python3

n = 10000000

pi = 0
for i in range(n) :
    if i % 2 == 0 :
        pi += (1 / (i * 2 + 1))
    else :
        pi -= (1 / (i * 2 + 1))

print(pi * 4)
