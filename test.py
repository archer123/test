import dpkt
import socket
import sys
import datetime
import os


f = open(sys.argv[1])
pcap = dpkt.pcap.Reader(f)


lastseq = 0
for ts, buf in pcap:
    eth = dpkt.ethernet.Ethernet(buf)
    ip = eth.data
    tcp = ip.data
    #print ts, len(tcp.data), tcp.seq
    if lastseq == tcp.seq :
        print "retransimission"
    else :
        lastseq = tcp.seq
