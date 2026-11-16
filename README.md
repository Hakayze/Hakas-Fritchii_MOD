# Hakas-Fritchii_MOD

Interface cinematográfica 3D nativa para o [Fritchii](https://github.com/Frizti/Fritchii), inspirada na experiência visual do BoxShelf.

O projeto pretende oferecer dois layouts selecionáveis: **Clássico**, com a interface existente, e **Cinematográfico**, com C++, QML e Qt Quick 3D. Ambos deverão compartilhar biblioteca, configurações, controles, saves e inicialização dos jogos.

## Estado atual

**Preparação e documentação.** O código-base do Fritchii foi preservado. A interface cinematográfica ainda não foi implementada ou compilada neste projeto; não há um executável Hakas validado para download.

O próximo marco é compilar e executar o Fritchii original no Windows e validar uma página Qt Quick 3D mínima com troca de layouts.

## Documentação

- [Visão do projeto e arquitetura](docs/hakas/README.md)
- [Roteiro e critérios de conclusão](docs/hakas/ROADMAP.md)
- [Compatibilidade e validação no Windows](docs/hakas/WINDOWS.md)
- [Decisões técnicas e revisão das contribuições](docs/hakas/DECISOES.md)
- [Responsabilidades e colaboração entre IAs](docs/hakas/COLABORACAO.md)
- [Planejamento original de 12/11/2026](docs/hakas/PLANEJAMENTO_ORIGINAL.md)
- [Guia de compilação herdado](docs/Build.md)

## Diretrizes

- Manter o sistema de emulação existente.
- Construir a nova interface dentro do próprio aplicativo.
- Preservar o modo Clássico e sua compatibilidade.
- Validar desempenho e fidelidade visual com medições e capturas.
- Distribuir as dependências necessárias no pacote final.

A proposta não utiliza Electron, WebEngine ou um launcher externo para desenhar a interface.

## Origem e licenças

Fork de [Frizti/Fritchii](https://github.com/Frizti/Fritchii), iniciado no commit `f66ea633db6058efe31a9611718019e5bd1667eb`. Referências visuais: [BoxShelf source](https://github.com/sassiestrhihno/boxshelf-source) e [BoxShelf beta](https://github.com/sassiestrhihno/boxshelf-beta).

Os avisos de autoria e as licenças herdadas permanecem aplicáveis. Consulte [LICENSE.txt](LICENSE.txt), [LICENSES](LICENSES) e o inventário do planejamento. Qt Quick 3D é disponibilizado sob GPLv3 ou licença comercial; assets de terceiros exigem avaliação própria antes de distribuição.

Mensagem original de Frizti preservada:

> Hi everybody i'm frizti i make things that work hopefully i love you

