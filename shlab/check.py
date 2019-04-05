import os

n = input()

if(int(n) < 10):
    n = '0' + n
os.system("make test" + n + " > res")
os.system('make rtest' + n + " > ref")
