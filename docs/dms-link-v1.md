# DMS-Link v1

Frame bruto little-endian: magic `44 4D`, versão, tipo, flags, status, transaction ID, comprimento, payload e CRC-16/CCITT-FALSE. O frame é codificado em COBS e terminado em `00`. Payload máximo: 384 bytes; buffers fixos.
