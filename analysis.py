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
out.write("id\tserver/client\tbegin\tend\tgoodput\n")

cptid = 0
cptcli = 0
cptser = 0

gettimebegin = True
tsbegin = 0
tsend = 0

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
		if len(tcp.data) > 0 and not (str(tcp.seq) in listseq):
			listseq.append(str(tcp.seq))
		#print (socket.inet_ntoa(ip.dst) == server)
			if socket.inet_ntoa(ip.dst) == server:
				#print ts, len(tcp.data)
				cptcli += len(tcp.data)
			else :
				#print ts, len(tcp.data)
				cptser += len(tcp.data)

	
	strcli = str(cptid) + "\t" + "client" + "\t" + str(tsbegin) + "\t" + str(tsend) + "\t" + str( 8* cptcli/((tsend - tsbegin)*1000000)) + "\n"
	strser = str(cptid) + "\t" + "server" + "\t" + str(tsbegin) + "\t" + str(tsend) + "\t" + str( 8* cptser/((tsend - tsbegin)*1000000)) + "\n\n"
	out.write(strcli)
	out.write(strser)

	cptcli = 0
	cptser = 0

	gettimebegin = True
	tsbegin = 0
	tsend = 0
	lsitseq = []

out.close()

