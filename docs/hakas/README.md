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

O protótipo M1 usa `QQuickView` com `QWidget::createWindowContainer`. A comparação com `QQuickWidget` continua pendente dos testes de comportamento, compatibilidade e custo.

A nova biblioteca deverá observar os dados existentes. Não criar um segundo scanner de jogos, contador de tempo ou caminho alternativo de boot.

## Base rastreável

| Fonte | Commit da pesquisa original |
| --- | --- |
| Fritchii, branch nx2 | `f66ea633db6058efe31a9611718019e5bd1667eb` |
| BoxShelf source, branch main | `068ef313d8a24e4a7ac8f9fb94e8055066216afa` |
| BoxShelf beta, branch main | `043d6b7cf5cf3456251ab04b4eeb046062459940` |

O fork foi conferido contra a mesma base do Fritchii em 16/11/2026. O [registro de M0](M0_VALIDATION.md) distingue a compilação no CI da abertura confirmada pelo usuário. O [protótipo M1](M1_PROTOTYPE.md) tem testes próprios; ainda não houve validação de fidelidade visual do novo frontend contra o vídeo.

O vídeo de referência não está incluído neste repositório. Para revisão visual, fornecê-lo junto à tarefa; a descrição textual não substitui a gravação.

## Ordem de leitura

1. [Roteiro](ROADMAP.md): marcos, dependências e critérios de aceitação.
2. [Windows](WINDOWS.md): matriz a validar, empacotamento e fallback.
3. [Decisões](DECISOES.md): propostas aceitas e correções das revisões.
4. [Colaboração](COLABORACAO.md): responsáveis e evidências esperadas.
5. [Planejamento original](PLANEJAMENTO_ORIGINAL.md): especificação visual e pesquisa completa, preservadas como histórico.

Em caso de divergência com o relatório histórico, as decisões atuais estão em `DECISOES.md` e os requisitos de compatibilidade em `WINDOWS.md`.

## Estado e branches

- `nx2`: base clássica, documentação e CI integrados no PR #1.
- `feat/cinematic-ui`: protótipo M1 opcional, com host nativo, cena de teste e CI Windows ON/OFF, em revisão no PR #2.
- Mudanças de implementação devem permanecer pequenas, revisáveis e acompanhadas de resultados reais.

O build e a abertura clássica atendem ao critério mínimo de M0. A versão/build do Windows e o hardware da máquina do usuário ainda não foram registrados; instalação limpa, compatibilidade ampla, renderização do protótipo e desempenho continuam pendentes.

## Regras para implementação

Consultar [AGENTS.md](../../AGENTS.md), [Coding.md](../Coding.md) e os guias herdados aplicáveis. Caminhos e máquinas citados nos documentos upstream não devem ser assumidos como o ambiente de Cassiano.

Preservar licenças e proveniência, limitar alterações ao frontend e registrar qualquer necessidade de mudança estrutural antes de ampliar o escopo.
