import sys
import scipy.stats

try:
    reg_file=sys.argv[1]
except IndexError:
    print "usage: %s REG_DATA_FILE" % (sys.argv[0],)
    sys.exit(1)

simtimes_x=[]
cputimes_y=[]

f = open(reg_file)
for line in f:
    fields=line.split(' ')
    try:
        fields[0] = float(fields[0])
        fields[1] = float(fields[1])
    except:
        print "parsing error on line:"
        print line
        sys.exit(1)

    simtimes_x.append(fields[0])
    cputimes_y.append(fields[1])

slope, intercept, r_value, p_value, std_err = scipy.stats.linregress(simtimes_x, cputimes_y)

print "# SLOP INTERCEPT R_VALUE P_VALUE STDERR"
print slope, intercept, r_value, p_value, std_err
