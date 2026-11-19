Fritchii v5.7.

#### Novidades
- **Jogos .nx** (NXFormat de fab918). O Fritchii abre arquivos `.nx` como qualquer outro jogo, com suas atualizações e DLCs. Para criar um, clique com o botão direito em um jogo na lista e escolha **Converter para NX...** (a partir de NUP, NSP, XCI ou de uma pasta de dump; atualizações e DLCs em NSP e XCI na mesma pasta também são incluídos). **Gerenciar arquivo NX** adiciona ou remove atualizações e DLCs depois. Tudo roda dentro do próprio Fritchii, sem precisar de arquivos extras (zortos293/Lichii#923, #1078 e #1232 por fab918).

#### Correções
- **Geometria esticada ou brilhante e áreas brancas após mudanças de área** devem ter desaparecido. Quando o jogo liberava parte da memória da GPU e reutilizava o mesmo endereço depois, o emulador continuava mostrando à GPU a memória antiga, que àquela altura já continha outros dados, fazendo com que uma malha (mesh) pudesse ler um pacote de arquivos do jogo como seus vértices até reiniciar (zortos293/Lichii#1199 e correções subsequentes por PantACRO4life, 99ivs e zortos293). Se você ainda vir algo assim, use Ajuda > Exportar log e publique no canal Issues.
- **Ctrl+U (alta taxa de atualização) funciona com o mod Hyrule Cheat Menu instalado.** O patch do Fritchii foi movido para um local que o mod não usa, e o código do mod permanece intacto.
- **Cemitério de Kakariko:** dois objetos desenhados pelo jogo estavam sendo ignorados (zortos293/Lichii#279, #1186 e #1181).
- **Fim dos crashes por falta de memória em PCs com 16 GB de RAM:** o cache de texturas agora também respeita o limite que o Windows consegue alocar, e não apenas o que a GPU possui (zortos293/Lichii#1210).

#### Mais rápido e mais fluido
- **Cerca de 5 fps a mais em combates** (de 51,9 para 56,9 em uma luta contra Stalchild em uma RTX 4080): quando o jogo lê de volta algo gerado pela GPU, a thread da GPU do emulador não para mais para esperar a placa de vídeo (a base de zortos293/Lichii#1028).
- **Menos engasgos logo após mudanças de área.** OoT roda em 6 núcleos de CPU emulados e cada um compilava o mesmo código do jogo para si após um carregamento; agora, os núcleos ociosos fazem isso antecipadamente.
- **Menos VRAM e mais RAM livre:** o cache de texturas faz a limpeza a partir de no máximo cerca de 7,5 GB, mantendo a VRAM por volta de 6 GB em placas maiores em vez de subir para 10 GB. O Windows também contabiliza essa memória contra a sua RAM, então os carregamentos de área param de engasgar quando o PC fica com pouca memória. Placas com menos VRAM mantêm seu próprio limite inferior.
- **Travamentos mais curtos em carregamentos e abates:** a leitura de arquivos do jogo não fica mais limitada a 40 MB/s, e a limpeza de memória em segundo plano aguarda enquanto o jogo está carregando.
- **Ajuda > Exportar log agora informa o que causou cada engasgo**, tornando os relatos de stutter muito mais fáceis de investigar.
- As configurações otimizadas do Fritchii são aplicadas novamente com esta atualização; você pode alterar o que quiser depois.

### Na v5.6

#### Correções
- **Os enormes triângulos brilhantes ("feixes de luz") estão muito mais raros.** Em Kokiri Forest, Kakariko, no cemitério e em Hyrule Field eles costumavam se multiplicar ao longo de uma sessão. Uma das causas foi corrigida: um compute shader escrevia além do que o emulador rastreava, e o jogo lia dados antigos de volta dali. Agora, todo compute shader é verificado uma vez e rastreado por completo se escrever além do limite. Sem custo de fps. Eles ainda podem aparecer em sessões longas: se você vir um, use Ajuda > Exportar log e publique no canal Issues.
- **AMD: fim dos travamentos de GPU em draws feitos pelo jogo com mesh shaders** (zortos293/Lichii#814). O driver da AMD agora passa por um fallback por compute para eles. Nova configuração **Mesh Shaders** em Gráficos (Automatic, Native, Compute) se você quiser testar o outro caminho.
- **AMD: fim do contorno arco-íris estilo "discoteca"** onde montanhas e cenários distantes encontram o céu. Draws que não escrevem em todos os canais de cor deixavam o restante indefinido, e o driver da AMD preenchia isso com lixo de memória. Port do Lichii #353 de jpkto (Fritchii #29 por trushna88).
- **As cutscenes de abertura e o vídeo do menu principal ficam sincronizados com o áudio.** O jogo exibe apenas cerca de 30 quadros por segundo nessas telas, então os vídeos de 60 fps rodavam na metade da velocidade enquanto a música continuava (quase 7 segundos de atraso após 12 segundos). Os vídeos agora exibem o quadro que corresponde ao áudio e pulam o restante. Um quadro de vídeo corrompido ou um decodificador com falha não derrubam mais o jogo.
- **A Song of Time não termina mais com um estalo alto** (zortos293/Lichii#1073 por justin-wq1337).
- **Fim dos congelamentos de até 3 segundos** quando o jogo troca um compute shader no meio da jogatina (zortos293/Lichii#969 por El-Sashok).
- **Desbloquear o limite de velocidade não faz mais OoT rodar acelerado.** Acima de 60 fps o jogo inteiro acelerava; agora ele permanece na velocidade normal. Turbo e Slow continuam funcionando como avanço rápido e câmera lenta. Em OoT, Ctrl+U agora alterna o modo de alta taxa de atualização (veja em Novidades).
- **Abrir a ocarina sem um microfone conectado não causa mais crash no jogo** (zortos293/Lichii#497 e #881, via Fritchii #16).
- Mods que aplicam patches no mesmo local dos patches integrados do Fritchii têm prioridade sobre eles (Fritchii #5). Títulos de jogos de arquivos NUP são exibidos corretamente (Fritchii #20). Valores vazios no mapeamento de controles não são mais perdidos (Fritchii #30 por FFcustoms). O Fritchii volta a compilar no Linux (Fritchii #19).
- **Ajuda > Exportar log sempre envia o log desta sessão.** Um Fritchii que não terminou de fechar (por exemplo, após uma atualização) podia manter o arquivo de log travado, fazendo com que a exportação enviasse um log antigo.

#### Novidades
- **Alta taxa de atualização (experimental): pressione Ctrl+U em OoT para passar de 60 fps** na velocidade normal do jogo, e pressione novamente para voltar, sem precisar reiniciar. O limite fica em Gráficos > Avançado (taxa de atualização do seu monitor, 90, 120, 144 ou 165), e a caixa de seleção em Gráficos acompanha o Ctrl+U. As ondulações da água também continuam se movendo acima de 60 (zortos293/Lichii#1152, após #655 por atk3001). É um recurso novo: avise se algum temporizador, cutscene ou minigame agir de forma estranha com ele ligado. O modo 30 FPS ainda tem prioridade sobre ele.
- **O mod independente Dynamic FPS do Dark é ignorado e ocultado automaticamente**, já que o Fritchii o traz integrado (o mod também bloqueava o modo de alta taxa de atualização). Seus arquivos não são alterados. Se você desativou o recurso integrado em Complementos para usar o mod, nada muda para você.
- **Restaurar padrões** (Configurações > Geral) coloca todas as opções de volta nas configurações otimizadas do próprio Fritchii, após salvar um backup. **Restaurar configurações** traz de volta qualquer conjunto salvo. Controles e pastas de jogos são mantidos.
- **"Minhas configurações não aplicam" explicado:** quando OoT tem valores próprios em Configurar jogo, a janela de configurações agora informa isso no topo, com um botão para devolver OoT às suas configurações globais. O Exportar log também lista esses valores.
- **RAM e VRAM no overlay de desempenho**, mostrando quanto ainda resta no seu PC (em âmbar quando estiver ficando baixo).
- **Relatos de "Shaders compilam novamente no jogo" agora se explicam sozinhos:** o log agora mostra o tamanho do cache de shaders do próprio driver da GPU na inicialização, após a preparação e ao sair, se a última sessão fechou corretamente e quão rápida foi a preparação.
- **Os vídeos de OoT voltam a ter a aparência que deveriam em NVIDIA RTX.** Os vídeos da tela de título e das cutscenes passam novamente pelo próprio upscaler do jogo (ele remove banding e aplica nitidez), em vez do esticamento simples que todas as builds usavam desde a v5. Não custa nada perceptível em placas RTX. GPUs que precisariam emular esses cálculos mantêm o esticamento rápido. Gráficos > "Ocarina of Time movies" permite escolher entre os dois.
- **Botões Discord e Donate** na tela inicial e no menu Help.

#### Mais rápido e mais leve
- **Cerca de 1 GB a menos de RAM:** a memória que o jogo zera é devolvida ao Windows.
- **PCs com menos de 24 GB de RAM** mantêm menos pipelines antigos de GPU na memória, fazendo sessões longas paginarem menos.
- **Cerca de 5 fps a mais em Castle Town** (de 45 para 50 em uma RTX 4080 parado no mercado): o emulador parou de enviar milhares de submissões vazias por segundo para a GPU, e reports e fences não alocam mais memória a cada vez (parte de zortos293/Lichii#1161).
- **Fim do acúmulo de VRAM:** render targets antigos substituídos pelo jogo são limpos, então a VRAM fica por volta de 5,5 GB em Castle Town em vez de encher até 10-11 GB, e os congelamentos de quando ela lotava sumiram (especialmente em combates).
- **Sessões longas:** pipelines reconstruídos após serem limpos mantêm seus buffers, impedindo que uma sessão longa quebre lentamente algumas malhas (zortos293/Lichii#1143).
- **Menos quedas em áreas movimentadas:** um depth buffer que o jogo limpa imediatamente não é mais copiado antes da memória do jogo (7,5 MB a cada quadro em Castle Town, a causa das quedas de um jogador de NVIDIA). Cerca de +2,5 fps em Castle Town.
- **A memória de texturas em placas de 10 GB ou mais** pode preencher mais antes de a limpeza começar (zortos293/Lichii#1129 por Liomar Oliveira).
- **FXAA vem desativado por padrão.** O próprio anti-aliasing de OoT (ou DLSS) já suaviza as bordas; FXAA por cima apenas borra a imagem. As configurações otimizadas do Fritchii são aplicadas novamente com esta atualização; você pode alterar o que quiser depois.

### Na v5.5

- **Fim da câmera piscando para jogadores que tinham frame generation ativado antes da v5.** O Fritchii não tem frame generation, mas uma configuração antiga o mantinha rodando em segundo plano (e desligava o frame pacing do Fritchii). Ele não pode mais rodar.
- **Extended Dynamic State é redefinido para EDS2 uma vez.** Jogadores que definiram como Disabled ou EDS3 (muitas vezes por uma dica antiga de crash na AMD) faziam o "Preparing shaders" pular quase todos os shaders. EDS2 é o modo com o qual os shaders são gravados, e o crash da AMD que essa dica evitava já foi corrigido desde a v5.3. Você ainda pode alterar depois.
- **NVIDIA: o tamanho do cache de shaders é definido pelo próprio Fritchii.** Drivers mais novos não mostram mais "Shader Cache Size" no Painel de Controle da NVIDIA. O Fritchii agora informa ao driver para manter um cache grande apenas para o Fritchii; nada mais no seu PC muda.
- **Correções de DLSS:** a caixa de seleção "Use NVIDIA DLSS" podia ser sobrescrita por um valor antigo por jogo e aparecer desmarcada no jogo; agora é uma configuração global. Nova opção **DLSS preset** (Default, J, K recomendado, L, M) em Gráficos.
- Gráficos > Extensões agora deixa claro: o GPU thread pinning é integrado e fica ligado automaticamente.
- A antiga janela "Prepare Shaders" agora informa que é o aquecimento rápido opcional (a preparação completa roda sozinha quando você inicia o jogo).

### Na v5.4

- **AMD: "Preparing shaders" agora prepara o jogo inteiro.** Até agora ele pulava 116.373 dos 120.461 shaders de OoT na AMD (terminava em segundos) porque eles foram gravados em uma placa NVIDIA, que lida com uma configuração da GPU (a dynamic logic operation) de forma diferente. O Fritchii agora os converte para o driver da AMD: todos eles são preparados, então novas áreas rodam como se você já tivesse estado lá antes, assim como na NVIDIA. Obrigado a koriko pelo log que identificou a causa.
- **AMD: espere uma tela completa de "Preparing shaders" uma vez** (cerca de 15 minutos em uma CPU de 8 núcleos), mesmo que você já tenha passado por ela na v5.x. Depois disso, vai direto para o jogo.
- Nada muda na NVIDIA.

### Na v5.3

- **AMD: Shader libraries agora estão realmente desativadas por padrão.** A v5.2 tinha a intenção de desativá-las no driver da AMD para Windows, mas sua verificação nunca correspondia, então placas RX 5000/6000 continuavam dando crash ao carregar shaders. Se você mesmo desmarcou a configuração, nada muda para você.
- **As atualizações tentam novamente sozinhas.** Quando o servidor de arquivos do GitHub responde com um erro como 504 ou a conexão cai, o Fritchii agora tenta de novo algumas vezes antes de desistir (a partir da próxima atualização).

### Na v5.2

- **Série GTX 10 e anteriores: geometria invisível volta a ser desenhada.** Objetos que o jogo desenha com mesh shaders, recurso que essas placas não possuem, agora passam por um fallback por compute (protótipo de zortos, zortos293/Lichii#569, #642, #841). Parte da vegetação ou algumas paredes ainda podem faltar ou piscar.
- **AMD: texturas não escrevem mais na memória de outras texturas.** Imagens cujo tamanho não é múltiplo do tamanho do seu bloco escreviam além da borda durante a decodificação. A NVIDIA ignora isso, mas a AMD jogava esses dados em outras texturas: blocos pretos, um padrão quadriculado e, no fim, um crash da GPU. Do fork AMD de CheezerWheezer.
- **AMD (driver do Windows): Shader libraries vêm desativadas por padrão.** Placas RX 5000 e 6000 davam crash ao compilá-las; desmarcar a opção era a solução alternativa, agora é automático. A NVIDIA as mantém ativadas.
- **O menu completo de clique direito está de volta na tela inicial** (Open Mod Data Location, Dump RomFS, Remove, Verify Integrity, Start Game without Custom Configuration...), para modders.
- **Fim dos crashes quando o limite de memória do PC é atingido** durante sessões longas (visto em Castle Town depois de meia hora). Buffers na memória do sistema agora podem ultrapassar o orçamento do Windows, assim como as texturas já podiam. Também do fork AMD de CheezerWheezer.

### Na v5.1

- **Textos de diálogo não viram mais retalhos de outras texturas.** Cada novo tamanho de caixa de texto podia mostrar lixo gráfico na primeira vez em que aparecia.
- **Excluir cache de shaders** no menu Help (e quando você clica com o botão direito no jogo na tela inicial), com confirmação. Apenas para um cache que corrompeu: os shaders são preparados novamente na próxima inicialização.
- **AMD: provável correção para um crash logo após pressionar Play** (visto em uma RX 6600 com um cache de shaders da v4). A mudança da v5 para a linha vermelha em Royal Family's Tomb foi revertida por enquanto, então essa linha pode aparecer novamente na AMD até ser refeita e testada em hardware AMD.
- Uma alteração da v5 na forma como a memória da GPU é atualizada após pequenas escritas também foi revertida: neste emulador ela podia trazer de volta dados obsoletos.
- **Telas 4K e com escala ajustada:** o Fritchii segue a escala definida no Windows, até onde sua tela der espaço para as janelas. 150% em uma tela 4K virava 200% (janelas grandes demais, caixas de diálogo cortadas) e 125% virava 100% (texto minúsculo).
- **Game Settings cabe na tela:** abas longas têm rolagem, a janela pode ser redimensionada e OK / Apply ficam sempre acessíveis.

### Todas as novidades da v5

Windows x64. Testado com The Legend of Zelda: Ocarina of Time (0400486024DEC000) em um Ryzen 7 7800X3D + RTX 4080: 60 fps estáveis em muitos lugares, raramente abaixo de 45 mesmo nos mais pesados (Water Temple incluído), sem engasgos de shader após a preparação.

#### Novidades
- **Os shaders são preparados uma vez, antes de você jogar.** Na primeira vez em que você inicia OoT, uma tela "Preparing shaders" compila os shaders do jogo em velocidade máxima (cerca de 15 minutos em uma CPU de 8 núcleos, uma única vez). Depois disso, lugares que você visita pela primeira vez rodam como se você já tivesse estado lá antes: entrar em uma nova área costumava congelar por ~0,7 s por novo shader; agora leva de 40 a 70 ms, e os carregamentos estão mais curtos. Um novo driver ou GPU os prepara novamente. Baseado no trabalho de pré-compilação de zortos (zortos293/Lichii#895, #896).
- **Novo visual.** Tema Graphite, tela inicial, "What's new" dentro do aplicativo e Ajuda > Exportar log para relatos de bugs.
- **Controles no jogo.** Mova o mouse para o topo da tela do jogo para Pause, Restart, Stop, Fullscreen e os menus.
- **Novo gráfico de desempenho** (Exibir > Indicadores de desempenho): fps em destaque, frame time, o menor fps recente e uma linha de referência em 60 (ou 30). Telas de carregamento deixam um espaço vazio em vez de uma queda.

#### Mais rápido
- **NPCs em movimento e multidões não derrubam mais a taxa de quadros.** Cerca de +3 fps em Castle Town para cada uma das correções envolvidas (realocação de render targets, image pool, um frame clock exato, limpeza de cópias obsoletas) e ~2,4 GB a menos de memória de vídeo (de 8,8 para 6,4 GB em Castle Town).
- **~2,3 GB a menos de RAM**, dando mais folga para PCs de 16 GB.
- **Windows 11: relógio de jogo preciso** integrado. Jogadores que tinham grandes ganhos com o ajuste de registro GlobalTimerResolutionRequests conseguem isso sem o ajuste.
- Tradução de shaders movida para fora da thread da GPU.
- Build otimizada guiada por perfil (profile-guided) retreinada no novo código.

#### Correções
- **Novos jogadores: "Press Start", balões de diálogo e iluminação** ficam corretos em um cache de shaders vazio (antes ficavam ausentes ou errados enquanto os shaders ainda estavam compilando).
- **Objetos que podiam sumir por uma sessão inteira** em alguns jogos (clip distances, zortos293/Lichii#902).
- **Fastmem mantido em PCs com pouca memória livre** em vez de rodar silenciosamente 20-25% mais lento (zortos293/Lichii#645).
- Menos RAM enquanto os shaders são preparados (#878), um deadlock raro de canal (#609), pastas de mods que não existem (#686), caminhos longos do Windows (#576) e algumas menores (#918, #727, #610).

#### Cache de shaders
- O cache passa para a versão 54 (numeração de zortos), então os caches das builds de zortos e do Fritchii são compartilhados. A primeira inicialização copia seu cache antigo uma vez (cerca de +1,2 GB para OoT). O arquivo `v52` antigo permanece para que você possa voltar à v4; você pode excluí-lo depois se não pretende fazer isso.

#### Problemas conhecidos
- As alterações para AMD e Intel não puderam ser testadas nesse hardware por aqui: por favor, relate problemas informando o modelo da sua GPU e enviando o log (Ajuda > Exportar log).

Dynamic FPS por Dark (integrado). Obrigado a zortos293 e a todos os colaboradores do Lichii cujo trabalho está aqui, e ao Discord **Ocarina of Forks** pelo apoio moral, pelas ideias inovadoras e pela ajuda na depuração.
