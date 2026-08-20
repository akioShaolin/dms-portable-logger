#!/usr/bin/env python3
import argparse,struct,time
def crc16(d):
 c=0xffff
 for b in d:
  c^=b<<8
  for _ in range(8):c=((c<<1)^0x1021)&0xffff if c&0x8000 else (c<<1)&0xffff
 return c
def cobs_encode(d):
 o=bytearray(b'\0');code=1;at=0
 for b in d:
  if b==0:o[at]=code;at=len(o);o.append(0);code=1
  else:o.append(b);code+=1
  if code==255:o[at]=code;at=len(o);o.append(0);code=1
 o[at]=code;return bytes(o)
def cobs_decode(d):
 o=bytearray();i=0
 while i<len(d):
  c=d[i];i+=1
  if not c or i+c-1>len(d):raise ValueError('COBS')
  o+=d[i:i+c-1];i+=c-1
  if c<255 and i<len(d):o.append(0)
 return bytes(o)
def frame(t,tx,p=b'',status=0,flags=0,bad=False):
 r=struct.pack('<2sBBBBHH',b'DM',1,t,flags,status,tx,len(p))+p;c=crc16(r)^(1 if bad else 0);return cobs_encode(r+struct.pack('<H',c))+b'\0'
def decode(w):
 r=cobs_decode(w.rstrip(b'\0'))
 if len(r)<12 or crc16(r[:-2])!=struct.unpack_from('<H',r,len(r)-2)[0]:raise ValueError('CRC')
 magic,ver,t,flags,status,tx,n=struct.unpack_from('<2sBBBBHH',r);return t,tx,r[10:10+n]
def jbd(cmd,p=b''):
 r=bytearray((0xdd,cmd,0,len(p)))+p;c=(-sum(r[2:]))&0xffff;return bytes(r+bytes((c>>8,c&255,0x77)))
def sample(stale=False):
 cells=[3300+i for i in range(24)];age=4000 if stale else 20;return struct.pack('<IQIIIIiqIIBBBBHHHHHHHHHBBIIIIII32s',42,int(time.time()*1000),1234,age,0x217,80000,-1200,-96000,50000,100000,80,3,24,1,10,0x2068,0,0,0,min(cells),max(cells),sum(cells)//len(cells),max(cells)-min(cells),1,24,70000,0,0,0,0,0,b'SP24S004L24S120A')+struct.pack('<h',250)+struct.pack('<24H',*cells)
def main():
 p=argparse.ArgumentParser();p.add_argument('port');p.add_argument('--baud',type=int,default=230400);p.add_argument('--scenario',choices=['valid','stale','timeout','bad-crc','seq-substituted','write-blocked'],default='valid');p.add_argument('--delay-ms',type=int,default=0);a=p.parse_args()
 try:import serial
 except ImportError:raise SystemExit('instale pyserial')
 cache={}
 with serial.Serial(a.port,a.baud,timeout=.1) as s:
  buf=bytearray()
  while True:
   b=s.read(1)
   if not b:continue
   if b!=b'\0':buf+=b;continue
   try:t,tx,payload=decode(bytes(buf));buf.clear()
   except ValueError:buf.clear();continue
   if tx in cache:s.write(cache[tx]);continue
   if a.scenario=='timeout':continue
   if a.delay_ms:time.sleep(a.delay_ms/1000)
   status=0;flags=1 if a.scenario=='seq-substituted' else 0
   if t==1:out=struct.pack('<BBHII',1,0,1,0x12345678,4194304)
   elif t==7:out=struct.pack('<III',1234,4096,0)
   elif t==6:out=struct.pack('<QBI',int(time.time()*1000),1,1234)
   elif t==2:out=sample(a.scenario=='stale')
   elif t==4:
    cmd=payload[0];raw=jbd(cmd,b'SP24S' if cmd==5 else (b'\x0c\xe4'*24 if cmd==4 else b'\0'*23));out=struct.pack('<IIHH',42,20,flags,len(raw))+raw
   elif t==5:
    n=struct.unpack_from('<H',payload)[0];request=payload[2:2+n]
    if a.scenario=='write-blocked' or len(request)<4 or request[1]==0x5a:status=10;out=b''
    else:cmd=request[2];raw=jbd(cmd,b'\x01');out=struct.pack('<IH',30,len(raw))+raw
   else:status=4;out=b''
   w=frame(t|0x80,tx,out,status,flags,a.scenario=='bad-crc');cache[tx]=w;s.write(w)
if __name__=='__main__':main()
