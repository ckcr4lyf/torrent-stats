import socket
import sys

iname = "domains.txt"
oname = "ips.txt"

if (len(sys.argv) > 1):
    iname = sys.argv[1]

if (len(sys.argv) > 2):
    oname = sys.argv[2]

f = open(iname, "r")
gg = open(oname, "w")

for tracker in f:
    if (len(tracker.strip()) > 5):
        protocol = tracker[:6]
        domain = ""
        port = ""
        full  = ""
        if (protocol == "udp://"):
            pos = tracker.find('/', 6)
            if (pos != -1):
                full = tracker[6:pos]
            else:
                full = tracker[6:]
            dpos = full.find(':')
            domain = full[:dpos]
            port = full[dpos+1:]
            try:
                ln = socket.gethostbyname(domain) + ":" + port
                gg.write(ln)
                gg.write('\n')
            except:
                print "Kaand ", domain