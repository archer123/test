import dpkt
import socket
import sys
import datetime
import os


path ="./"
f = open(sys.argv[1])

pcap = dpkt.pcap.Reader(f)


#get name of current file
(name, extension) = f.os.path.spilt(f)

#get the name list in time order
files =os.listdir(path)
files.sort(key = os.path.getmtime)
s= []
for filename in files:
    if not os.path.isdir(path +filename) and filename.endswith(".pcap") and filename.startswith("AP") and filename.without("_f"):
        (f_name, extension) = filename.os.path.spilt(filename)
        s.append(f_name)
print s

#get cptAP
cptAP = 0
found = False
while not found and cptAP < len(s):
	found = (s[cptAP] in name)
	cptAP +=1

outname = "output.txt"

#print outname
if not os.path.exists(outname):
	out = open(outname, 'w')
	out.write("AP" + str(cptAP) + "\n")
else:
	out = open(outname, 'a+')
	out.write("AP"+ str(cptAP) + "\n")



#counter for goodput for all
cpt = 0
first = 0

#counter for goodput per second
cptbuffer = 0
cptsec = 0
firstsec = 0
lastsec = 0
flag = True

for ts, buf in pcap:
	eth = dpkt.ethernet.Ethernet(buf)
	ip = eth.data
	tcp = ip.data
	#print datetime.datetime.utcfromtimestamp(ts), len(tcp.data)
	if cpt == 0:
		#get first time time of incoming data
		first = ts
		cpt += len(tcp.data)
	else:
		#get last time time of incoming data
		time = ts
		cpt += len(tcp.data)

	if cptsec == 0 and flag:
		firstsec = ts
		cptsec += len(tcp.data)
		flag = False
	else:
		lastsec = ts
		cptsec += len(tcp.data)
		flag = (lastsec - firstsec) >= 1
		if flag:
			info =  "From time "+ str(firstsec) + " to "+ str(lastsec) + " the goodput is "+ str( (8 * cptsec/(lastsec - firstsec))/1000000)+ "mbits/s"
			#print info
			out.write(info + "\n")
			cptsec = 0


#print cpt, time
info = "goodput = " + str((8 * cpt/(time - first))/1000000) + " mbits/s, during " + str(time - first) + "s"
out.write(info + "\n")

f.close()
out.close()
