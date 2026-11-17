> Histórico da base nx2 anterior. A fonte atual é o Fritchii v5.8.4: consulte [BASE_CORRECTED.md](BASE_CORRECTED.md). Os testes registrados aqui não validam um novo build dessa versão.

# Desenvolvimento remoto e validação Windows

Este guia se aplica ao `Hakayze/Hakas-Fritchii_MOD`. A branch principal é `nx2`, que deve permanecer estável até revisão e validação. O projeto ainda está em preparação (M0); compilar o frontend clássico não comprova que a futura interface cinematográfica funciona.

## Continuar de outro computador ou celular

1. Abra [o repositório](https://github.com/Hakayze/Hakas-Fritchii_MOD) autenticado na conta Hakayze.
2. Para cada mudança, crie uma branch a partir de `nx2` (por exemplo, `feat/quick3d-probe`) ou reutilize uma branch de trabalho correspondente.
3. Edite pelo GitHub, VS Code local ou Codespaces quando disponível. Em máquina compartilhada, não salve credenciais, ROMs, chaves ou saves pessoais.
4. Abra um pull request para `nx2`. Revise o diff e aguarde os checks do GitHub Actions. Não misture mudanças no núcleo do emulador com a interface sem justificativa.
5. Depois de uma validação bem-sucedida e revisão humana, faça a integração no GitHub. No PC doméstico, atualize o repositório antes de testar o executável.

Com Git instalado num computador pessoal:
```powershell
git clone https://github.com/Hakayze/Hakas-Fritchii_MOD.git
cd Hakas-Fritchii_MOD
git switch nx2
git pull --ff-only
git switch -c feat/minha-tarefa
# Depois de editar os arquivos:
git add .
git commit -m "feat: descreva a mudanca"
git push -u origin feat/minha-tarefa
```

## Validação do Windows

O workflow [Hakas Windows baseline](../../.github/workflows/windows-baseline.yml) é separado do workflow herdado de releases e roda em pull requests para `nx2` com mudanças nos caminhos relevantes. Depois que o arquivo estiver presente na branch principal, a opção **Run workflow** também estará disponível na aba Actions.

- Executor: `windows-2022` (Windows x64), MSVC, CMake/Ninja, Qt 6.8.3 e Vulkan SDK.
- Reutiliza os flags do build Windows clássico existente e tenta compilar os targets `yuzu` e `yuzu-cmd`.
- Verifica `lichii.exe` e `eden-cli.exe`, executa `windeployqt` e armazena um artefato temporário por três dias.
- Permissões do token: somente leitura. Não cria tags ou releases, não faz merge, não publica binários como produto final.
- Falha de dependência, SDK, configuração ou compilação deve ser investigada nos logs de cada etapa; não significa automaticamente regressão no código.
- **Limitação:** um check verde só prova que o build passou no ambiente do runner. Não comprova execução do app, qualidade do renderizador, compatibilidade ampla com Windows, jogabilidade ou visual cinematográfico.
- **Custo:** workflows em máquinas GitHub-hosted consomem cota de Actions conforme o tipo de conta e repositório; evite repetir builds desnecessários.

## Regras de segurança

- Nunca faça commit de ROMs, firmware, chaves de emulação, tokens ou dados pessoais.
- Mantenha `nx2` como destino dos PRs e não mescle código sem conferir diffs e checks.
- A proteção formal de branch e a exigência de aprovação/checks são configurações do GitHub; este guia e o workflow, sozinhos, **não** impedem push direto em `nx2`. Configure regras de proteção quando disponíveis no plano e repositório.
- Não habilite workflows com `pull_request_target` para compilar código de PRs não confiáveis.

## Próximos marcos

1. Fazer o CI clássico Windows passar e registrar logs, versão das ferramentas e commit testado (M0 parcial).
2. Executar o binário numa máquina Windows real, registrar a inicialização e as dependências (M0).
3. Somente depois criar a opção experimental `ENABLE_CINEMATIC_UI` e uma cena de teste Qt Quick 3D numa branch independente (M1).

Consulte [ROADMAP.md](ROADMAP.md), [WINDOWS.md](WINDOWS.md) e [DECISOES.md](DECISOES.md) para os requisitos oficiais do projeto.
