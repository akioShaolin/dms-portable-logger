#!/usr/bin/env python3
import argparse,json,time
def checksum(data):return (-sum(data))&0xffff
def request(cmd,write=False,bad=False):
 b=bytearray((0xdd,0x5a if write else 0xa5,cmd,0));c=checksum(b[2:]);b+=bytes((c>>8,c&255,0x77));
 if bad:b[-3]^=1
 return bytes(b)
def exact(s,n):
 b=bytearray()
 while len(b)<n:
  x=s.read(n-len(b))
  if not x:break
  b+=x
 return bytes(b)
def receive(s):
 h=exact(s,4)
 return h+exact(s,h[3]+3) if len(h)==4 else h
def valid(r,cmd):return len(r)>=7 and len(r)==r[3]+7 and r[0]==0xdd and r[1]==cmd and r[-1]==0x77 and checksum(r[2:-3])==int.from_bytes(r[-3:-1],"big")
def main():
 p=argparse.ArgumentParser();p.add_argument("port");p.add_argument("--baud",type=int,default=9600);p.add_argument("--command",type=lambda x:int(x,0));p.add_argument("--bad-checksum",action="store_true");p.add_argument("--write",action="store_true");p.add_argument("--capture",default="panel_capture.jsonl");a=p.parse_args()
 try:import serial
 except ImportError:raise SystemExit("instale pyserial")
 cmds=[a.command] if a.command is not None else [3,4,5,0x0f]
 with serial.Serial(a.port,a.baud,timeout=.6) as s,open(a.capture,"a",encoding="utf-8") as cap:
  for cmd in cmds:
   q=request(cmd,a.write,a.bad_checksum);start=time.monotonic_ns();s.write(q);r=receive(s);ms=(time.monotonic_ns()-start)/1e6;item={"command":cmd,"latency_ms":ms,"request":q.hex(),"response":r.hex(),"valid":valid(r,cmd),"status":r[2] if len(r)>2 else None};cap.write(json.dumps(item)+"\n");print(item)
if __name__=="__main__":main()
