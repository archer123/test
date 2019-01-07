import dpkt
import socket
import sys
import datetime
import os
import struct

port = 5000

path = "./"
files = os.listdir(path)
files.sort()

s= []
for filename in files:
    if not os.path.isdir(path +filename) and filename.endswith(".cap"):
        f_name = str(filename)
        s.append(f_name)
print(s)


out = open("output_ack.txt", "w")
out.write("id\tbegin\tend\tgoodput\n")

cptid = 0
cptcli = 0
cptsec = 0

gettimebegin = True
tsbegin = 0
tsend = 0
ack = 0
ackinit = 0

listseq= []
for f in s:
	cptid += 1
	fp = open(f)
	pcap = dpkt.pcap.Reader(fp)
	for ts, buf in pcap:
		if gettimebegin:
			tsbegin = ts
			gettimebegin = False
		tsend = ts
		eth = dpkt.ethernet.Ethernet(buf)
		ip = eth.data
		tcp = ip.data
		#if len(tcp.data) > 0 and not (str(tcp.seq) in listseq):
		#	listseq.append(str(tcp.seq))
		#print (socket.inet_ntoa(ip.dst) == server)
		#	if tcp.dport == port:
				#print ts, len(tcp.data)
		#		cptcli += len(tcp.data)
        #        if cptsec == 0:
        #            secbegin = ts
        #            secend = ts

        #second way by using ack number
        if ( tcp.flags & dpkt.tcp.TH_SYN ) != 0:
            ackinit = tcp.seq + 1
            ack = ackinit
        if ( tcp.flags & dpkt.tcp.TH_ACK ) != 0 and ( tcp.flags & dpkt.tcp.TH_PUSH) == 0 :
            if ack < tcp.ack:
                ack = tcp.ack



	strcli = str(cptid) +  "\t" + str(tsbegin) + "\t" + str(tsend) + "\t" + str( 8*(ack-ackinit)/((tsend - tsbegin)*1000000)) + "\n\n"
	out.write(strcli)

	cptcli = 0

	gettimebegin = True
	tsbegin = 0
	tsend = 0
	lsitseq = []

out.close()
