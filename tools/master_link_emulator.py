#!/usr/bin/env python3
"""Gerador de cenários DMS-Link para bancada; não encaminha escritas."""
import argparse
SCENARIOS=("valid","stale","timeout","bad-crc","seq-substituted","raw03","raw04","proxy0f","write-blocked")
def main():
 p=argparse.ArgumentParser();p.add_argument("port");p.add_argument("--scenario",choices=SCENARIOS,default="valid");a=p.parse_args();print(f"cenário {a.scenario} em {a.port}; conecte somente à UART0 TTL 3,3 V")
if __name__=="__main__":main()
