# Contribuindo

Contribuições são bem-vindas quando preservam a segurança, a compatibilidade e o escopo read-only do projeto.

## Fluxo de contribuição

1. Crie uma branch específica para a mudança.
2. Mantenha o conjunto pequeno, relacionado e fácil de revisar.
3. Não misture correções, refatorações ou documentação sem relação.
4. Explique objetivo, riscos, compatibilidade e testes realizados.
5. Atualize a documentação quando o comportamento observável mudar.
6. Preserve alterações locais que não pertencem à contribuição.

## Fonte canônica

`shared/DmsCommon` é a biblioteca compartilhada canônica. O pré-build cria cópias em `masterlogger/lib/DmsCommon` e `slavelogger/lib/DmsCommon` para contornar limitações do toolchain Windows em caminhos Unicode. Nunca edite essas cópias manualmente.

## Compilação e testes

Execute na raiz do repositório:

```powershell
pio run -d masterlogger
pio run -d slavelogger
pio test -d masterlogger -e native
pio test -d slavelogger -e native
python tools/test_tools.py
```

Inclua testes proporcionais à mudança. Alterações em codecs, proxy, cache, `age`, timeout, `sequence`, serialização ou formato de log exigem casos de sucesso, falha e compatibilidade. Mudanças que possam afetar BMS ou painel devem descrever claramente o risco e a forma de regressão utilizada.

Testes físicos seguem [docs/stationary-motorcycle-test.md](docs/stationary-motorcycle-test.md). Não declare como validado algo que foi apenas compilado ou simulado.

## Regras obrigatórias de segurança

- Preserve o gateway read-only.
- Nunca encaminhe ações JBD `0x5A` ao BMS.
- Nunca permita controle MOS `0xE1`.
- Não fabrique SOC, tensão, corrente, capacidade, temperaturas ou tensões de células.
- Ausência, expiração ou corrupção de dados deve produzir erro ou estado stale explícito, não um valor aparentemente válido.
- LittleFS não deve ser formatado ou apagado automaticamente.
- Mudanças de protocolo, pinagem, baud rate ou formato persistente exigem justificativa e documentação de compatibilidade.

## Arquivos que não devem ser publicados

- `secrets.h`, senhas locais, tokens ou outras credenciais;
- `.dmslog` de ensaios reais, salvo decisão consciente e anonimização;
- planilhas pessoais de diagnóstico;
- dumps, capturas ou metadados não destinados à publicação;
- conteúdo gerado em `.pio` e cópias staged de `DmsCommon`.

Antes de enviar uma contribuição, revise `git status --short --untracked-files=all` e `git diff --check`.

Vulnerabilidades devem seguir [SECURITY.md](SECURITY.md). Não abra issue pública contendo exploit, dump sensível ou passos reproduzíveis antes da correção.
