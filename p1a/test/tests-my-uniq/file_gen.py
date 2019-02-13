import random
import string

random.seed(0)
all = []
for i in range(4):
    x = ''
    for i in range(4000):
        x += random.choice(string.ascii_lowercase + ' ')
    all.append(x)

for i in range(20000):
    j = random.randint(0, 100)
    if j < 90:
        print(all[0])
    elif j < 95:
        print(all[1])
    elif j < 98:
        print(all[2])
    else:
        print(all[3])
