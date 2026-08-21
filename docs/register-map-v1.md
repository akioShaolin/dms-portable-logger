# Register Map v1

Registradores são palavras de 16 bits em ordem big-endian conceitual. Inteiros de 32/64 bits ocupam palavras mais significativas primeiro. A leitura é coerente apenas quando `SEQ_START` e `SEQ_END` coincidem.

| Endereço | Campo |
|---:|---|
| 0–1 | `SEQ_START` |
| 2–3 | schema, role |
| 4–5 | quality flags |
| 6–7 | boot ID |
| 8–11 | UTC epoch ms |
| 12–13 | age ms |
| 14–25 | pack mV, current mA, power mW, RmCap, nominal |
| 26–35 | ciclos, produção, software, RSOC, FETs, proteção, balanceamento, contagens |
| 36–41 | min/max/delta/média e índices |
| 48–55 | temperaturas dC |
| 64–95 | células mV |
| 112–127 | hardware ASCII, dois caracteres por palavra |
| 144–153 | contadores poll/timeout/checksum/link/log |
| 254–255 | `SEQ_END` |

Leitores devem obter o bloco completo e aceitar a amostra somente quando `SEQ_START == SEQ_END`. Se forem diferentes, o banco foi atualizado durante a leitura e deve ser consultado novamente. Campos reservados devem ser ignorados e mantidos disponíveis para evolução compatível.

O mapa é uma visão de diagnóstico; não existe operação de escrita de registradores no gateway.
