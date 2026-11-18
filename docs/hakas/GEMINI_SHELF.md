# Componentes da estante produzidos com Gemini

Esta entrega prepara a camada QML do modo Cinematográfico para a base **Fritchii v5.8.4**, commit `7d98cb2303ff185b3a6e26ef3d10e80a13e1e7f6`. A integração com a Home, o adaptador C++ e o empacotamento Windows ainda estão pendentes. Os arquivos não estão ligados ao target do emulador nesta etapa.

Por instrução do usuário em 18/11/2026, acompanhar releases upstream, incluindo a [v6.1](https://github.com/Frizti/Fritchii/releases/tag/fritchii-v6.1), sem atualizar esta base, sincronizar o fork ou trocar seus executáveis automaticamente.

## Entrega e revisão

O Gemini gerou `GameCase.qml`, `ShelfScene.qml` e `CinematicLibrary.qml` no notebook Fritchii CineBox, nesta [conversa](https://gemini.google.com/app/fe33482562cda0ff). Foram extraídos três blocos completos, totalizando **905 linhas originais**; a estimativa de 545 linhas dada na resposta não foi confirmada.

A revisão do coordenador acrescentou `ComponentBehavior: Bound` nos componentes com delegates, qualificou a seleção de malhas via metadados dinâmicos, removeu um import desnecessário, corrigiu o hash de cores para não exceder `int32`, tornou títulos/chaves texto simples, bloqueou cliques através do painel e priorizou o bloqueio de entrada antes dos modificadores. O `qmllint` de Qt 6.8.3 terminou sem diagnósticos após esses ajustes.

## Contrato para a futura ponte C++

| Entrada / sinal | Responsabilidade |
| --- | --- |
| `gameModel` | Modelo de janela já limitado a 15 jogos, com identidade preservada ao navegar. |
| Roles | `gameKey`, `titleText`, `frontCover`, `spineCover`, `backCover`, `relativeOffset`, `isCurrent`. Os tipos são string, string, URL, URL, URL, real e bool. |
| `gameCount` | Quantidade total de jogos disponível ao usuário, independente da janela visual. |
| `selectedKey`, `selectedTitle` | Chave opaca estável e título do selecionado; IDs `u64` não devem passar por `Number` do JavaScript. |
| `inputEnabled`, `loading` | Estado fornecido pelo host; modal/boot deve desativar a entrada da biblioteca. |
| `navigateRequested(delta)` | Solicitação de deslocamento ao adaptador, que atualiza a janela e a seleção. |
| `selectRequested(gameKey)` | Solicitação de seleção por identidade estável. |
| `launchRequested(gameKey)` | Resolver chave para path/program ID e chamar o boot original por `HomeHooks.play`. |
| `returnToClassic()` | Retornar ao layout existente e restaurar foco/estado no host. |

O adaptador deve projetar o modelo existente, tratar árvore/plano e clones de favoritos e fornecer URLs de capa locais ou do image provider autorizado. Esta camada não faz discovery, persistência, contagem de tempo ou boot.

`GameCase` constrói corpo, lombada, contracapa, tampa com dobradiça e cartucho com primitivas. A estante move o mesmo objeto para inspeção. A câmera usa `FrustumCamera`, distância lógica 1800 e origem vertical de projeção em 42%. A geometria e as poses são uma proposta incremental; não foram medidas contra os frames do vídeo.

## Validação reproduzível

A dependência Python é somente para desenvolvimento, não para distribuir o emulador:

```bash
python3 -m venv .venv-qml
.venv-qml/bin/python -m pip install PySide6==6.8.3
.venv-qml/bin/python tools/cinematic/smoke_qml.py
.venv-qml/bin/pyside6-qmllint src/yuzu/cinematic/qml/GameCase.qml src/yuzu/cinematic/qml/ShelfScene.qml src/yuzu/cinematic/qml/CinematicLibrary.qml
```

No Windows, usar os executáveis equivalentes em `.venv-qml/Scripts/`. O teste exige Qt exatamente 6.8.3, informa uma contagem sintética total de 500 jogos e fornece 15 linhas ao modelo visual. Exercita carregamento, sinais, teclado, detalhes/inspeção/Escape, chave emitida pelo sinal de início, bloqueio de entrada, loading, reset/vazio e frustum com viewport zero. Não cria um scanner ou um catálogo real de 500 jogos.

O backend nulo permite testar os objetos e a entrada, mas **não renderiza Qt Quick 3D** e imprime esse aviso. O teste não valida imagem, picking por raio, texturas, animação renderizada, FPS, consumo de VRAM, gamepad, integração de janela nativa ou Windows. Ainda é necessário conferir os componentes num backend 3D real e integrá-los ao host correto antes de oferecer um executável.

Reflexo, blur, livreto, progresso de shaders e tela de boot não fazem parte desta entrega.

## Referências

- [FrustumCamera em Qt 6.8](https://doc.qt.io/qt-6.8/qml-qtquick3d-frustumcamera.html).
- [Repeater3D em Qt 6.8](https://doc.qt.io/qt-6.8/qml-qtquick3d-repeater3d.html).
- [Modelo e hooks reais da Home](https://github.com/Hakayze/Hakas-Fritchii_MOD/blob/base/fritchii-v5.8.4/src/yuzu/fritchii/home.h).
