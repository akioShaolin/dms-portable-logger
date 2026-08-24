# Validação

## Executado localmente

- [x] `pio run -d masterlogger`: aprovado em Windows no caminho Unicode.
- [x] `pio run -d slavelogger`: aprovado em Windows no caminho Unicode.
- [x] `pio test -d masterlogger -e native`: 15/15 casos aprovados.
- [x] `pio test -d slavelogger -e native`: 3/3 casos aprovados.
- [x] `py -3 -m unittest tools/test_tools.py`: 9/9 casos aprovados, incluindo truncamento final, corrupção intermediária, proveniência e ausência de endpoint web JBD write.
- [x] Master: RAM 48.352/327.680 bytes (14,8%); flash 881.385/1.572.864 bytes (56,0%).
- [x] Slave: RAM 47.704/327.680 bytes (14,6%); flash 878.801/1.572.864 bytes (55,9%).
- [x] Rótulo `littlefs`, estado mount/erro, path traversal, header/CRC, rotação, reserva e persistência seletiva testados.
- [x] Codec JBD, corrente negativa, data, temperatura, 24 células/estado reinicializado, hardware ASCII, erros, COBS/CRC16, snapshot wire, RTC epoch e record CRC.

## Validado em hardware

- [x] BMS pelo master, DMS-Link, request `0x03`, resposta slave aceita e SOC `100%` no painel, sempre com a moto parada (relato do usuário).

## Pendente em hardware

- [ ] Forçar várias rotações com registros sintéticos, confirmar ausência de panic/watchdog/arquivo vazio e simular falha de abertura preservando o anterior; não reduzir o limite de produção.
- [ ] Desconectar somente o BMS por mais de 5 s e confirmar idade crescente, stale/inválido, sequência estável, cache conforme política e nenhum reboot.

- [ ] Revalidar LittleFS, logging, listagem, download e CSV nos dois ED100 após esta correção.
- [ ] Perdas de BMS/UART0, reboots e brownout.
- [ ] Download concorrente enquanto o painel consulta.
- [ ] Limites reais de flash em 85%, 95% e stop.
- [ ] Queda abrupta e recuperação do último registro.
- [ ] Ensaio contínuo de 30 minutos.
- [ ] Checklist estacionário; nenhum teste de condução.

Compilação e simulação não são marcadas como validação física.

Checklist pós-gravação: abrir `/api/status`; confirmar `partition_label=littlefs`; inicializar conscientemente se a partição for virgem; reiniciar; confirmar `logging=true`; verificar arquivo ≥64 bytes e crescimento em 30 s; criar sessão; baixar/decodificar; validar CSV; reiniciar e confirmar preservação; verificar que o painel mantém SOC durante gravação/download. O procedimento detalhado está em [stationary-motorcycle-test.md](stationary-motorcycle-test.md).
