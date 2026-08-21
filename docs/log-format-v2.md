# DMSLOG2

O cabeçalho fixo tem 64 bytes, little-endian: magic `DMSLOG2`, versão 2, papel, clock quality, schema, boot/session IDs, início UTC, dispositivo, firmware, git SHA e CRC32 dos primeiros 60 bytes. O logger só declara sucesso após abrir, gravar integralmente, fazer flush e confirmar o arquivo.

Cada registro contém `sync:u16=0xD25A`, tipo, versão, `length:u32`, payload e CRC32 do header+payload. Tipos: master sample, poll error, slave sample, panel transaction, raw JBD, link, system e time. Só o último registro truncado é tolerado; corrupção anterior é erro.

Samples guardam UTC/monotonic/sequence/age/quality, grandezas elétricas, capacidades/SOC, proteção/FETs/balanceamento, temperaturas, estatísticas e células. `RAW_JBD_FRAME` preserva request/response/checksums: `0x03`/`0x04`/`0x05` no início, quando mudam e a cada 60 s; sempre em erro, proxy, desconhecido ou write bloqueado. Bytes extras de `0x03` não são interpretados sem documentação.

Rotação: 512 KiB. Reserva: 64 KiB, sem apagar antigos. CSV web e `tools/dmslog_decode.py` validam CRC; o utilitário também exporta transações e desconhecidos em hexadecimal.

## Integridade e compatibilidade

O CRC do cabeçalho cobre os primeiros 60 bytes. O CRC de cada registro cobre sync, tipo, versão, comprimento e payload. Decoders devem ignorar tipos desconhecidos somente depois de preservar `payload_hex`; não devem tentar ressincronizar silenciosamente após corrupção intermediária.

Mudanças incompatíveis exigem nova versão do formato. A versão do firmware não substitui a versão DMSLOG: ambas devem aparecer nas notas de release.
