> Histórico da base nx2 anterior. A fonte atual é o Fritchii v5.8.4: consulte [BASE_CORRECTED.md](BASE_CORRECTED.md). Os testes registrados aqui não validam um novo build dessa versão.

# Roteiro de implementação

[Voltar ao projeto](README.md)

Atualização: 16/11/2026. As fases abaixo são incrementais; estimativas de prazo só serão feitas após conhecer o ambiente de build e os bloqueios de M0.

## Situação

- [x] Nome e escopo definidos.
- [x] Pesquisa original disponível e revisões externas recebidas.
- [x] Fork na conta Hakayze e documentação inicial.
- [x] M0 parcial: build clássico Windows e pacote conferidos; [evidências](M0_VALIDATION.md).
- [ ] M0: abertura e funcionamento básico em Windows real.
- [ ] M1: prova de integração Qt Quick 3D.
- [ ] M2–M6: implementação, fidelidade e distribuição.

## Marcos

| Marco | Entrega | Critério para avançar |
| --- | --- | --- |
| M0 — base reproduzível | Inventário de ferramentas/dependências e build do Fritchii sem mudanças funcionais | Executável original inicia no Windows; commit, comandos, versões e logs registrados |
| M1 — integração mínima | Uma caixa de teste, página Quick e troca Clássico/Cinematográfico | Trocas repetidas, foco, teclado, controle, modal, fullscreen e convivência com Vulkan/OpenGL avaliados |
| M2 — biblioteca compartilhada | Adapter, seleção estável, busca, favoritos e ativação pelo fluxo existente | Mesmos jogos nos dois modos, sem duplicar favoritos; resets e remoção de jogos tratados |
| M3 — estante | Projeção, poses, materiais, fundo e navegação | Capturas e quadros intermediários comparados à referência; desempenho medido |
| M4 — detalhes e inspeção | Painel lateral, caixa articulada, cartucho e livreto por etapas | Seleção e origem de retorno preservadas; transição sem salto perceptível demonstrada |
| M5 — execução e retorno | Overlay, progresso real e estados de lançamento | Cancelamento, erro, jogo pronto e encerramento distintos; nenhuma tela de carregamento presa |
| M6 — distribuição | Pacotes Windows, dependências e inventário de licenças | Execução em instalação limpa sem SDK Qt; builds com e sem Cinematográfico verificados |

## Primeira tarefa de código: M0/M1

1. Conferir CMake, compilador, SDK Windows, arquitetura e Qt efetivamente disponíveis.
2. Reproduzir o build original antes de acrescentar módulos.
3. Registrar as opções de compilação realmente utilizadas.
4. Acrescentar a opção proposta `ENABLE_CINEMATIC_UI`, inicialmente desativada.
5. Com a opção ativada, exigir apenas os módulos necessários e incluir uma cena de calibração com arte original.
6. Comparar os hosts Quick; medir o bloqueio do boot existente sem alterar as threads do core.
7. Testar retorno ao Clássico e preservar suas preferências.

A opção de build acima é uma proposta; ainda não existe no código publicado nesta etapa.

## Verificação funcional

- [ ] Biblioteca vazia, descoberta em andamento e pastas indisponíveis.
- [ ] Modelos em árvore/plano, clones de favoritos e IDs iguais em caminhos distintos.
- [ ] Busca, favoritos e seleção após `modelReset`.
- [ ] Seleção de usuário, configuração por jogo e cancelamento do boot.
- [ ] Tentativa duplicada de iniciar; erro de arquivo ou carregamento.
- [ ] Parada, retorno, janela separada, tela cheia, Alt+Tab e troca de DPI.
- [ ] Teclado/controle sem ações duplicadas ou entrada vazando ao jogo.
- [ ] Saves e tempo jogado compartilhados com o fluxo existente.
- [ ] Recursos visuais suspensos ou liberados durante a emulação.

## Metas de desempenho e fidelidade

Metas iniciais, ainda sem benchmark do Hakas:

- Navegação a 60 fps em 1080p na máquina alvo a definir.
- Frame time aquecido: p95 até 16,7 ms e p99 até 33,3 ms.
- Resposta visual à entrada em até 100 ms.
- Posição de pontos principais: desvio até 2% da altura do quadro.
- Duração de transições: tolerância inicial de ±10% ou ±50 ms, o maior.

Registrar hardware, driver, resolução, perfil, tamanho da biblioteca, cache e método de medição. Separar arte, geometria e tipografia ao avaliar diferenças. Não aprovar fidelidade apenas pelos estados finais.

## Evidência mínima de cada entrega

Commit, escopo, comandos, resultado do build, verificações executadas, limitações e próximos bloqueios. “Compilou”, “iniciou” e “jogabilidade testada” são resultados distintos.

