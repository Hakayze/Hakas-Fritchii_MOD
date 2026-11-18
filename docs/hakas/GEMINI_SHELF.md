# Componentes da estante produzidos com Gemini

Esta entrega prepara a camada QML do modo Cinematográfico para a base **Fritchii v5.8.4**, commit `7d98cb2303ff185b3a6e26ef3d10e80a13e1e7f6`. O adaptador C++ está implementado e testado isoladamente. A ligação com a Home e o empacotamento Windows ainda estão pendentes. Os arquivos não estão ligados ao target do emulador nesta etapa.

Por instrução do usuário em 18/11/2026, acompanhar releases upstream, incluindo a [v6.1](https://github.com/Frizti/Fritchii/releases/tag/fritchii-v6.1), sem atualizar esta base, sincronizar o fork ou trocar seus executáveis automaticamente.

## Entrega e revisão

O Gemini gerou `GameCase.qml`, `ShelfScene.qml` e `CinematicLibrary.qml` no notebook Fritchii CineBox, nesta [conversa](https://gemini.google.com/app/fe33482562cda0ff). Foram extraídos três blocos completos, totalizando **905 linhas originais**; a estimativa de 545 linhas dada na resposta não foi confirmada.

A revisão do coordenador acrescentou `ComponentBehavior: Bound` nos componentes com delegates, qualificou a seleção de malhas via metadados dinâmicos, removeu um import desnecessário, corrigiu o hash de cores para não exceder `int32`, tornou títulos/chaves texto simples, bloqueou cliques através do painel e priorizou o bloqueio de entrada antes dos modificadores. O `qmllint` de Qt 6.8.3 terminou sem diagnósticos após esses ajustes.

## Contrato da ponte C++

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

## Adaptador C++ entregue pelo Gemini

Em 18/11/2026, o Gemini, no modo exibido como **Neon Alto**, produziu os quatro arquivos completos do adaptador `CinematicGameModel` e do probe Qt Test na mesma conversa do notebook. O código foi revisado e compilado pelo coordenador com os headers, moc e bibliotecas oficiais de **Qt 6.8.3**, sem alterações no core ou no modelo original.

O adaptador recebe `QStandardItemModel` sem assumir sua propriedade e exige `SourceRoles` explícitos. Percorre árvore/plano, ignora itens não jogáveis, une clones de favoritos por ID e caminho normalizado e mantém entradas com o mesmo ID em caminhos diferentes. A seleção usa chave estável; IDs expostos ao QML são strings hexadecimais de 16 caracteres. A janela contém o selecionado e até sete vizinhos de cada lado, usando inserções, remoções e movimentos em vez de reset a cada seta.

Mudanças do modelo fonte são agrupadas em uma chamada enfileirada. A solicitação de início força atualização pendente e só emite `launchGameRequested(path, programId)` para o item atual e ainda existente. Capas retornam URLs vazias nesta etapa. Não há scanner adicional, boot, persistência ou contador de tempo no adaptador.

Para integrar no host, preencher `SourceRoles` com `GameListItem::TypeRole`, `GameListItemPath::TitleRole`, `GameListItemPath::FullPathRole`, `GameListItemPath::ProgramIdRole` e os valores `GameListItemType::Game` e `GameListItemType::Favorites`. Ligar as propriedades e sinais da tabela acima, encaminhando o sinal de início para o callback nativo `HomeHooks::play`.

### Resultado dos testes C++

O probe usa `QAbstractItemModelTester`, `QSignalSpy` e Qt Test. Foram executados **oito casos funcionais, mais inicialização e encerramento: 10 passaram, 0 falharam**, em Linux com Qt 6.8.3 e GCC 14.2. Cobrem árvore/plano, itens não jogáveis, favoritos com nomes traduzidos, clones, IDs iguais em caminhos distintos, ID zero, filtros, **500 entradas reais no modelo sintético**, janela de no máximo 15, navegação extrema, seleção após reordenação/reset, remoção com recusa de chave antiga e destruição do modelo fonte.

O SDK foi obtido dos arquivos oficiais de Qt, com SHA-256 verificado. Neste ambiente faltam os headers de desenvolvimento OpenGL exigidos pelo pacote CMake `Qt6Gui`; por isso a compilação local usou `g++`, `moc` e as bibliotecas oficiais diretamente. O projeto CMake/AUTOMOC abaixo foi fornecido, mas seu fluxo completo ainda precisa ser validado em um ambiente com as dependências de desenvolvimento instaladas:

```bash
cmake -S tools/cinematic/model_probe -B build/model_probe -DCMAKE_PREFIX_PATH=<Qt-6.8.3>
cmake --build build/model_probe --config Release
ctest --test-dir build/model_probe -C Release --output-on-failure
```

Os testes C++ são independentes do teste QML e não comprovam a integração entre ambos, a hospedagem Windows ou a renderização 3D. Nenhum executável novo do emulador foi publicado nesta etapa.
