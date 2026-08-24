# Changelog

## [Unreleased]

### Corrigido

- normalização única de nomes LittleFS com e sem barra inicial para listar, baixar e excluir logs;
- exclusão com contagens reais, preservação do ativo, falha parcial e atualização imediata do espaço;
- rotação transacional sem reentrada do mutex, com recuperação do arquivo anterior e remoção de placeholder inválido;
- cache BMS stale/inválido após 2,5/5 s sem sequência artificial;
- flag compatível de build sujo no DMSLOG2.

### Validado

- análise pós-teste de percurso real de aproximadamente 60 km até esgotamento da bateria;
- recuperação externa do LittleFS e correlação entre rotação, arquivo vazio e reinicializações.

## [0.1.0] - 2026-08-21

### Adicionado

- firmwares MasterLogger e SlaveLogger para dois ED100/ESP32-WROOM-32E;
- codecs JBD e DMS-Link, snapshots coerentes, mapa de registradores e RTC;
- logging DMSLOG2, rotação, reserva de espaço e decoder/CSV;
- AP local, API HTTP, interface responsiva, LEDs e ferramentas de bancada;
- persistência seletiva dos frames JBD brutos;
- `dmslog_decode.py` com exportação de amostras e transações para CSV;
- LEDs de estado e documentação de operação, segurança e release;
- diagrama próprio das interligações externas entre BMS, ED100 e painel.

### Corrigido

- mount LittleFS com o rótulo explícito `littlefs` nos dois firmwares;
- distinção entre partição ausente, mount falho, logger parado e lista vazia;
- inicialização do filesystem somente mediante confirmação explícita, sem formatação automática.

### Validado

- builds e testes automatizados dos dois firmwares;
- ensaio estacionário em 2026-08-21 com BMS JBD, dois ED100 e painel real;
- leitura BMS, DMS-Link, requests `0x03`, resposta aceita e SOC `100%` no painel;
- criação, download e validação CRC dos logs Master/Slave.

Não foram realizados teste em movimento, descarga até SOC baixo, ensaio prolongado até memória cheia nem diagnóstico definitivo de queda de SOC em outras motocicletas.
