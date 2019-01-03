import dpkt
import socket
import sys
import datetime
import os

f = open(sys.argv[1])
pcap = dpkt.pcap.Reader(f)


outname = os.path.splitext(sys.argv[1])[0] + ".txt"
#print outname
out = open(outname, 'w')
out.write("output file for " + os.path.splitext(sys.argv[1])[0] + "\n")
out.close()

out = open(outname, 'a')

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

