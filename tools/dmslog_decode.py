#!/usr/bin/env python3
"""Validador/decoder streaming do DMSLOG2. Sem dependências externas."""
import argparse,csv,datetime as dt,struct,sys,zlib
MAGIC=b"DMSLOG2"; SYNC=0xD25A
def records(fp):
    header=fp.read(64)
    if len(header)<64 or not header.startswith(MAGIC): raise ValueError("cabeçalho DMSLOG2 inválido/truncado")
    while True:
        pos=fp.tell(); h=fp.read(8)
        if not h:return
        if len(h)<8: print(f"registro final incompleto em {pos}",file=sys.stderr);return
        sync,kind,version,length=struct.unpack("<HBBI",h)
        if sync!=SYNC or length>65536: raise ValueError(f"enquadramento inválido em {pos}")
        payload=fp.read(length); crc=fp.read(4)
        if len(payload)!=length or len(crc)!=4: print(f"registro final incompleto em {pos}",file=sys.stderr);return
        expected,=struct.unpack("<I",crc)
        if zlib.crc32(h+payload)&0xffffffff!=expected: raise ValueError(f"CRC inválido em {pos}")
        yield kind,version,payload
def main():
    ap=argparse.ArgumentParser();ap.add_argument("file");ap.add_argument("--csv");a=ap.parse_args()
    try:
        with open(a.file,"rb") as f: data=list(records(f))
        print(f"registros íntegros: {len(data)}")
        if a.csv:
            with open(a.csv,"w",newline="",encoding="utf-8") as out:
                w=csv.writer(out);w.writerow(["type","version","payload_hex"]);w.writerows((k,v,p.hex()) for k,v,p in data)
    except (OSError,ValueError) as e: print(e,file=sys.stderr);return 2
    return 0
if __name__=="__main__":raise SystemExit(main())
