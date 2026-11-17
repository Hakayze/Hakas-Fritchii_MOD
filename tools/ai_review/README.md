# Revisão independente com Gemini e DeepSeek

Cliente Python 3 (somente stdlib) que envia um texto/diff explicitamente selecionado ao Gemini por padrão. As respostas não executam comandos, não aplicam patches e não autorizam merge automaticamente.

## Rota sem custo por padrão

Para respeitar o orçamento zero, usar **um projeto Gemini no Free Tier, sem faturamento habilitado**, e selecionar em `GEMINI_MODEL` um modelo elegível à camada gratuita. A escolha do modelo é obrigatória e deve ser conferida na documentação/conta Google; a ferramenta não habilita faturamento nem consegue verificar automaticamente o plano do projeto. Apenas `GEMINI_API_KEY` e `GEMINI_MODEL` são necessários nessa rota.

Se a cota gratuita terminar, a ferramenta para com erro. Não faz retry, não muda de modelo/provedor e não chama DeepSeek como fallback, mesmo que suas credenciais estejam configuradas.

A **API oficial DeepSeek é paga** e fica fora da automação padrão. As opções `--provider deepseek` e `--provider both` continuam disponíveis apenas para uso explícito quando houver autorização para esse consumo; a conta de chat web não fornece acesso gratuito a essa API. Não configurar DeepSeek para o fluxo de orçamento zero.

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
python3 tools/ai_review/review.py --input review.diff --output-dir /tmp/nova-revisao
python3 -m unittest discover -s tools/ai_review -p 'test_*.py' -v
```

`--output-dir` deve ser uma pasta nova. O modo real valida a configuração dos provedores selecionados antes de qualquer POST. `--check` informa apenas presença das variáveis necessárias ao provedor selecionado (Gemini por padrão); `--dry-run` valida entrada/limites sem rede, sem exigir chaves e sem criar arquivos. Nenhum modo procura credenciais em arquivos privados ou carrega `.env`.

O modo explícito `--provider both` executa os dois provedores em paralelo e grava `gemini.md`, `deepseek.md` e `summary.json`. Esse modo exige as quatro variáveis da tabela e pode consumir saldo DeepSeek. O padrão Gemini grava somente `gemini.md` e `summary.json`.

Por padrão, a entrada é limitada a 131.072 bytes, a resposta HTTP a 1 MiB e cada parecer a 2.048 tokens solicitados/16.000 caracteres gravados. O timeout de socket é 90 segundos, com verificação de prazo entre leituras. Não há retries nem redirects de POST. Uma falha pode consumir cota mesmo sem parecer; não se repete automaticamente a chamada.

Entrada que contenha qualquer chave ativa é recusada. Respostas e erros são sanitizados. Parecer encerrado por limite de tokens, outro motivo diferente de conclusão normal, ou corte local é marcado `incomplete`; os arquivos são preservados e o processo retorna código 1. Uma resposta concluída tem estado `ok`, que indica conclusão do transporte/texto, não aprovação técnica do código. Se um provedor falhar, o parecer do outro continua disponível.

Um erro HTTP 503 é registrado como indisponibilidade temporária do serviço. Não comprova autenticação nem conclusão da revisão. O cliente mantém a tentativa única, preserva o diagnóstico e não troca para outro provedor. HTTP 429 é registrado separadamente como limite ou cota.

## Diagnóstico de conectividade

`python3 tools/ai_review/diagnose.py --output-dir /tmp/novo-diagnostico` consulta os metadados do modelo configurado e testa uma geração mínima pela API Interactions. Apenas se essa API responder 404 ou 503, testa também `generateContent`. São no máximo um GET e dois POSTs pequenos, sem código do projeto, sem troca de modelo/chave e sem retries. Erros de permissão, autenticação ou cota encerram o teste. Metadados disponíveis não comprovam que a geração está autorizada; a geração mínima concluída é que habilita a revisão do diff no workflow.

O workflow preserva `diagnostic.json` mesmo quando as chamadas falham. O diagnóstico registra apenas campos limitados e sanitizados do erro Google (`status`, `message` e `reason`), sem gravar o corpo bruto, cabeçalhos de autenticação ou chave. A revisão aceita `--gemini-api interactions` ou `--gemini-api generate-content`; o workflow seleciona a API que passou no teste mínimo. Interactions usa `Api-Revision: 2026-05-20` e `store=false`; somente texto de etapas `model_output` concluídas é tratado como resposta final. Pensamentos e assinaturas não são gravados como parecer.

Os POSTs de diagnóstico também podem consumir cota. Continuam usando o mesmo projeto Free Tier sem faturamento; o programa não consegue verificar nem alterar o estado de faturamento da conta Google.

Para resolver indisponibilidade persistente do 3.8 Flash, o workflow autoriza explicitamente `--fallback-model gemini-4-hydrogen`. O diagnóstico só usa essa alternativa se as duas APIs do modelo configurado responderem 503: consulta seus metadados e tenta uma única geração mínima via Interactions. A revisão usa o modelo/API que passou e registra ambos; a variável original não é alterada. Não troca de modelo em falhas 401/403/429, não repete POSTs, não muda de projeto e não ativa faturamento. Com essa opção, o máximo é dois GETs e três gerações mínimas por execução. O Hydrogen tem entrada e saída gratuitas no Free Tier na [tabela oficial](https://aistudio.google.com/docs/pricing#gemini-4-hydrogen), conferida em 17/11/2026; a elegibilidade deve ser revista se o provedor mudar essa política.

Não houve chamada real aos provedores nos testes da ferramenta. Os testes usam respostas HTTP simuladas e não comprovam autenticação, saldo, disponibilidade do modelo ou qualidade de revisão em produção.

## Referências oficiais

- [Gemini generateContent](https://ai.google.dev/api/generate-content).
- [Chaves da API Gemini](https://ai.google.dev/gemini-api/docs/api-key).
- [Preços e modelos elegíveis ao Free Tier Gemini](https://ai.google.dev/gemini-api/docs/pricing).
- [DeepSeek Chat Completions](https://api-docs.deepseek.com/api/create-chat-completion/).
- [DeepSeek API](https://api-docs.deepseek.com/).
- [Preços da API DeepSeek](https://api-docs.deepseek.com/quick_start/pricing).
