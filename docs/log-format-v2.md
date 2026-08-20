# DMSLOG2

Cabeçalho fixo de 64 bytes, little-endian: magic `DMSLOG2`, versão 2, role, boot ID, session ID, device e CRC32 dos primeiros 60 bytes.

Cada registro contém `sync:u16=0xD25A`, tipo, versão, `length:u32`, payload e CRC32 de header+payload. Tipos: master sample, poll error, slave sample, panel transaction, raw JBD, link, system e time. Um último registro truncado é ignorado; corrupção em registro íntegro é erro.

Samples armazenam explicitamente sequence, UTC, monotonic, age, quality, pack/current/power, capacidades, RSOC/FETs, temperaturas e todas as células. Rotação ocorre em 512 KiB; o logger para com menos de 64 KiB livres e não apaga arquivos antigos.
