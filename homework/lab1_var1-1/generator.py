from random import randint, choice
from string import ascii_lowercase, digits

n = 10000000
alphabet = ascii_lowercase + digits
for _ in range(n):
    k = randint(0, 65535)
    v = "".join(choice(alphabet) for _ in range(randint(1, 64)))
    print(f"{k}\t{v}")
