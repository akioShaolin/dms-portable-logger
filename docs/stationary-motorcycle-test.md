# Primeiro teste estacionário

Não conduza a moto nesta etapa. Faça as ligações com moto desligada e carregador desconectado. Separe fisicamente os dois segmentos RS485: BMS somente no master e painel somente no slave. Cruze TX0/RX0 entre os ED100, una apenas GND e nunca una os pinos 3,3 V. Confirme A/B com o conjunto desenergizado; a nomenclatura pode estar invertida. Mantenha a roda sem possibilidade de tração.

1. Energize os ED100 sem BMS/painel. Confirme os dois APs; Wi-Fi verde indica AP pronto.
2. Abra `192.168.4.1`, acerte o RTC do master e confirme o slave sincronizado.
3. Com tudo desligado, conecte o BMS apenas ao master; reenergize.
4. Confirme `0x03`, `0x04`, 24 células, tensão, RmCap, sequence e age. Pulso RS485 verde indica frame válido; vermelho indica erro/timeout.
5. Confirme que o `.dmslog` cresce e que o LED Server permanece verde.
6. Confirme na página do slave que o master está online.
7. Desligue, conecte o painel apenas ao RS485 do slave e ligue somente a ignição.
8. Observe a porcentagem e compare request, response, source sequence, age e latência.
9. Desligue imediatamente se houver aquecimento, reset, LED vermelho persistente ou comportamento inesperado.
10. Baixe os logs dos dois dispositivos antes de alterar firmware ou ligações. Não saia com a moto.

Server amarelo indica uso acima de 85%; vermelho lento, acima de 95%; vermelho fixo, logger parado/erro de mount. RS485 vermelho lento indica BMS/master offline, vermelho duplo proteção ativa e amarelo rápido proxy.
