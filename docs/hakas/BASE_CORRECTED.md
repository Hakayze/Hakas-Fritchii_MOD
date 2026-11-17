# Base correta: Fritchii v5.8.4

O pacote `frichii.zip` fornecido pelo usuário foi comparado com a [release oficial fritchii-v5.8.4](https://github.com/Frizti/Fritchii/releases/tag/fritchii-v5.8.4), sem executar os binários.

| Item | Resultado |
| --- | --- |
| Commit fonte | `4f5c53149b7e17d1c24445f68d79a83a3b2a17f9` |
| Branch upstream na auditoria | `oot-gather-fix` |
| Executável oficial e enviado | `lichii.exe`, 53.387.264 bytes |
| SHA-256 idêntico em ambos | `e3138be7663d5af875f10d41cf52d0ae580db8ceaab35a2c92b3b4f80cf0ca5a` |
| Fonte da UI | [src/yuzu/fritchii](https://github.com/Frizti/Fritchii/tree/fritchii-v5.8.4/src/yuzu/fritchii) |

O fork inicial usou `nx2` upstream, commit `f66ea633db6058efe31a9611718019e5bd1667eb`. Esse commit não contém a UI FritchiiUi da versão enviada. O nome do executável é herdado nas duas versões; a diferença foi identificada pelo conteúdo e pela correspondência dos executáveis.

## Preservação e integração

Esta branch usa a árvore da release correta e preserva a documentação Hakas anterior como histórico. A branch `nx2` do projeto não é alterada por esta preparação. As diferenças de emulação herdadas da release não são mudanças de core desenvolvidas pelo Hakas.

- `FritchiiUi::Enabled()` exige `g_build_name == "Fritchii"` e ausência de `FRITCHII_CLASSIC_UI`. Renomear o build sem adaptar esse gate desliga a interface desejada.
- A versão vem de `dist/frizti_version.txt` (5.8.4). Uma tag CI `hakas-ci-N` preserva a identidade; `build-N` seleciona outro caminho no gerador.
- Nesta versão `EDEN_DIR` é `fritchii` e o portátil é `user`. Preservar ambos.
- Home, rail, GameControls, PerfOverlay, PrecompileGate, saves, fontes e tema permanecem como publicados.
- O updater original permanece nesta reprodução da base; a política de atualização do MOD deve ser validada antes de sua distribuição.

## Validação

A igualdade do EXE oficial comprova qual pacote o usuário forneceu; não comprova execução de um binário recompilado pelo Hakas. O CI desta branch verifica versão e identidade, compila os targets originais e empacota Qt. Build, abertura da UI correta e instalação limpa precisam de resultados próprios.

O protótipo Qt Quick 3D do PR #2 está preservado na branch anterior. Seus testes de QML/host não comprovam integração com esta Home. A adaptação é um próximo incremento, com opção inicialmente desativada.

## Revisões externas

O workflow automático chama somente Gemini quando a chave e o modelo estão configurados. O cliente também oferece DeepSeek como opção manual explícita. Credenciais não fazem parte do repositório nem do executável. Sem configuração, o workflow informa que as revisões não foram executadas. Os testes do cliente usam HTTP simulado e não são pareceres dos provedores.

## Ajuste de compatibilidade MSVC e modo gratuito

A primeira execução Windows passou pela configuração e pela verificação de identidade Fritchii, mas falhou em C4459 em `src/common/frizti_settings.cpp`: quatro parâmetros/variáveis locais `values` escondiam `Settings::values`. A correção renomeia somente esses identificadores e suas referências, sem mudar tipos, configurações, defaults ou schema. O novo build deve verificar a correção; o restante da fonte da release é preservado.

Por solicitação de orçamento zero, o workflow automático passou a chamar somente Gemini, com chave de projeto Free Tier sem faturamento e modelo elegível. DeepSeek oficial não é solicitado por esse workflow. O cliente continua oferecendo DeepSeek como opção explícita, sujeita às tarifas do provedor. A cota esgotada termina a execução, sem retry ou troca para API paga. Não é possível detectar o estado de faturamento do projeto Google a partir da chave; a configuração Free Tier precisa ser feita na conta do usuário.

## Validação após configuração e compilador Windows

Em 17/11/2026, o workflow reconheceu `GEMINI_API_KEY` e `GEMINI_MODEL=gemini-3.8-flash`. Duas chamadas reais ao Gemini terminaram em HTTP 503, sem parecer. A presença das variáveis e esses erros não comprovam autenticação nem uma revisão concluída. O cliente distingue indisponibilidade do serviço de erros de credenciais e cota; não há fallback pago ou retry automático.

Após a correção de identificadores locais, o build MSVC avançou e parou em C4457/C4458 em `kepler_compute.cpp` e `maxwell_3d.cpp`. O próximo ensaio seleciona clang-cl LLVM 22.1.8, mantendo VS 2022 x64, SDK Windows e Qt 6.8.3 MSVC. O instalador oficial tem versão e SHA-256 fixados, e o CI verifica o compilador e a ABI antes do build. Essa escolha segue o ambiente local documentado no `AGENTS.md` upstream, não reproduz comprovadamente o compilador do EXE oficial: o workflow de release padrão usa MSVC e a variante PGO usa MinGW clang.

Nenhum arquivo do core foi modificado para contornar esses avisos. `ENABLE_WERROR` e a identidade Fritchii permanecem ativos. A instalação e o build com LLVM 22 precisam passar no runner Windows; validação estática do YAML não comprova isso.

