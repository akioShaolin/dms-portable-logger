# Arquitetura

O BMS pertence exclusivamente ao RS485 do master. O painel pertence exclusivamente ao RS485 do slave. UART0 é reservada ao DMS-Link binário e nunca recebe texto de depuração.

No master, `bmsTask` é o único proprietário da UART2 e arbitra polling e proxy; `linkServerTask` é o único proprietário da UART0. No slave, `panelTask` possui UART2 e UART0, atendendo o painel antes da atualização de background. Escrita de log e HTTP executam no Core 0 e nunca seguram a UART.

O snapshot só avança após `0x03` e `0x04` válidos. O store mantém atual/anterior sob mutex. O slave mantém lease de 400 ms e cache stale limitado a 5 s.
