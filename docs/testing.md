# Validação

## Executado localmente

- [x] `pio run -d masterlogger`: aprovado em Windows no caminho Unicode.
- [x] `pio run -d slavelogger`: aprovado em Windows no caminho Unicode.
- [x] `pio test -d masterlogger -e native`: 11/11 casos aprovados.
- [x] `pio test -d slavelogger -e native`: 3/3 casos aprovados.
- [x] `python -m unittest -v tools/test_tools.py`: 6/6 casos aprovados, incluindo arquivo final truncado, CSV físico e preservação hexadecimal de tipo desconhecido.
- [x] Master: RAM 47.912/327.680 bytes (14,6%); flash 860.233/1.572.864 bytes (54,7%).
- [x] Slave: RAM 47.680/327.680 bytes (14,6%); flash 857.893/1.572.864 bytes (54,5%).
- [x] Codec JBD, corrente negativa, data, temperatura, 24 células/estado reinicializado, hardware ASCII, erros, COBS/CRC16, snapshot wire, RTC epoch e record CRC.

## Pendente de hardware

- [ ] DMS-Link entre os dois ED100 físicos.
- [ ] Comparar `0x03`/`0x04`/`0x05` com JBDTools.
- [ ] Validar slave com o painel real e confirmar o comportamento de `0x0F`.
- [ ] Perdas de BMS/UART0, reboots e brownout.
- [ ] Download concorrente enquanto o painel consulta.
- [ ] Limites reais de flash em 85%, 95% e stop.
- [ ] Queda abrupta e recuperação do último registro.
- [ ] Ensaio contínuo de 30 minutos.
- [ ] Checklist estacionário; nenhum teste de condução.

Compilação e simulação não são marcadas como validação física.
