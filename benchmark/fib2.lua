local n = 50

local a, b = 0, 1
for i = 2, n do
    a, b = b, a + b
end

print(b)

