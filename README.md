# DMS Portable Logger

Logger experimental e gateway JBD estritamente read-only para uma moto DMS, implementado com dois ED100/ESP32-WROOM-32E. O master consulta o BMS; o slave apresenta ao painel os frames brutos validados e registra cada transação.

![Protótipo inicial do logger portátil DMS com duas placas ED100 interligadas](docs/images/dms-portable-logger-prototype.jpg)

*Protótipo inicial de desenvolvimento. Fotografia: Pedro Akio Sakuma, 2026.*

```text
BMS JBD <-- RS485 9600 8N1 --> MasterLogger <-- DMS-Link/UART0 230400 --> SlaveLogger <-- RS485 9600 8N1 --> painel
```

`0x03`, `0x04` e `0x05` usam RAW_MIRROR. `0x0F` e outras leituras desconhecidas são encaminhadas por proxy sem interpretação. Ação `0x5A`, inclusive controle MOS `0xE1`, é bloqueada nos dois lados e nunca chega ao BMS. O projeto não altera SOC, RmCap, calibração, proteções ou MOSFETs.

## Hardware

A pinagem está centralizada em `shared/DmsCommon/src/Ed100Pins.h`: UART0 GPIO1/3, UART2 GPIO17/16, direção RS485 GPIO4, RTC GPIO21/22 e LEDs GPIO32/33, 25/26 e 27/14. Ligue TX0 master ao RX0 slave, RX0 master ao TX0 slave e GND comum. Não una os pinos 3,3 V. Desconecte o outro ED100 durante gravação pela UART0.

O hardware analisado não possui isolamento galvânico nem cartão SD. A/B não tem nomenclatura universal. Confirme alimentação, referência elétrica, modo comum e polaridade com o sistema desenergizado. O logger não é certificado para segurança funcional.

## Arquitetura operacional

O master combina `0x03` e `0x04` a cada deadline absoluto de um segundo e só então avança `snapshot_seq`. `0x05` é consultado até ser válido e depois a cada hora. Polling e proxy passam pelo único proprietário da UART2. O servidor DMS-Link oferece HELLO, snapshot, registradores, raw JBD, proxy, horário e ping, com repetição idempotente da última transação.

O slave prioriza requests do painel, mantém lease de sequence por 400 ms e atualiza seu snapshot em background. Cache é normal até 2500 ms, stale permitido até 5000 ms e rejeitado depois disso; valores zero nunca são fabricados.

## Compilar e testar

Na raiz do repositório:

```powershell
pio run -d masterlogger
pio run -d slavelogger
pio test -d masterlogger -e native
pio test -d slavelogger -e native
python tools/test_tools.py
```

O pré-build copia automaticamente a fonte canônica `shared/DmsCommon` para a biblioteca local ignorada pelo Git, contornando toolchains Windows que não abrem biblioteca externa em caminho Unicode. Nunca edite a cópia em `masterlogger/lib` ou `slavelogger/lib`.

## Wi‑Fi e página

- Master: `DMS-MasterLogger-XXXX`, canal 1.
- Slave: `DMS-SlaveLogger-XXXX`, canal 6.
- Endereço: `http://192.168.4.1/`.
- Senha fallback de desenvolvimento: `dmslogger`.

Para senha local, copie `secrets.example.h` para `secrets.h` no projeto e altere `DMS_AP_PASSWORD`; o arquivo é ignorado pelo Git e a senha não aparece nas APIs.

A página exibe medições, sequence, timestamp, age, células, proteções e filesystem; sincroniza RTC pelo navegador; lista, baixa e exporta logs; rotaciona sessão e permite exclusão com confirmação. Downloads e CSV executam no Core 0, separados das tarefas de protocolo.

## Logs

Cada firmware usa LittleFS de `0x270000` bytes (2,4375 MiB), sem OTA. Arquivos `.dmslog` têm cabeçalho `DMSLOG2`, enquadramento e CRC32. A fila RAM desacopla flash dos protocolos, o sync ocorre em até cinco segundos e arquivos giram em 512 KiB. Com menos de 64 KiB livres o logger para sem apagar arquivos antigos. A API mostra taxa medida e estimativa de espaço; não há promessa de 24 horas.

```powershell
python tools/dmslog_decode.py arquivo.dmslog --csv samples.csv --transactions painel.csv
```

## LEDs

- Wi‑Fi verde: AP pronto; vermelho: falha.
- Server verde: logger normal; âmbar: ≥85%; vermelho lento: ≥95%; vermelho fixo: mount/log parado.
- RS485 pulso verde: frame válido; pulso vermelho: erro/timeout; vermelho duplo: proteção; âmbar rápido: proxy/stale.

## Falhas

BMS offline preserva o último snapshot e gera `POLL_ERROR`. Master offline permite cache do slave apenas dentro de 5 s. RTC inválido mantém timeouts/age monotônicos e marca clock unset. Falha de LittleFS ou Wi‑Fi não interrompe RS485. Fila cheia descarta somente log e incrementa contador. Memória cheia para o logger antes de ocupar toda a partição.

## Primeiro ensaio

Siga [stationary-motorcycle-test.md](docs/stationary-motorcycle-test.md). O ensaio é exclusivamente parado; testes físicos permanecem pendentes até serem realizados pelo usuário.

Documentação: [arquitetura](docs/architecture.md), [JBD](docs/jbd-protocol.md), [DMS-Link](docs/dms-link-v1.md), [registradores](docs/register-map-v1.md), [logs](docs/log-format-v2.md), [web](docs/web-interface.md), [LEDs](docs/leds.md), [testes](docs/testing.md) e [limitações](docs/limitations-and-safety.md).

Autor: Pedro Akio Sakuma. Licença MIT. Projeto independente, sem vínculo ou aprovação dos fabricantes citados.
