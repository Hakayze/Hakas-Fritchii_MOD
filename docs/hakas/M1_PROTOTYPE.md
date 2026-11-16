# M1 — protótipo de integração Qt Quick 3D

Este incremento acrescenta uma cena de calibração à janela existente. As três caixas usam arte geométrica original; ainda não representam a biblioteca de jogos. O pacote de M0 continua sendo a base clássica.

## Compilar e abrir

`ENABLE_CINEMATIC_UI` fica **OFF por padrão**. Nesse caminho, fontes, recursos e links cinematográficos não são adicionados, e a configuração não exige Qml, Quick nem Quick3D.

Para o experimento, usar Qt 6.8 ou superior da mesma instalação/ABI, com Qml, Quick e Quick3D. O CI usa Qt 6.8.3 MSVC2022_64 e instala `qtcharts qtquick3d qtshadertools qtquicktimeline` no job habilitado. A configuração conhecida usa `YUZU_USE_BUNDLED_QT=OFF`.

```powershell
cmake -S . -B build/cinematic -G Ninja -DCMAKE_BUILD_TYPE=Release -DYUZU_USE_BUNDLED_QT=OFF -DENABLE_CINEMATIC_UI=ON
cmake --build build/cinematic --target yuzu yuzu-cmd --parallel 4
```

Os demais flags e a instalação das dependências devem seguir o [workflow Windows](../../.github/workflows/windows-baseline.yml). A compilação completa no Windows é uma validação própria deste incremento, distinta do build clássico registrado em M0.

O executável inicia no Clássico. No build habilitado, selecionar **View → Cinematic 3D Prototype (Experimental)**. Setas esquerda/direita selecionam uma caixa; cima/baixo giram; clique seleciona; Esc ou o botão de retorno fecha a cena. Reabrir usa uma cena nova; a seleção de teste não é persistida.

## Integração

- `CinematicLibraryHost` contém uma `QQuickView` embutida por `QWidget::createWindowContainer`. O container assume a propriedade da view e controla sua visibilidade.
- A cena e o host são criados sob demanda. O QML é empacotado por `qt_add_qml_module`, URI `Hakas.Cinematic`, com recursos em `/qt/qml`.
- A biblioteca e seu modelo continuam sendo os existentes. Grid/Tree/Carousel e suas preferências não são substituídos por um novo enum.
- Enquanto a cena está aberta, a lista e o placeholder ficam ocultos, inclusive após notificações de descoberta assíncrona. Input e ativação da lista oculta são bloqueados.
- Ao iniciar qualquer tentativa de boot, a cena é fechada e o Clássico é restaurado. Cancelar ou falhar o boot também mantém o Clássico. O protótipo fica indisponível durante boot, emulação ou fechamento, inclusive com jogo em janela separada.
- Retorno e erros são processados por conexões adiadas; `QPointer` evita que eventos antigos atuem num host já destruído. O host é ocultado e removido do layout antes de `deleteLater()`.
- Erros QML ou de inicialização recuperável do scene graph retornam ao Clássico com mensagem. Backend Quick de software conhecido é recusado. Isso não garante recuperação de falhas fatais do driver ou ausência de DLLs exigidas na abertura do processo.

O backend gráfico do core e suas threads não são alterados. Não há integração de boot/progresso cinematográfico neste incremento.

## Validação registrada

Em 16/11/2026, a cena foi verificada localmente com Qt/PySide 6.8.3:

- `qmllint`, com os imports da mesma instalação: sem avisos.
- `QQuickView.Ready`, objeto raiz e foco ativo, sem mensagens QML.
- Eventos de teclado: seleção limitada aos índices 0–2, rotação limitada a -55°/55°, modificadores preservados, uma emissão de retorno por Esc e repetição de Esc ignorada.
- Diff sem erros de espaços e sintaxe YAML do workflow verificada; matriz `OFF`/`ON` com token de leitura.

Esses testes carregaram o QML sem mostrar a janela nem produzir frames GPU. Não comprovam renderização, picking visual, integração nativa QWidget nem comportamento no Windows.

O host C++ e seu moc foram compilados em objetos com headers/ferramentas oficiais Qt 6.8.3, GNU g++ 14.2.0, C++20 e `-Wall -Wextra -Werror`, sem erros. Esse teste não fez link nem executou a integração nativa. O CMake do harness local ficou limitado pelas dependências de desenvolvimento OpenGL ausentes no ambiente; a compilação completa e o empacotamento continuam sob responsabilidade do CI Windows.

O CI compila os dois caminhos em jobs separados, executa `qmllint` apenas no habilitado, usa `windeployqt --qmldir` e verifica os imports/dependências do pacote 3D. O pacote clássico é conferido para não incorporar QtQuick3D. Resultados e links dos jobs devem ser registrados após a execução, sem antecipar aprovação.

A [execução 43934085853](https://github.com/Hakayze/Hakas-Fritchii_MOD/actions/runs/43934085853) foi iniciada para o commit de implementação `ed028475d243854a531fe10b05be82e10f5837f5`. A configuração Windows do job ON e o lint QML já passaram; o resultado da compilação e dos pacotes deve ser conferido na própria execução. Commits posteriores somente de documentação não mudam o código desse build.

## Testes ainda necessários

- [ ] Builds Windows `OFF` e `ON` e inspeção de seus pacotes.
- [ ] Ativar a cena e voltar ao Clássico repetidamente, por menu, botão e Esc.
- [ ] Aparência das caixas, texturas e clique de seleção num frame real.
- [ ] Descoberta/atualização da biblioteca enquanto a cena está aberta.
- [ ] Controle não navega nem ativa a biblioteca oculta. Navegação do protótipo por gamepad ainda não foi implementada.
- [ ] Diálogos, popups, Alt+Tab, Tab, mudança de DPI e redimensionamento.
- [ ] Cancelamento/erro de boot e execução/parada nas janelas única e separada.
- [ ] Testes nas GPUs/backend dos PCs de destino e instalação sem Qt/SDK no PATH.
- [ ] Comparação com `QQuickWidget`, se os testes do container apontarem problemas de foco/composição.

Fullscreen da biblioteca não é uma funcionalidade herdada: o fluxo existente de fullscreen só atua durante a emulação. A meta de desempenho e a fidelidade da estante continuam pendentes dos marcos seguintes.

## Referências

- [QWidget::createWindowContainer](https://doc.qt.io/qt-6.8/qwidget.html#createWindowContainer).
- [QQuickView](https://doc.qt.io/qt-6.8/qquickview.html) e [erros do scene graph](https://doc.qt.io/qt-6.8/qquickwindow.html#sceneGraphError).
- [FrustumCamera](https://doc.qt.io/qt-6.8/qml-qtquick3d-frustumcamera.html), [View3D](https://doc.qt.io/qt-6.8/qml-qtquick3d-view3d.html), [PrincipledMaterial](https://doc.qt.io/qt-6.8/qml-qtquick3d-principledmaterial.html) e [Texture](https://doc.qt.io/qt-6.8/qml-qtquick3d-texture.html).
