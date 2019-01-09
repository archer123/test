import dpkt
import socket
import sys
import datetime
import os
import struct


server =sys.argv[1]

path = "./"
files = os.listdir(path)
files.sort()

s= []
for filename in files:
    if not os.path.isdir(path +filename) and filename.endswith(".pcap") and filename.startswith("AP"):
        f_name = str(filename)
        s.append(f_name)
print(s)


out = open("output.txt", "w")
out.write("id\tbegin\tend\tgoodput\n")

cptid = 0
cptcli = 0
cptsec = 0

gettimebegin = True
tsbegin = 0
tsend = 0


port = []
ack = []
ackinit = []


for f in s:
	cptid += 1
	fp = open(f)
	pcap = dpkt.pcap.Reader(fp)
	for ts, buf in pcap:
		#print ts, len(buf)
		if gettimebegin:
			tsbegin = ts
			gettimebegin = False
		tsend = ts
		eth = dpkt.ethernet.Ethernet(buf)
		ip = eth.data
		tcp = ip.data

		if socket.inet_ntoa(ip.dst) == server:

			#print ts, len(tcp.data)
			#cptcli += len(tcp.data)

			if ( tcp.flags == dpkt.tcp.TH_SYN):
				if (tcp.sport not in port):
					
					port.append(tcp.sport)
					ackinit.append(0)
					ack.append(0)
				
				
				ack[port.index(tcp.sport)] = tcp.seq + 1
				ackinit[port.index(tcp.sport)] = tcp.seq + 1
			

		else :
			if (tcp.flags == dpkt.tcp.TH_ACK) :
				if ack[port.index(tcp.dport)] < tcp.ack:
					ack[port.index(tcp.dport)] = tcp.ack

	total = 0
	for i in range(len(ack)):
		total += (ack[i] - ackinit[i])
	strcli = str(cptid) + "\t" + str(tsbegin) + "\t" + str(tsend) + "\t" + str(total*8/((tsend - tsbegin)*1000000)) + "\n\n"
	out.write(strcli)
	cptcli = 0

	gettimebegin = True
	tsbegin = 0
	tsend = 0
	lsitseq = []
	ack = []
	ackinit = []
	prot = []

out.close()
