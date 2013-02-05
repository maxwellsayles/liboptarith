# reads all .dat files and for each bit size, determines the file
# with the fastest time.
import csv
from glob import glob

files = glob('*.dat')
bestTime = {}  # map bits to best time
bestFile = {}  # map bits to best file
for f in files:
    with open(f, 'r') as csvfile:
        reader = csv.reader(csvfile)
        for row in reader:
            b = int(row[0])
            t = float(row[1])
            if b in bestTime:
                if t < bestTime[b]:
                    bestTime[b] = t
                    bestFile[b] = f
            else:
                bestTime[b] = t
                bestFile[b] = f

for b in sorted(bestTime.keys()):
    print b, bestTime[b], bestFile[b]
