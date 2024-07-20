local n = 100

local a, b = 0, 1
for i = 2, n do
    local c = a + b
    a = b
    b = c
    print(a, b, c)
end

print(b)
