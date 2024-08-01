var n = 10000000;

var pi = 0;
for (var i = 0; i < n; i++)
{
    if (i % 2 == 0)
    {
        pi += (1 / (i * 2 + 1))
    }
    else
    {
        pi -= (1 / (i * 2 + 1))
    }
}

console.log(pi * 4)
