# Limitações e segurança

Sem isolamento galvânico, certificação funcional ou armazenamento SD. A flash interna tem endurance e apenas cerca de 2,44 MiB para LittleFS; não há promessa de 24 horas. O painel recebe uma representação dos dados; proteções continuam responsabilidade do BMS. Dados perdidos nunca equivalem a zero.

O firmware nunca formata automaticamente. **Inicializar LittleFS** e `uploadfs` sobrescrevem a partição: use apenas conscientemente, com a moto parada e depois de preservar logs. Falha de mount em dispositivo antes funcional deve ser diagnosticada antes de formatar. Testes de persistência/download no hardware continuam pendentes após esta correção.

## Limitações conhecidas

- não há OTA; atualização exige acesso serial;
- não há cartão SD nem retenção garantida por período fixo;
- a flash interna sofre desgaste e o firmware não substitui uma aquisição certificada;
- RTC inválido reduz a qualidade temporal, embora monotonic/age continuem disponíveis;
- cache stale é limitado a 5 s e sempre sinalizado;
- o CSV web atende inspeção prática; `tools/dmslog_decode.py` é a ferramenta de validação completa;
- o sistema foi validado apenas estaticamente, sem teste de condução.

Não conduza a motocicleta para “completar” um teste. Preserve logs antes de atualizar firmware, inicializar LittleFS ou usar `uploadfs`.
