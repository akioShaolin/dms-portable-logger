# DMS-Link v1

Frame bruto little-endian: magic `44 4D`, versão, tipo, flags, status, transaction ID, comprimento, payload e CRC-16/CCITT-FALSE. O frame é codificado em COBS e terminado em `00`. Payload máximo: 384 bytes; buffers fixos.

Tipos request/response: HELLO `01/81`, GET_SNAPSHOT `02/82`, READ_REGISTERS `03/83`, GET_JBD_RESPONSE `04/84`, PROXY_JBD_READ `05/85`, GET_TIME `06/86`, PING `07/87`.

GET_JBD_RESPONSE request: command u8, preferred sequence u32, max age u32. Response: source sequence u32, age u32, flags u16, frame length u16 e frame. PROXY request: length u16 + request JBD; response: latency u32 + length u16 + response JBD. GET_TIME retorna epoch u64, quality u8 e monotonic u32. GET_SNAPSHOT usa o layout explícito de sample descrito em `log-format-v2.md`. READ_REGISTERS recebe address/count u16 e retorna address/count seguidos pelas palavras.

O master conserva o último request/response para repetir transaction IDs duplicados sem repetir proxy.

HELLO retorna link version u8, role u8, snapshot schema u16, boot ID u32 e flash bytes u32. PING retorna uptime u32, stack high-water u32 e logs descartados u32. GET_SNAPSHOT retorna o sample wire completo. READ_REGISTERS request contém address/count u16; response repete address/count e as palavras u16. Todos os inteiros do DMS-Link são little-endian.

Flags indicam resposta stale, cache, proxy ou substituição da sequência solicitada. Status diferencia request inválido, snapshot ausente, stale, comando não suportado, write bloqueado, timeout, ocupado e erro interno. O Slave nunca deve converter falha de link em valores zerados aparentemente válidos.

UART0 é dedicada ao protocolo binário e não deve receber texto de debug. Alterações incompatíveis exigem nova versão do link e testes de interoperabilidade Master/Slave.
