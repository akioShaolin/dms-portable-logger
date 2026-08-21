# Interface web

Cada ED100 cria seu próprio AP (`DMS-MasterLogger-XXXX`, canal 1; `DMS-SlaveLogger-XXXX`, canal 6) e atende em `http://192.168.4.1/`. A senha fallback é `dmslogger`; `secrets.h` pode substituí-la e nunca é exposto. A página responsiva não usa CDN.

Telemetria é atualizada a cada segundo e armazenamento/listagem a cada quatro segundos. Cards mostram grandezas elétricas, SOC, proteções, FETs, balanceamento, temperaturas, comunicação e células. JSON bruto fica em **Diagnóstico avançado**.

## Contrato HTTP

- `GET /api/status`: identidade, firmware/git, uptime/reset/RTC, `partition_found`, rótulo, mount, total/usado/livre, logger, erro, arquivo ativo, registros, bytes, descartes, taxa, estimativa, rotações e tempos da última escrita/flush.
- `GET /api/live`: snapshot e contadores Master/Slave.
- `GET /api/logs`: objeto com mount, logger, arquivo ativo e `files[]`. Mount falho retorna HTTP 503, nunca apenas `[]`.
- `GET /api/logs/{nome}`: download binário; o ativo recebe flush e header `X-DMS-Log-Active: true`.
- `GET /api/logs/{nome}/csv`: valida cabeçalho e CRC de cada registro, tolerando apenas truncamento final. Exporta telemetria, 32 células, transações e desconhecidos em hexadecimal.
- `POST /api/session/rotate`: retorna arquivo anterior e novo ativo.
- `POST /api/time`: epoch em milissegundos.
- `DELETE /api/logs/{nome}`: somente inativo.
- `POST /api/logs/erase-all`, corpo `ERASE-ALL`: apaga apenas inativos.
- `POST /api/storage/initialize`, corpo `INITIALIZE-LITTLEFS`: somente desmontado/logger parado; formata explicitamente e reinicia.

Nomes com traversal, barras, controles, aspas ou extensão incorreta são rejeitados. Transferências rodam incrementalmente no Core 0; protocolos permanecem no Core 1. **Finalizar e baixar** rotaciona e baixa o anterior fechado. Atualizações pausam durante confirmações destrutivas.

Inicializar formata e apaga. Se já havia logs e o mount falhou, preserve a flash e diagnostique/recupere externamente. `uploadfs` também sobrescreve a partição.

O download simples do arquivo ativo força `flush()` e informa `X-DMS-Log-Active: true`, mas não encerra a sessão. Para uma cópia definitiva, use **Finalizar e baixar**, que cria um novo arquivo ativo antes de iniciar o download do anterior.
