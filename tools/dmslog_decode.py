#!/usr/bin/env python3
"""Streaming validator and physical CSV exporter for DMSLOG2."""
import argparse,csv,datetime as dt,json,struct,sys,zlib
MAGIC=b"DMSLOG2";HEADER=64;SYNC=0xD25A
TYPES={1:"MASTER_SAMPLE",2:"POLL_ERROR",3:"SLAVE_SAMPLE",4:"PANEL_TRANSACTION",5:"RAW_JBD_FRAME",6:"LINK_EVENT",7:"SYSTEM_EVENT",8:"TIME_EVENT"}
SAMPLE_HEADER=struct.Struct("<IQIIIIiqIIBBBBHHHHHHHHHBBIIIIII32s")
SAMPLE_FIELDS=["record_type","timestamp_iso","timestamp_ms","monotonic_ms","sequence","age_ms","quality","pack_mV","current_mA","power_mW","rmcap_mAh","nominal_mAh","rsoc","fets","cycles","production_raw","protection","balance_low","balance_high","min_mV","max_mV","average_mV","delta_mV","min_index","max_index","poll_us","timeouts","checksum_errors","discarded_frames","link_errors","dropped_logs","hardware","cell_count","ntc_count"]+[f"temp_{i:02d}_dC" for i in range(1,9)]+[f"cell_{i:02d}_mV" for i in range(1,33)]
TX_FIELDS=["monotonic_ms","latency_ms","sequence","age_ms","command","flags","request_hex","response_hex","payload_hex"]
def iso(ms):return dt.datetime.fromtimestamp(ms/1000,dt.timezone.utc).isoformat().replace("+00:00","Z") if ms else ""
def read_header(fp):
 h=fp.read(HEADER)
 if len(h)!=HEADER or h[:7]!=MAGIC or h[7]!=2:raise ValueError("cabeçalho DMSLOG2 inválido")
 if zlib.crc32(h[:60])&0xffffffff!=struct.unpack_from("<I",h,60)[0]:raise ValueError("CRC do cabeçalho inválido")
 return {"version":h[7],"role":h[8],"clock_quality":h[9],"schema":struct.unpack_from("<H",h,10)[0],"boot_id":struct.unpack_from("<I",h,12)[0],"session_id":struct.unpack_from("<I",h,16)[0],"start_epoch_ms":struct.unpack_from("<Q",h,20)[0],"device":h[28:40].split(b"\0",1)[0].decode("ascii","replace"),"firmware":h[40:48].split(b"\0",1)[0].decode("ascii","replace"),"git_sha":h[48:60].split(b"\0",1)[0].decode("ascii","replace")}
def records(fp):
 while True:
  pos=fp.tell();h=fp.read(8)
  if not h:return
  if len(h)<8:print(f"registro final truncado em {pos}",file=sys.stderr);return
  sync,kind,version,length=struct.unpack("<HBBI",h)
  if sync!=SYNC or length>65536:raise ValueError(f"enquadramento inválido em {pos}")
  payload=fp.read(length);crc=fp.read(4)
  if len(payload)!=length or len(crc)!=4:print(f"registro final truncado em {pos}",file=sys.stderr);return
  if zlib.crc32(h+payload)&0xffffffff!=struct.unpack("<I",crc)[0]:raise ValueError(f"CRC inválido no meio do arquivo em {pos}")
  yield pos,kind,version,payload
