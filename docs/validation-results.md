# Resultados de validação

Este documento diferencia testes automatizados, evidências físicas estacionárias e atividades ainda pendentes. A validação descrita aqui não constitui certificação funcional nem autoriza teste em movimento.

## Testes automatizados e simulados

Executados localmente em Windows, inclusive no caminho Unicode do projeto:

- builds PlatformIO de MasterLogger e SlaveLogger aprovados;
- 14/14 testes native do MasterLogger aprovados;
- 3/3 testes native do SlaveLogger aprovados;
- 8/8 testes Python aprovados;
- codecs JBD, COBS/CRC16, snapshot, RTC, DMSLOG2/CRC32, rotação, reserva, rótulo LittleFS, path traversal, truncamento final e corrupção intermediária cobertos.

Os comandos, uso de RAM/flash e casos cobertos estão detalhados em [testing.md](testing.md).

## Validação física estacionária — 2026-08-21

O ensaio utilizou a motocicleta parada, BMS JBD, dois ED100 e painel real na topologia documentada em [wiring.md](wiring.md).

Resultados observados:

- comunicação RS485 entre MasterLogger e BMS funcionando;
- comunicação UART0/DMS-Link entre MasterLogger e SlaveLogger funcionando;
- painel enviando requests JBD `0x03` aproximadamente uma vez por segundo;
- primeira transação durante a inicialização sem snapshot disponível, seguida de recuperação automática;
- 45 respostas consecutivas bem-sucedidas após a inicialização;
- latência média aproximada de 10 ms e máxima observada de 41 ms após a inicialização;
- painel exibindo corretamente SOC de 100%;
- logs criados e baixados dos dois ED100;
- cabeçalhos DMSLOG2 e CRCs válidos, sem registro intermediário corrompido ou registro final truncado;
- 81 amostras Master durante aproximadamente 80 s, em torno de 1 Hz;
- 44 amostras Slave durante aproximadamente 43 s, em torno de 1 Hz;
- nenhum registro descartado;
- frames JBD brutos e transações do painel presentes nos logs;
- 24 tensões de células e cinco temperaturas registradas por amostra;
- dados do Slave abaixo do limite normal de 2500 ms durante o ensaio;
- um timeout adicional de polling durante o período do Slave, sem perda do DMS-Link e com recuperação.

O decoder local confirmou nos arquivos analisados:

| Arquivo | Papel | Registros principais |
| --- | --- | --- |
| sessão Master | MasterLogger | 81 `MASTER_SAMPLE`, 159 `RAW_JBD_FRAME`, 1 `SYSTEM_EVENT`, 1 `TIME_EVENT` |
| sessão Slave | SlaveLogger | 44 `SLAVE_SAMPLE`, 46 `PANEL_TRANSACTION`, 1 `RAW_JBD_FRAME`, 1 `SYSTEM_EVENT` |

## Ressalvas do ensaio

- Os arquivos Master e Slave analisados pertencem a sessões diferentes e não permitem correlação linha a linha.
- Houve sincronização do RTC pelo navegador durante o arquivo Master, causando avanço no timestamp de parede; o tempo monotônico permaneceu adequado para duração e ordenação.
- Tensão, corrente, SOC, temperaturas e células pertencem à bateria específica do ensaio e não são especificações universais do projeto.
- O timeout observado demonstra recuperação naquele cenário, não cobertura de todas as falhas possíveis.

## Ainda não validado

- funcionamento com a motocicleta em movimento;
- resistência a vibração, chuva, temperatura e interferência de uso rodoviário;
- autonomia do pack portátil;
- registro contínuo por muitas horas ou até memória cheia;
- comportamento durante descarga até SOC baixo;
- diagnóstico definitivo da queda de SOC observada em outras motocicletas;
- todos os comandos JBD que um painel possa emitir;
- vida útil da flash em uso prolongado.

Qualquer ensaio adicional deve seguir [stationary-motorcycle-test.md](stationary-motorcycle-test.md), registrar versão/commit e separar claramente observação física de inferência.
