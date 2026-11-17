# Revisão independente com Gemini e DeepSeek

Cliente Python 3 (somente stdlib) que envia um texto/diff explicitamente selecionado aos dois provedores em paralelo e grava `gemini.md`, `deepseek.md` e `summary.json`. As respostas não executam comandos, não aplicam patches e não autorizam merge automaticamente.

## Configuração

| Ambiente | Uso |
| --- | --- |
| `GEMINI_API_KEY` | Chave da API Gemini |
| `GEMINI_MODEL` | ID de modelo disponível para essa chave, sem default |
| `DEEPSEEK_API_KEY` | Chave da API DeepSeek |
| `DEEPSEEK_MODEL` | ID de modelo disponível para essa chave, sem default |

Em GitHub Actions, guardar as chaves em **Settings → Secrets and variables → Actions → Secrets** e os modelos em **Variables**. Não colocar chaves no código, diff, argumentos de linha de comando ou mensagens de chat. Contas dos chats web e credenciais de API são configurações diferentes; a ferramenta não recupera sessões de navegador nem arquivos de contas. A utilização das APIs depende das credenciais, modelos e cotas dos respectivos provedores.

```bash
python3 tools/ai_review/review.py --check
python3 tools/ai_review/review.py --input review.diff --dry-run
python3 tools/ai_review/review.py --input review.diff --output-dir /tmp/nova-revisao --provider both
python3 -m unittest discover -s tools/ai_review -p 'test_*.py' -v
```

`--output-dir` deve ser uma pasta nova. O modo real valida a configuração dos dois provedores antes de qualquer POST. `--check` informa apenas presença das quatro variáveis; `--dry-run` valida entrada/limites sem rede, sem exigir chaves e sem criar arquivos. Nenhum modo procura credenciais em arquivos privados ou carrega `.env`.

Por padrão, a entrada é limitada a 131.072 bytes, a resposta HTTP a 1 MiB e cada parecer a 2.048 tokens solicitados/16.000 caracteres gravados. O timeout de socket é 90 segundos, com verificação de prazo entre leituras. Não há retries nem redirects de POST. Uma falha pode consumir cota mesmo sem parecer; não se repete automaticamente a chamada.

Entrada que contenha qualquer chave ativa é recusada. Respostas e erros são sanitizados. Parecer encerrado por limite de tokens, outro motivo diferente de conclusão normal, ou corte local é marcado `incomplete`; os arquivos são preservados e o processo retorna código 1. Uma resposta concluída tem estado `ok`, que indica conclusão do transporte/texto, não aprovação técnica do código. Se um provedor falhar, o parecer do outro continua disponível.

Não houve chamada real aos provedores nos testes da ferramenta. Os testes usam respostas HTTP simuladas e não comprovam autenticação, saldo, disponibilidade do modelo ou qualidade de revisão em produção.

## Referências oficiais

- [Gemini generateContent](https://ai.google.dev/api/generate-content).
- [Chaves da API Gemini](https://ai.google.dev/gemini-api/docs/api-key).
- [DeepSeek Chat Completions](https://api-docs.deepseek.com/api/create-chat-completion/).
- [DeepSeek API](https://api-docs.deepseek.com/).

## Revisões automáticas no GitHub

O workflow `.github/workflows/ai-review.yml` roda nos pull requests da própria conta Hakayze, dentro deste repositório, e aceita execução manual quando disponível na branch principal. PRs de forks externos ficam excluídos. O diff inclui apenas caminhos de código/build selecionados, com o limite de entrada acima. A base incremental é usada quando já contém a UI Fritchii; na correção inicial, o fallback é o commit oficial v5.8.4.

Cadastre uma vez os Secrets `GEMINI_API_KEY` e `DEEPSEEK_API_KEY` e as Variables `GEMINI_MODEL` e `DEEPSEEK_MODEL` em Settings → Secrets and variables → Actions. Use IDs de modelos disponíveis na sua conta; não há um nome padrão presumido. Sem configuração, os testes do cliente podem passar, mas o resumo informa explicitamente que nenhuma API foi chamada. Com configuração, os dois pareceres e o resumo ficam no artefato da execução por sete dias. Consultar esses relatórios é uma etapa de revisão; o workflow não envia comentários nem aplica mudanças ou merges.
