fR = 3.904173
def comp(tA, tB, f):
    rT = (tB - tA) / 5e6
    rF = f / fR

    print "(f: %f, t: %f) inv(f: %f, t: %f)" % (rF, rT, 1. / rF, 1. / rT)

while True:
    f = float(raw_input("Freq? "))
    b = int(raw_input("TimeB? "))
    a = int(raw_input("TimeA? "))

    comp(a,b,f)
