# M0 — validação do build clássico Windows

Registro: 16/11/2026. **Build e inspeção do pacote aprovados; abertura no Windows ainda pendente.** Esta evidência cobre o frontend clássico, sem implementação cinematográfica.

## Origem e execução

| Item | Evidência |
| --- | --- |
| Pull request | [PR #1 — desenvolvimento remoto e CI](https://github.com/Hakayze/Hakas-Fritchii_MOD/pull/1) |
| Branch de trabalho | `chore/remote-dev-windows-ci` |
| Commit da branch compilada | [`d3bb226936aac5af923ac066fbdad3d6a490c170`](https://github.com/Hakayze/Hakas-Fritchii_MOD/commit/d3bb226936aac5af923ac066fbdad3d6a490c170) |
| Checkout executado pelo CI | [`76b964b40afc71eef74b7a6a6fb0cb47d887f2b0`](https://github.com/Hakayze/Hakas-Fritchii_MOD/commit/76b964b40afc71eef74b7a6a6fb0cb47d887f2b0) — merge temporário do PR |
| Árvore de arquivos | `47a3e14ebba499da9e1f71c88f9963bab442478d`, idêntica nos dois commits acima |
| Base preservada | `nx2` em `b24db9637085e48b50091edcbab49cb990684768` |
| Workflow | [Hakas Windows baseline](https://github.com/Hakayze/Hakas-Fritchii_MOD/blob/d3bb226936aac5af923ac066fbdad3d6a490c170/.github/workflows/windows-baseline.yml) |
| Execução | [43906260116](https://github.com/Hakayze/Hakas-Fritchii_MOD/actions/runs/43906260116) — `success` |
| Job | [Build classic Windows x64](https://github.com/Hakayze/Hakas-Fritchii_MOD/actions/runs/43906260116/job/133435812062) — todas as etapas concluídas com sucesso |
| Intervalo do job | 16/11/2026, 16:19:51–17:12:41 UTC (12:19:51–13:12:41 em Manaus) |
| Compilação dos targets | 16:23:43–17:12:16 UTC, aproximadamente 48 min 33 s |

O checkout de um PR usa um commit de merge temporário. A igualdade das árvores foi conferida pela API do GitHub; o conteúdo compilado corresponde ao commit da branch informado acima. Alterações posteriores apenas de documentação não estão incluídas neste binário.

## Ambiente registrado nos logs

| Ferramenta / ambiente | Versão ou configuração observada |
| --- | --- |
| Executor | GitHub-hosted `windows-2022`, x64 |
| Compilador | MSVC 19.44.35230.0; ferramentas 14.44.35207 |
| Windows SDK | 10.0.26100.0 |
| CMake | 3.31.6 |
| Ninja | 1.13.2 |
| Qt | 6.8.3, kit `msvc2022_64`, módulo adicional `qtcharts` |
| Vulkan SDK | 1.4.341.1 |
| Configuração | Release, Ninja, compilação com 4 tarefas em paralelo |

As versões do runner e dos SDKs são evidência desta execução; não definem por si só o Windows mínimo de distribuição.

Comandos e flags completos estão no [workflow utilizado](https://github.com/Hakayze/Hakas-Fritchii_MOD/blob/d3bb226936aac5af923ac066fbdad3d6a490c170/.github/workflows/windows-baseline.yml). Pontos relevantes:

- `BUILD_TESTING=OFF` e `YUZU_TESTS=OFF`: esta execução não rodou testes unitários.
- `YUZU_CMD=ON`, `YUZU_ROOM=OFF` e `YUZU_ROOM_STANDALONE=OFF`.
- Qt externo ao mecanismo de download do CMake (`YUZU_USE_BUNDLED_QT=OFF`) e SDL3 empacotado habilitado.
- Compilação: `cmake --build build/windows-ci --target yuzu yuzu-cmd --parallel 4`.
- Deploy: `windeployqt --release --no-translations build/windows-ci/bin/lichii.exe`.

Nenhum módulo QML/Quick/Quick3D foi acrescentado nesta etapa. O workflow tem token de leitura e não cria releases.

## Pacote inspecionado

[Artefato 14249543059](https://github.com/Hakayze/Hakas-Fritchii_MOD/actions/runs/43906260116/artifacts/14249543059)

- Nome: `hakas-classic-windows-x64-unverified-76b964b40afc71eef74b7a6a6fb0cb47d887f2b0`.
- ZIP: 118.160.223 bytes; 41 arquivos; 293.384.019 bytes descompactados.
- SHA-256 do ZIP baixado, igual ao informado no upload:
  `17fe273ecca6a382e0ca0f6a0683aa981714b0b5886b9eed15eef80aa8184874`.
- Retenção configurada: três dias. Expiração registrada: **19/11/2026 às 17:12:16 UTC** (13:12:16 em Manaus). Após a expiração, o link pode ficar indisponível.
- Integridade do ZIP: CRC de todos os arquivos conferido, sem erros.

| Executável | Tamanho | SHA-256 |
| --- | ---: | --- |
| `lichii.exe` | 41.360.384 bytes | `f69366bbe2fbfc7f550707a1e3349466a96873cc6c383b0b713c1c09c9238361` |
| `eden-cli.exe` | 37.644.800 bytes | `9902e220002fdad39a90392ac0feb8be6c44a9b3ee126eeffcca779ddce43e09` |

Os dois executáveis têm cabeçalhos PE AMD64 (máquina `0x8664`). Foram encontrados Qt6Core, Gui, Widgets, Charts, Network, OpenGL, OpenGLWidgets e Svg, o plugin `platforms/qwindows.dll`, plugins de imagem/rede/TLS e o instalador `vc_redist.x64.exe`.

A inspeção estática das importações encontrou as DLLs Qt diretas dos executáveis no pacote. DLLs do Windows e do runtime MSVC não são todas copiadas para a pasta; o instalador do runtime está incluído. Inspeção de arquivos e imports não comprova que o processo abre nem substitui o teste numa instalação Windows sem ferramentas de desenvolvimento.

## Teste de abertura para concluir M0

1. Baixar o ZIP acima e extrair **todos** os arquivos numa pasta separada, sem substituir a instalação atual.
2. Criar uma pasta chamada `user` ao lado de `lichii.exe`, antes de abri-lo. O caminho portátil é definido por [`PORTABLE_DIR`](https://github.com/Hakayze/Hakas-Fritchii_MOD/blob/d3bb226936aac5af923ac066fbdad3d6a490c170/src/common/fs/fs_paths.h#L12); a prioridade desse diretório no Windows está em [`path_util.cpp`](https://github.com/Hakayze/Hakas-Fritchii_MOD/blob/d3bb226936aac5af923ac066fbdad3d6a490c170/src/common/fs/path_util.cpp#L114).
3. Abrir `lichii.exe`. Se o Windows indicar ausência de runtime MSVC, instalar o `vc_redist.x64.exe` incluído e tentar novamente.
4. Confirmar que a janela aparece, os menus respondem e o aplicativo fecha normalmente. Neste teste de abertura não é preciso iniciar um jogo. Registrar avisos ou erros, sem tratá-los automaticamente como falha de compilação.
5. Fechar e abrir uma segunda vez, conferindo se a configuração portátil é mantida.
6. Registrar a versão/build do Windows, CPU, RAM, GPU e driver, além do resultado e de eventual mensagem de erro. Não atualizar o executável durante a validação deste pacote.

| Verificação | Resultado |
| --- | --- |
| Configuração e compilação clássica | Aprovadas no CI |
| Existência dos executáveis e deploy Qt | Aprovados no CI |
| Integridade do ZIP e inspeção estática | Aprovadas |
| Abertura, menus, fechamento e segunda abertura no Windows | **Pendente** |
| Instalação sem Qt/SDK no PATH | **Pendente** |
| Jogabilidade e regressões de emulação | Não avaliadas |
| Interface cinematográfica / desempenho 3D | Ainda não implementados |

M0 permanece parcialmente concluído. Após o teste de abertura, registrar a máquina e o resultado neste documento; depois iniciar M1 em branch separada, preservando a opção de build Clássico.
