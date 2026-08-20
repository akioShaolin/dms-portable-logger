# LEDs

Os canais via ULN2003 são ativos em HIGH; vermelho+verde produz âmbar.

| Estado | Indicação |
|---|---|
| AP pronto | Wi‑Fi verde |
| falha do AP | Wi‑Fi vermelho |
| logger normal | Server verde |
| filesystem ≥85% | Server âmbar |
| filesystem ≥95% | Server vermelho lento |
| mount/log parado | Server vermelho fixo |
| frame válido | pulso RS485 verde |
| timeout/checksum | pulso RS485 vermelho |
| proteção JBD | RS485 vermelho duplo |
| proxy/stale | RS485 âmbar rápido |

Todos os padrões usam `millis()`; não há `delay()`.
