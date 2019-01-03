import os
from itertools import islice 

path ="./"
files =os.listdir(path) 
files.sort(key = os.path.getmtime)
s= []
for filename in files:
    if not os.path.isdir(path +filename) and filename.endswith(".txt") and filename.startswith("AP"):
        f_name = str(filename)
        s.append(f_name) 
#print(s)

out = open("output.txt", "w")
cpt = 0
for filename in s:
	cpt += 1
	out.write("Out put for AP"+ str(cpt) + "\n")
	fp = open(filename, "r")
	for line in islice(fp, 1, None):
		out.write(line)
	fp.close()

out.close()
