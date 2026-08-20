# JBD suportado

Requests: `DD A5 CMD LEN DATA CHECKSUM_BE 77`. Responses: `DD CMD STATUS LEN DATA CHECKSUM_BE 77`. Checksum é o complemento de dois de 16 bits; no response soma `STATUS+LEN+DATA`. O parser usa `LEN`, aceita qualquer byte no payload e ressincroniza em `DD`. `0x03` contém informações básicas e temperaturas; `0x04`, células; `0x05`, hardware ASCII. `0x0F` é desconhecido e não deve ser interpretado.
