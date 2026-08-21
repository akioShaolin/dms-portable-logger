# Política de segurança

## Versões suportadas

Ainda não existe release ou tag estável.

| Versão | Suporte |
| --- | --- |
| `main` / desenvolvimento atual | Sim |
| versões e commits anteriores | Não |

## Como relatar uma vulnerabilidade

O método preferencial é o **Private Vulnerability Reporting / Security Advisory do GitHub**. Em 2026-08-21, a API do repositório informou que esse recurso ainda está desabilitado; portanto, atualmente não há canal privado funcional publicado pelo projeto.

O mantenedor deve habilitá-lo em **Settings → Security → Private vulnerability reporting**. Depois de habilitado, relatos privados poderão ser iniciados em:

```text
https://github.com/akioShaolin/dms-portable-logger/security/advisories/new
```

Enquanto o recurso estiver desabilitado, não publique exploit, dump, credenciais ou passos reproduzíveis em issue pública. Não há e-mail alternativo ou prazo de resposta declarado neste repositório.

Ao relatar privadamente, informe versão/commit, topologia, impacto, condições de reprodução, logs sanitizados e uma possível mitigação. Remova dados pessoais, credenciais e informações do veículo sem relação com o defeito.

## Impactos críticos

- encaminhamento indevido de escrita ao BMS;
- bypass do bloqueio JBD `0x5A` ou controle MOS `0xE1`;
- fabricação ou corrupção silenciosa de valores apresentados ao painel;
- aceitação de frame sem validação de comando, tamanho, status, checksum ou terminador;
- erro de cache/`age` que faça dado vencido parecer atual;
- path traversal, leitura indevida ou exclusão não autorizada pela interface web;
- exposição da senha do AP, `secrets.h` ou outro segredo local;
- negação de serviço que comprometa o painel, DMS-Link ou polling do BMS.

## Limites do projeto

O DMS Portable Logger é experimental, não possui certificação de segurança funcional e não deve ser considerado mecanismo de proteção da motocicleta ou da bateria. As proteções continuam sendo responsabilidade do BMS e dos sistemas originais.

A senha fallback `dmslogger` é destinada a bancada e ambientes controlados. Antes de outro uso, substitua-a por senha local em `masterlogger/include/secrets.h` e `slavelogger/include/secrets.h`; esses arquivos são ignorados pelo Git.

Consulte também [docs/limitations-and-safety.md](docs/limitations-and-safety.md).
