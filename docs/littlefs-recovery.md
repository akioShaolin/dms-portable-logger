# Recuperação do LittleFS

Não formate, não execute `uploadfs` e não continue gravando quando houver logs a preservar. Leia primeiro uma imagem estática completa e calcule seu SHA-256; trabalhe somente em cópias.

Neste layout, a partição `littlefs` começa em `0x190000`, mede `0x270000` bytes, usa blocos de 4096 bytes e páginas de 256 bytes. Confirme esses valores em `partitions.csv` e no firmware analisado.

Uma ferramenta `mklittlefs` antiga pode acusar diretório corrompido mesmo quando uma implementação LittleFS compatível monta a imagem. A falha da ferramenta não autoriza formatar nem prova perda dos dados. Preserve imagem, hash, geometria e commit; tente uma implementação compatível em ambiente isolado e valide cada DMSLOG2 com `tools/dmslog_decode.py`.

Não versione dumps, logs recuperados, CSVs decodificados ou identificadores. Documente somente conclusões agregadas e sanitizadas.
