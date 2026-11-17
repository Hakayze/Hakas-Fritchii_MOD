> Histórico da base nx2 anterior. A fonte atual é o Fritchii v5.8.4: consulte [BASE_CORRECTED.md](BASE_CORRECTED.md). Os testes registrados aqui não validam um novo build dessa versão.

# Decisões técnicas e consolidação das revisões

[Voltar ao projeto](README.md)

Data: 16/11/2026. Estado: decisões de projeto e verificações documentais/matemáticas; nenhum snippet externo foi compilado no Hakas.

## Proveniência

O planejamento original foi elaborado antes da implementação. Gemini forneceu um parecer; DeepSeek forneceu uma revisão textual. O material recebido do DeepSeek declara ausência de consulta aos repositórios e de execução dos exemplos.

As contribuições são entradas para revisão. Elas não constituem resultados de testes do projeto. O vídeo precisa estar disponível para qualquer avaliação de fidelidade visual. Um nome de arquivo diferente não prova ausência de acesso à gravação.

## Decisões

| Tema | Direção atual | Evidência / pendência |
| --- | --- | --- |
| Arquitetura | Widgets preservado; C++ integra dados e ações; QML/Quick 3D apresenta a nova UI | Proposta a validar em M0/M1 |
| Hospedagem | Comparar QQuickView/createWindowContainer e QQuickWidget | Decisão final por foco, modais, renderização e custo medidos |
| Biblioteca | Adapter sobre o modelo existente; gameKey estável e IDs u64 como strings | Confirmar roles e chamadas reais no commit de implementação |
| Câmera | FrustumCamera para projeção assimétrica da estante | Fórmula conferida numericamente sob as convenções abaixo; execução Qt pendente |
| Materiais | Calibrar cores e brilho; opacityMap disponível | API documental confirmada, aparência não validada |
| Boot | Estados explícitos e progresso real; manter o caminho de inicialização existente | Integração e sinais novos ainda propostos |
| Voo para inspeção | Preservar projeção e silhueta durante toda a transição | Estratégia precisa de protótipo; fórmula simplificada externa corrigida abaixo |
| Windows | Preservar Clássico, testar perfis e pacote sem SDK | Matriz real em WINDOWS.md, ainda pendente |

## Câmera: fórmula de calibração

Para uma referência de largura W, altura H, distância CSS P=1800 e origem vertical a=0,42, adotar a câmera na origem, olhando para -Z, Y para cima:

```text
X = x_css - 0,5 W
Y = a H - y_css
Z = z_css - P

left   = -0,5 W n / P
right  = +0,5 W n / P
top    = +a H n / P
bottom = -(1-a) H n / P
```

Aqui n é clipNear. Os limites do frustum são coordenadas no plano próximo; para manter a mesma projeção ao variar n, escalá-los proporcionalmente.

Com W=1920, H=1080 e n=1: left=-0,533333…, right=0,533333…, top=0,252 e bottom=-0,348.

A comparação numérica entre a projeção CSS e essas equações, em cinco pontos de calibração, apresentou erro máximo inferior a 10⁻¹⁰ pixel. Isso confirma a equivalência matemática nesses casos, não a cena Qt, as transformações de todas as caixas ou a fidelidade ao vídeo.

A política de redimensionamento e os sinais de rotação precisam ser validados. Manter as unidades consistentes ao converter o layout e os pivôs.

Fonte: [FrustumCamera, Qt 6.8](https://doc.qt.io/qt-6.8/qml-qtquick3d-frustumcamera.html).

## Materiais: correções de API

No PrincipledMaterial, a configuração sem iluminação é:

```qml
lighting: PrincipledMaterial.NoLighting
```

A documentação Qt 6.8 também inclui opacityMap, que permite variar a opacidade ao longo da superfície. Não precisamos adiar a confirmação da existência dessa propriedade para M3.

Cores exatas ainda dependem de texturas, espaço de cor, exposição e composição. Não usar emissão como substituto automático do brilho especular. PrincipledMaterial, material sem iluminação com glare separado e CustomMaterial continuam candidatos a comparação visual; a escolha precisa de captura e medição.

Fonte: [PrincipledMaterial, Qt 6.8](https://doc.qt.io/qt-6.8/qml-qtquick3d-principledmaterial.html).

## Adapter: correções e cuidados

- Conectar uma vez aos sinais do modelo fonte. Os índices fornecidos por dataChanged identificam os itens e seus pais; não há uma conexão separada a cada pai.
- Se a lista de roles de dataChanged estiver vazia, considerar todos os roles alterados.
- Sobreviver a reset usando identidade estável; resolver novamente índices após reconstrução.
- Tratar inserções, remoções, movimentos, reordenação e destruição do modelo.
- Evitar ponteiros pendentes; usar ownership e observação de QObject adequados.
- Deduplicar clones de favoritos sem colapsar jogos distintos com o mesmo ID em caminhos diferentes.
- Não assumir que uma conexão enfileirada torna segura a leitura direta de um modelo pertencente a outra thread.
- Reaproveitar critérios de busca existentes apenas após confirmar os símbolos e suas dependências no código.

Fonte: [QAbstractItemModel, Qt 6.8](https://doc.qt.io/qt-6.8/qabstractitemmodel.html).

## Boot: apresentação e estados

Um QTimer::singleShot(0) não comprova que a tela já desenhou um quadro; a ordem entre timers de intervalo zero e outras fontes de eventos é indefinida. A espera por um quadro deve usar evidência do ciclo de renderização e encaminhar a chamada de boot para a thread apropriada.

Não chamar BootGame diretamente de um callback da thread de renderização. Não usar processEvents reentrante ou mover o core de thread apenas para animar o overlay.

Estados propostos: solicitação recebida, validação/preparação, cancelada, falhou, carregando, jogo pronto para exibição, executando e emulação encerrada. “Jogo pronto” e “emulação encerrada” precisam de notificações distintas.

Bloquear tentativas duplicadas já na entrada, antes de agendar a ação. Relacionar resultados à tentativa ativa para ignorar notificações antigas. Representar progresso indeterminado quando não houver dados reais.

Fontes: [QTimer](https://doc.qt.io/qt-6.8/qtimer.html), [QQuickWindow](https://doc.qt.io/qt-6.8/qquickwindow.html) e [desempenho QML](https://doc.qt.io/qt-6.8/qtquick-performance.html).

## Voo: correção da distância simplificada

Para uma caixa frontal de altura h em unidades de mundo que deve ocupar uma fração f da altura do viewport, com FOV vertical theta:

```text
distancia = h / (2 f tan(theta / 2))
f = altura_projetada_em_pixels / altura_do_viewport_em_pixels
```

A fórmula externa sem f corresponde somente ao caso de ocupar toda a altura do viewport. Por exemplo, h=500, viewport de 1080 px, altura projetada desejada de 500 px e FOV de 30° exigem aproximadamente 2015,31 unidades, não 933,01.

Esse cálculo é uma checagem de escala para uma face frontal. Uma caixa inclinada exige comparar a projeção dos cantos, profundidade, silhueta e perspectiva. Igualar apenas altura, pose de mundo ou acrescentar crossfade não demonstra uma transição contínua.

Não rejeitar antecipadamente uma cena ou câmera única: testar uma projeção contínua antes de adotar duas cenas e um handoff mais complexo.

## Limites

Qt 6.8.3 é a referência inicial identificada no build herdado, não a versão mínima de introdução de todas as APIs. Tempos, taxas de quadros e limites de memória são metas até que existam medições.

O [planejamento original](PLANEJAMENTO_ORIGINAL.md) permanece histórico; estas correções orientam a implementação futura.

