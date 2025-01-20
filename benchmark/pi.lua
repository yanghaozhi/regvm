local n = 10000000

local pi = 0
for i = 0, n do
    if i % 2 == 0 then
        pi = pi + (1 / (i * 2 + 1))
    else
        pi = pi - (1 / (i * 2 + 1))
    end
end

print(pi * 4)