def sample(payload):
 if len(payload)<SAMPLE_HEADER.size:raise ValueError("sample curto")
 values=SAMPLE_HEADER.unpack_from(payload);(seq,ts,mono,age,quality,pack,current,power,rmcap,nominal,rsoc,fets,cells,ntcs,cycles,production,protection,balance_low,balance_high,min_mv,max_mv,average_mv,delta_mv,min_index,max_index,poll_us,timeouts,checksum_errors,discarded_frames,link_errors,dropped_logs,hardware)=values
 if cells>32 or ntcs>8 or len(payload)!=SAMPLE_HEADER.size+2*(cells+ntcs):raise ValueError("contagens inválidas no sample")
 offset=SAMPLE_HEADER.size;temps=list(struct.unpack_from(f"<{ntcs}h",payload,offset)) if ntcs else [];offset+=2*ntcs;cell_values=list(struct.unpack_from(f"<{cells}H",payload,offset)) if cells else []
 row={"timestamp_iso":iso(ts),"timestamp_ms":ts,"monotonic_ms":mono,"sequence":seq,"age_ms":age,"quality":quality,"pack_mV":pack,"current_mA":current,"power_mW":power,"rmcap_mAh":rmcap,"nominal_mAh":nominal,"rsoc":rsoc,"fets":fets,"cycles":cycles,"production_raw":production,"protection":protection,"balance_low":balance_low,"balance_high":balance_high,"min_mV":min_mv,"max_mV":max_mv,"average_mV":average_mv,"delta_mV":delta_mv,"min_index":min_index,"max_index":max_index,"poll_us":poll_us,"timeouts":timeouts,"checksum_errors":checksum_errors,"discarded_frames":discarded_frames,"link_errors":link_errors,"dropped_logs":dropped_logs,"hardware":hardware.split(b"\0",1)[0].decode("ascii","replace"),"cell_count":cells,"ntc_count":ntcs}
 row.update({f"temp_{i+1:02d}_dC":v for i,v in enumerate(temps)});row.update({f"cell_{i+1:02d}_mV":v for i,v in enumerate(cell_values)});return row
def transaction(payload):
 if len(payload)<22:return {"payload_hex":payload.hex()}
 mono,latency,seq,age=struct.unpack_from("<IIII",payload);cmd,flags=struct.unpack_from("<BB",payload,16);offset=18;request_length=struct.unpack_from("<H",payload,offset)[0];offset+=2
 if offset+request_length+2>len(payload):return {"payload_hex":payload.hex()}
 request=payload[offset:offset+request_length];offset+=request_length;response_length=struct.unpack_from("<H",payload,offset)[0];offset+=2;response=payload[offset:offset+response_length]
 return {"monotonic_ms":mono,"latency_ms":latency,"sequence":seq,"age_ms":age,"command":f"0x{cmd:02X}","flags":flags,"request_hex":request.hex(),"response_hex":response.hex(),"payload_hex":payload.hex()}
def main():
 parser=argparse.ArgumentParser();parser.add_argument("file");parser.add_argument("--csv");parser.add_argument("--transactions");parser.add_argument("--unknown",help="JSONL com registros desconhecidos preservados em hexadecimal");args=parser.parse_args();counts={};first=last=None;sample_file=tx_file=unknown_file=None
 try:
  if args.csv:sample_file=open(args.csv,"w",newline="",encoding="utf-8");sample_writer=csv.DictWriter(sample_file,SAMPLE_FIELDS,extrasaction="ignore");sample_writer.writeheader()
  else:sample_writer=None
  if args.transactions:tx_file=open(args.transactions,"w",newline="",encoding="utf-8");tx_writer=csv.DictWriter(tx_file,TX_FIELDS,extrasaction="ignore");tx_writer.writeheader()
  else:tx_writer=None
  if args.unknown:unknown_file=open(args.unknown,"w",encoding="utf-8")
  with open(args.file,"rb") as source:
   header=read_header(source)
   for position,kind,version,payload in records(source):
    name=TYPES.get(kind,f"UNKNOWN_{kind}");counts[name]=counts.get(name,0)+1
    if kind in (1,3):
     row=sample(payload);row["record_type"]=name;first=first or row["timestamp_ms"];last=row["timestamp_ms"]
     if sample_writer:sample_writer.writerow(row)
    elif kind==4 and tx_writer:tx_writer.writerow(transaction(payload))
    elif kind not in TYPES and unknown_file:unknown_file.write(json.dumps({"offset":position,"type":kind,"version":version,"payload_hex":payload.hex()})+"\n")
  print(json.dumps({"header":header,"records":counts,"first_timestamp":iso(first or 0),"last_timestamp":iso(last or 0)},ensure_ascii=False,indent=2))
 except (OSError,ValueError,struct.error) as error:print(error,file=sys.stderr);return 2
 finally:
  if sample_file:sample_file.close()
  if tx_file:tx_file.close()
  if unknown_file:unknown_file.close()
 return 0
if __name__=="__main__":raise SystemExit(main())
