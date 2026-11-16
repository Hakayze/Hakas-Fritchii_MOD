# Hakas-Fritchii_MOD

Interface cinematográfica 3D nativa para o [Fritchii](https://github.com/Frizti/Fritchii), inspirada na experiência visual do BoxShelf.

O projeto pretende oferecer dois layouts selecionáveis: **Clássico**, com a interface existente, e **Cinematográfico**, com C++, QML e Qt Quick 3D. Ambos deverão compartilhar biblioteca, configurações, controles, saves e inicialização dos jogos.

## Estado atual

**Protótipo M1 em validação.** O build clássico Windows de M0 passou no CI, e o usuário confirmou a abertura da interface existente. A branch `feat/cinematic-ui` acrescenta uma cena de teste Qt Quick 3D opcional, sem alterações no sistema de emulação.

A opção `ENABLE_CINEMATIC_UI` fica OFF por padrão. O build ON oferece **View → Cinematic 3D Prototype (Experimental)**, com três caixas de arte original e retorno ao Clássico. Os builds Windows ON/OFF e os testes de renderização e integração têm validação própria; o pacote M0 continua exibindo a interface clássica. Consulte o [escopo M1](docs/hakas/M1_PROTOTYPE.md) e o [PR #2](https://github.com/Hakayze/Hakas-Fritchii_MOD/pull/2).

## Documentação

- [Visão do projeto e arquitetura](docs/hakas/README.md)
- [Roteiro e critérios de conclusão](docs/hakas/ROADMAP.md)
- [Evidências do build e abertura clássica — M0](docs/hakas/M0_VALIDATION.md)
- [Protótipo 3D opcional e testes — M1](docs/hakas/M1_PROTOTYPE.md)
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
