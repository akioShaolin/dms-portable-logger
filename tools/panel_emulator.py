#!/usr/bin/env python3
import argparse,time
def checksum(data):return (-sum(data))&0xffff
def request(cmd):
    b=bytearray((0xdd,0xa5,cmd,0));c=checksum(b[2:]);return bytes(b+(c>>8,c&255,0x77))
def main():
    p=argparse.ArgumentParser();p.add_argument("port");p.add_argument("--baud",type=int,default=9600);p.add_argument("--command",type=lambda x:int(x,0),default=0x0f);p.add_argument("--capture",default="panel_capture.bin");a=p.parse_args()
    try: import serial
    except ImportError: raise SystemExit("instale pyserial: python -m pip install pyserial")
    with serial.Serial(a.port,a.baud,timeout=.5) as s,open(a.capture,"ab") as cap:
        for cmd in (3,4,5,a.command):
            q=request(cmd);t=time.monotonic();s.write(q);r=s.read_until(b"\x77");ms=(time.monotonic()-t)*1000;cap.write(q+r);print(f"0x{cmd:02X}: {ms:.1f} ms {r.hex(' ')}")
if __name__=="__main__":main()
