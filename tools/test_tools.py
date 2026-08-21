import importlib.util,struct,tempfile,unittest,zlib,sys
from pathlib import Path
ROOT=Path(__file__).parent
def load(name):
 spec=importlib.util.spec_from_file_location(name,ROOT/f"{name}.py");m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m
dec=load("dmslog_decode");emu=load("master_link_emulator");panel=load("panel_emulator")
class ToolsTest(unittest.TestCase):
 def test_firmware_mount_label_and_web_cannot_write_jbd(self):
  for role in ("masterlogger","slavelogger"):
   source=(ROOT.parent/role/"src"/"main.cpp").read_text(encoding="utf-8")
   self.assertIn("LITTLEFS_PARTITION_LABEL",source);self.assertIn("LittleFS.begin(false,LITTLEFS_BASE_PATH,LITTLEFS_MAX_OPEN_FILES,LITTLEFS_PARTITION_LABEL)",source)
  web=(ROOT.parent/"shared"/"DmsCommon"/"src"/"WebPortal.cpp").read_text(encoding="utf-8")
  self.assertNotIn("0x5A",web);self.assertNotIn("PROXY_JBD_READ",web)
 def test_cobs_crc(self):
  raw=bytes(range(256));self.assertEqual(raw,emu.cobs_decode(emu.cobs_encode(raw)));self.assertEqual(0x29B1,emu.crc16(b"123456789"))
 def test_jbd_length_and_checksum(self):
  q=panel.request(3);self.assertEqual(7,len(q));self.assertEqual(0xFFFD,int.from_bytes(q[-3:-1],"big"))
 def test_physical_sample(self):
  p=emu.sample();row=dec.sample(p);self.assertEqual(24,row["cell_count"]);self.assertEqual(3300,row["cell_01_mV"]);self.assertEqual(-1200,row["current_mA"])
 def test_log_stream_and_truncated_tail(self):
  h=bytearray(64);h[:7]=b"DMSLOG2";h[7]=2;h[8]=0;struct.pack_into("<II",h,12,1,2);struct.pack_into("<I",h,60,zlib.crc32(h[:60])&0xffffffff)
  p=emu.sample();rh=struct.pack("<HBBI",0xD25A,1,1,len(p));record=rh+p+struct.pack("<I",zlib.crc32(rh+p)&0xffffffff)
  with tempfile.TemporaryFile() as f:
   f.write(h+record+b"\x5a");f.seek(0);self.assertEqual(2,dec.read_header(f)["version"]);items=list(dec.records(f));self.assertEqual(1,len(items));self.assertEqual(42,dec.sample(items[0][3])["sequence"])
 def test_corruption_in_middle_is_rejected(self):
  p=b"abc";rh=struct.pack("<HBBI",0xD25A,7,1,len(p));record=rh+p+struct.pack("<I",0);tail=rh+p+struct.pack("<I",zlib.crc32(rh+p)&0xffffffff)
  with tempfile.TemporaryFile() as f:
   f.write(record+tail);f.seek(0)
   with self.assertRaisesRegex(ValueError,"CRC inválido"):list(dec.records(f))
 def test_decoder_csv(self):
  h=bytearray(64);h[:7]=b"DMSLOG2";h[7]=2;struct.pack_into("<HII",h,10,1,1,2);struct.pack_into("<I",h,60,zlib.crc32(h[:60])&0xffffffff);p=emu.sample();rh=struct.pack("<HBBI",0xD25A,1,1,len(p));record=rh+p+struct.pack("<I",zlib.crc32(rh+p)&0xffffffff)
  with tempfile.TemporaryDirectory() as folder:
   source=Path(folder)/"synthetic.dmslog";output=Path(folder)/"samples.csv";source.write_bytes(h+record);old=sys.argv;sys.argv=["dmslog_decode.py",str(source),"--csv",str(output)]
   try:self.assertEqual(0,dec.main())
   finally:sys.argv=old
   text=output.read_text(encoding="utf-8");self.assertIn("cell_24_mV",text);self.assertIn("3323",text)
 def test_decoder_preserves_unknown_payload(self):
  h=bytearray(64);h[:7]=b"DMSLOG2";h[7]=2;struct.pack_into("<I",h,60,zlib.crc32(h[:60])&0xffffffff);p=b"\x00\xdd\x77";rh=struct.pack("<HBBI",0xD25A,99,7,len(p));record=rh+p+struct.pack("<I",zlib.crc32(rh+p)&0xffffffff)
  with tempfile.TemporaryDirectory() as folder:
   source=Path(folder)/"unknown.dmslog";output=Path(folder)/"unknown.jsonl";source.write_bytes(h+record);old=sys.argv;sys.argv=["dmslog_decode.py",str(source),"--unknown",str(output)]
   try:self.assertEqual(0,dec.main())
   finally:sys.argv=old
   self.assertIn('\"payload_hex\": \"00dd77\"',output.read_text(encoding="utf-8"))
if __name__=="__main__":unittest.main()
