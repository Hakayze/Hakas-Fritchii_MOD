# Hakas-Fritchii_MOD — documentação do projeto

Atualização: **16/11/2026**. Responsável: Cassiano Gualberto, conta GitHub **Hakayze**.

## Objetivo

Recriar a experiência cinematográfica 3D do BoxShelf dentro do Fritchii, com prioridade para o vídeo de referência e ampla compatibilidade com os PCs Windows já atendidos pelo emulador.

O nome oficial é **Hakas-Fritchii_MOD**. “Fritchii Cinematic UI” é o título histórico dos documentos de pesquisa.

## Arquitetura proposta

| Parte | Responsabilidade |
| --- | --- |
| Qt Widgets existente | Layout Clássico, menus e serviços atuais |
| C++ do frontend | Adaptar dados, encaminhar comandos e coordenar estados |
| QML / Qt Quick | Composição visual, navegação, painéis e transições |
| Qt Quick 3D | Câmera, caixas, materiais e inspeção |
| Sistema de emulação existente | Executar jogos, renderizar, preservar configurações e saves |

A primeira opção de hospedagem a testar é `QQuickView` com `QWidget::createWindowContainer`. A comparação com `QQuickWidget` será decidida no protótipo por comportamento, compatibilidade e custo medidos.

A nova biblioteca deverá observar os dados existentes. Não criar um segundo scanner de jogos, contador de tempo ou caminho alternativo de boot.

## Base rastreável

| Fonte | Commit da pesquisa original |
| --- | --- |
| Fritchii, branch nx2 | `f66ea633db6058efe31a9611718019e5bd1667eb` |
| BoxShelf source, branch main | `068ef313d8a24e4a7ac8f9fb94e8055066216afa` |
| BoxShelf beta, branch main | `043d6b7cf5cf3456251ab04b4eeb046062459940` |

O fork foi conferido contra a mesma base do Fritchii em 16/11/2026. A pesquisa original contém observações de código e vídeo; esta publicação de documentação não representa nova execução do emulador.

O vídeo de referência não está incluído neste repositório. Para revisão visual, fornecê-lo junto à tarefa; a descrição textual não substitui a gravação.

## Ordem de leitura

1. [Roteiro](ROADMAP.md): marcos, dependências e critérios de aceitação.
2. [Windows](WINDOWS.md): matriz a validar, empacotamento e fallback.
3. [Decisões](DECISOES.md): propostas aceitas e correções das revisões.
4. [Colaboração](COLABORACAO.md): responsáveis e evidências esperadas.
5. [Planejamento original](PLANEJAMENTO_ORIGINAL.md): especificação visual e pesquisa completa, preservadas como histórico.

Em caso de divergência com o relatório histórico, as decisões atuais estão em `DECISOES.md` e os requisitos de compatibilidade em `WINDOWS.md`.

## Estado e branches

- `nx2`: código-base herdado e documentação inicial do projeto.
- `feat/cinematic-ui`: branch prevista para iniciar o desenvolvimento da nova interface.
- Mudanças de implementação devem permanecer pequenas, revisáveis e acompanhadas de resultados reais.

A preparação documental não conclui M0. Nenhuma versão do Windows, GPU ou desempenho foi validada pelo Hakas até esta publicação.

## Regras para implementação

Consultar [AGENTS.md](../../AGENTS.md), [Coding.md](../Coding.md) e os guias herdados aplicáveis. Caminhos e máquinas citados nos documentos upstream não devem ser assumidos como o ambiente de Cassiano.

Preservar licenças e proveniência, limitar alterações ao frontend e registrar qualquer necessidade de mudança estrutural antes de ampliar o escopo.

