# JBD suportado

Requests: `DD A5 CMD LEN DATA CHECKSUM_BE 77`. Responses: `DD CMD STATUS LEN DATA CHECKSUM_BE 77`. Checksum é o complemento de dois de 16 bits; no response soma `STATUS+LEN+DATA`. O parser usa `LEN`, aceita qualquer byte no payload e ressincroniza em `DD`. `0x03` contém informações básicas e temperaturas; `0x04`, células; `0x05`, hardware ASCII. `0x0F` é desconhecido e não deve ser interpretado.

Somente requests de leitura (`0xA5`) podem chegar ao BMS. Ações (`0x5A`) são respondidas localmente como bloqueadas. Frames são aceitos apenas com início, comando esperado, status, comprimento, checksum e terminador válidos.

Os bytes extras observados no payload `0x03` são preservados no frame bruto e no log, mas não recebem significado sem documentação verificável. Corrente é tratada como inteiro assinado; tensões, capacidades e temperaturas seguem as escalas documentadas pelo protocolo JBD.
