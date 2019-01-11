import dpkt
import socket
import sys
import datetime
import os
import struct
from openpyxl import Workbook



def main():
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


	#out = open("output.txt", "w")
	#out.write("id\tbegin\tend\tgoodput\n")

	wb = Workbook()
	ws = wb.active
	
	
	
	args = ["id", "begin", "end", "goodput"]
	ws.append(args)

	cptid = 0
	cptcli = 0
	cptsec = 0

	gettimebegin = True
	tsbegin = 0
	tsend = 0


	port = []
	ack = []
	ackinit = []

	secbegin = 0
	secend = 0
	acksec = []
	acksecinit = []


	for f in s:
		cptid += 1
		fp = open(f)
		pcap = dpkt.pcap.Reader(fp)
		wb.create_sheet("AP" + str(cptid))
		flagsec = True

		#every time we find a new AP, we construct a new sheet to store the goodput for every second
		newsheet = wb.get_sheet_by_name("AP" + str(cptid))
		args = ["begin", "end", "goodput"]
		newsheet.append(args)
		totalsec = 0
		for ts, buf in pcap:

			#
			if gettimebegin:
				tsbegin = ts
				secbegin = ts 
				gettimebegin = False
			tsend = ts
			secend = ts
			eth = dpkt.ethernet.Ethernet(buf)
			ip = eth.data
			tcp = ip.data


			flagsec = (secend - secbegin) >= 1
			if socket.inet_ntoa(ip.dst) == server:

				#find out which ports the program use by SYN
				if ( tcp.flags == dpkt.tcp.TH_SYN):
					if (tcp.sport not in port):
						
						port.append(tcp.sport)
						ackinit.append(0)
						ack.append(0)
						acksec.append(0)
						acksecinit.append(0)

								
					ack[port.index(tcp.sport)] = tcp.seq + 1
					ackinit[port.index(tcp.sport)] = tcp.seq + 1
					acksec[port.index(tcp.sport)] = tcp.seq + 1
					acksecinit[port.index(tcp.sport)] = tcp.seq + 1
				
			else :

				#check every ts if the ack is the bigest in the pcap
				if (tcp.flags == dpkt.tcp.TH_ACK) :
					if ack[port.index(tcp.dport)] < tcp.ack:
						ack[port.index(tcp.dport)] = tcp.ack
						if flagsec:
							for i in range(len(acksec)):
								totalsec += (acksec[i] - acksecinit[i])
							args = [str(secbegin), str(secend), str(totalsec*8/((secend - secbegin)*1000000))]
							newsheet.append(args)
							secend = ts
							secbegin = ts
							acksecinit[port.index(tcp.dport)] = tcp.ack
							acksec[port.index(tcp.dport)] = tcp.ack
							totalsec = 0
						else:
							acksec[port.index(tcp.dport)] = tcp.ack
							secend = ts

			#bool for every second to check if the new second begin
			
		total = 0
		for i in range(len(ack)):
			total += (ack[i] - ackinit[i])

		#
		args = [str(cptid), str(tsbegin), str(tsend), str(total*8/((tsend - tsbegin)*1000000))]
		ws.append(args)
		cptcli = 0

		gettimebegin = True
		tsbegin = 0
		tsend = 0
		lsitseq = []
		ack = []
		ackinit = []
		port = []
		acksec = []
		acksecinit = []
		secend = 0
		secbegin = 0

	#out.close()
	wb.save("output.xlsx")

if __name__ == "__main__":
	main()