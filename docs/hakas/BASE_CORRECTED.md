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

A ferramenta e o workflow de revisão chamam Gemini e DeepSeek somente quando chaves e modelos estão configurados. Credenciais não fazem parte do repositório nem do executável. Sem configuração, o workflow informa que as revisões não foram executadas. Os testes do cliente usam HTTP simulado e não são pareceres dos provedores.
