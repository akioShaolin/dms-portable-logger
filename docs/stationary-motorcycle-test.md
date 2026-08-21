# Validação estacionária

Não conduza a moto nesta etapa. Faça as ligações com moto desligada e carregador desconectado. Separe fisicamente os dois segmentos RS485: BMS somente no master e painel somente no slave. Cruze TX0/RX0 entre os ED100, una apenas GND e nunca una os pinos 3,3 V. Confirme A/B com o conjunto desenergizado; a nomenclatura pode estar invertida. Mantenha a roda sem possibilidade de tração.

1. Grave cada firmware e energize os ED100 sem BMS/painel. Confirme os dois APs; Wi-Fi verde indica AP pronto.
2. Em cada AP, abra `http://192.168.4.1/api/status` e confirme `partition_found=true`, `partition_label="littlefs"` e o estado do mount.
3. Somente se a partição for virgem, abra a página, leia o aviso e confirme **Inicializar LittleFS** com `INITIALIZE-LITTLEFS`; aguarde o reinício. Não faça isso para uma flash com logs a recuperar.
4. Confirme `fs_mounted=true`, `logging=true` e `/api/logs` com ao menos um arquivo de 64 bytes. Aguarde 30 s e confirme crescimento.
5. Abra `192.168.4.1`, acerte o RTC do master e confirme o slave sincronizado.
6. Com tudo desligado, conecte o BMS apenas ao master; reenergize e confirme `0x03`, `0x04`, células, tensão, RmCap, sequence e age.
7. Confirme `.dmslog` crescendo e Server verde; no slave, confirme master online.
8. Desligue, conecte o painel apenas ao slave e ligue somente a ignição. Observe SOC e transações.
9. Use **Nova sessão**, baixe o anterior e valide com `python tools/dmslog_decode.py arquivo.dmslog --csv amostras.csv --transactions painel.csv`.
10. Baixe o CSV web, confira telemetria/células; reinicie e confirme arquivos antigos.
11. Durante gravação/download, confirme que o painel continua exibindo SOC. Desligue imediatamente diante de aquecimento, reset ou erro persistente. Não conduza a moto.

Server amarelo indica uso acima de 85%; vermelho lento, acima de 95%; vermelho fixo, logger parado/erro de mount. RS485 vermelho lento indica BMS/master offline, vermelho duplo proteção ativa e amarelo rápido proxy.

## Registro do resultado

Anote data, commit/versão, firmware em cada ED100, SSIDs, estado de `/api/status`, nomes/tamanhos dos logs e resultado do decoder. Preserve os DMSLOG originais fora do repositório; publique somente amostras sanitizadas e deliberadamente selecionadas.

Já observado com a motocicleta parada: leitura do BMS pelo Master, DMS-Link, request `0x03`, resposta do Slave aceita pelo painel e SOC `100%`. As etapas de persistência e download após a correção do LittleFS continuam pendentes.
