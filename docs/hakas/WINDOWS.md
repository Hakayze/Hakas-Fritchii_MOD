# Compatibilidade com Windows

[Voltar ao projeto](README.md)

## Requisito central

O novo frontend deve preservar a possibilidade de executar o modo Clássico nos PCs já compatíveis com o Fritchii. O modo Cinematográfico terá perfis ajustáveis, carregamento limitado de assets e recuperação para o Clássico quando tecnicamente possível.

“Todos os PCs com Windows” é um objetivo de ampla compatibilidade, não uma garantia de funcionamento em qualquer versão, arquitetura ou hardware. Os mínimos reais do emulador, Qt e drivers precisam ser conciliados e publicados após validação.

## Matriz a preencher em M0/M1

| Item | Registro necessário | Estado |
| --- | --- | --- |
| Windows | Edição, versão/build e arquitetura | Pendente |
| CPU e RAM | Modelo, recursos exigidos e consumo | Pendente |
| GPU integrada e dedicada | Modelo, driver, backend Quick e backend do jogo | Pendente |
| Qt | Versão, kit/ABI e módulos efetivamente instalados | Pendente |
| Compilador / SDK | Versões e opções usadas | Pendente |
| Instalação limpa | Sem ferramentas de desenvolvimento ou Qt no PATH | Pendente |

Windows 10/11 em x64 são candidatos para a matriz inicial. O suporte exato de versões e arquiteturas deve resultar da auditoria das dependências e dos testes; ainda não foi demonstrado por este fork.

## Configurações de distribuição

- **Clássico:** preservar o funcionamento herdado.
- **Cinematográfico completo:** efeitos calibrados pela referência.
- **Cinematográfico reduzido:** reduzir reflexos, efeitos e previews conforme medições.
- **Build sem Cinematográfico:** deve configurar e compilar sem exigir QML/Quick/Quick3D.

A seleção de perfil não deve alterar o sistema de emulação ou as configurações originais do modo Clássico.

## Qt e empacotamento

A pesquisa original identificou Qt 6.8.3 no workflow e uma opção empacotada apontando para 6.11.1. Isso não comprova a instalação dos módulos novos.

Para o frontend proposto, auditar `Qml`, `Quick`, `Quick3D` e, conforme a implementação, `QuickControls2` e `QuickWidgets`. Usar módulos da mesma instalação/ABI. Multimedia só será necessário se houver recursos que efetivamente o utilizem.

O pacote final deverá incluir DLLs, plugins, imports QML e recursos exigidos. Conferir descoberta de imports com `windeployqt --qmldir` no fluxo de deploy e executar o resultado em uma instalação limpa.

Um fallback em código não resolve a ausência de uma DLL necessária para o próprio processo iniciar. Preservar também o caminho de build/pacote Clássico sem dependências cinematográficas.

## Recuperação e desempenho

- Criar a cena sob demanda e limitar a quantidade de caixas/texturas carregadas.
- Reduzir trabalho visual enquanto o jogo executa.
- Tratar erros recuperáveis de imports, cena e assets com retorno ao Clássico.
- Projetar um caminho de recuperação na próxima abertura caso a inicialização cinematográfica falhe.
- Não assumir que toda falha fatal de driver permite recuperação no mesmo processo.
- O renderizador de software do Qt Quick não oferece conteúdo Quick 3D; nessa situação, o caminho de compatibilidade é o Clássico.

## Critérios para anunciar suporte

Publicar apenas combinações verificadas. Registrar também falhas e limitações conhecidas. Comparar o emulador base e o Hakas na mesma máquina para separar custo da interface e custo da emulação.

Consultar a [documentação oficial de requisitos do Qt Quick 3D](https://doc.qt.io/qt-6.8/qtquick3d-requirements.html) e o [roteiro de validação](ROADMAP.md). Os ambientes e caminhos de PCs citados nos guias herdados pertencem ao histórico upstream e não comprovam o ambiente do responsável por este projeto.

