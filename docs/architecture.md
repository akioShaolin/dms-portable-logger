# Arquitetura do sistema

O BMS pertence exclusivamente ao RS485 do master. O painel pertence exclusivamente ao RS485 do slave. UART0 é reservada ao DMS-Link binário e nunca recebe texto de depuração.

No master, `bmsTask` é o único proprietário da UART2 e arbitra polling e proxy; `linkServerTask` é o único proprietário da UART0. No slave, `panelTask` possui UART2 e UART0, atendendo o painel antes da atualização de background. Escrita de log e HTTP executam no Core 0 e nunca seguram a UART.

O snapshot só avança após `0x03` e `0x04` válidos. O store mantém atual/anterior sob mutex. O slave mantém lease de 400 ms e cache stale limitado a 5 s.

## Separação por núcleo

- Core 1: polling BMS, UART0/DMS-Link e atendimento do painel;
- Core 0: fila de log, LittleFS, AP e HTTP;
- fila cheia descarta apenas o registro e incrementa o contador, sem bloquear protocolo.

## Fluxo de dados

O Master publica snapshot, registradores e respostas JBD brutas. O Slave solicita dados em background, mas interrompe esse trabalho para atender o painel. Requests duplicados do DMS-Link recebem a última resposta em cache sem repetir uma operação de proxy. Writes JBD são bloqueados nos dois lados.

LittleFS e Wi‑Fi são opcionais para a função gateway: uma falha nesses subsistemas deixa logging/web indisponíveis, mas não deve interromper BMS → Master → Slave → painel.
