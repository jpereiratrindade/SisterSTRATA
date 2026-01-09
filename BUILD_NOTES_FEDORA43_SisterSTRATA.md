# SisterSTRATA – Guia de Setup Fedora 43
## Relato Técnico de Dependências, Build e IA

Este documento registra os passos necessários para configurar o ambiente de desenvolvimento e execução do SisterSTRATA em um sistema Fedora 43.

**Configuração de Referência:**
- **OS**: Fedora 43 (Workstation)
- **GPU**: NVIDIA RTX 4060 (Driver Proprietário via RPM Fusion recomendado)
- **Engine**: Vulkan 1.3
- **IA**: Ollama 0.5.x

---

### 1. Ferramentas de Build e Dependências Gráficas

Para compilar o projeto, o sistema precisa das ferramentas de base e das bibliotecas de desenvolvimento gráficas que não são incluídas via `FetchContent`.

**Comando de Instalação:**
```bash
sudo dnf install cmake gcc-c++ SDL2-devel vulkan-devel glslc glslang glm-devel
```

> [!NOTE]
> O pacote `glslc` (Google Shaderc) é crucial para o processo de build do SisterSTRATA, pois automatiza a compilação dos shaders `.vert` e `.frag` para binários `.spv` durante o `make`.

---

### 2. Configuração do Ollama (Como Servidor de IA)

No Fedora, a instalação do Ollama via DNF não configura automaticamente o serviço de background (systemd).

#### Passo A: Instalação
```bash
sudo dnf install ollama
```

#### Passo B: Criação do Serviço Systemd
Crie o arquivo `/etc/systemd/system/ollama.service` com o seguinte conteúdo (ajuste o usuário):

```ini
[Unit]
Description=Ollama LLM Server
After=network-online.target
Wants=network-online.target

[Service]
Type=simple
ExecStart=/usr/bin/ollama serve
Restart=always
RestartSec=5
# Substitua <seu_usuario> pelo seu login unix
User=jpereiratrindade
Environment="OLLAMA_HOST=http://127.0.0.1:11434"
# Se usar GPU NVIDIA, garanta que os drivers estejam no PATH (geralmente automático no Fedora)

[Install]
WantedBy=multi-user.target
```

#### Passo C: Ativação
```bash
sudo systemctl daemon-reload
sudo systemctl enable --now ollama
```

---

### 3. Ajustes de Segurança e Permissões (SELinux/UNIX)

Se você encontrar erros de "Permission Denied" ao baixar modelos ou ao rodar o serviço, o problema geralmente é o contexto do SELinux ou permissões de pasta.

**Erro Comum:** `remove /home/.../.ollama/models: permission denied`

**Soluções:**

1.  **Permissões de Dono:**
    ```bash
    sudo chown -R $USER:$USER ~/.ollama
    chmod -R u+rwX ~/.ollama
    ```

2.  **Contexto SELinux (Fundamental no Fedora):**
    O serviço systemd pode ser bloqueado de acessar a pasta pessoal se o contexto não for o correto.
    ```bash
    sudo restorecon -Rv ~/.ollama
    ```

3.  **Monitoramento de Bloqueios:**
    ```bash
    sudo ausearch -m avc -ts recent
    ```

---

### 4. Verificação e Execução

Para garantir que o SisterSTRATA utilize a IA real e não o "Mock LLM":

1.  **Baixe o Modelo Recomendado:**
    Para a RTX 4060 (8GB VRAM), o modelo de 14b oferece o melhor equilíbrio para análises científicas:
    ```bash
    ollama pull qwen2.5:14b
    ```

2.  **Compilação e Run:**
    ```bash
    mkdir build && cd build
    cmake ..
    make -j$(nproc)
    ./bin/SisterSTRATA
    ```

---

### Dicas para Ambientes Corporativos/Institucionais

- **Proxy**: Se estiver atrás de um proxy, adicione `Environment="https_proxy=..."` na seção `[Service]` do `ollama.service`.
- **GPU**: Se o STRATA não detectar a GPU, verifique se instalou o `vulkan-loader` e os drivers via **RPM Fusion** (evite drivers baixados diretamente do site da NVIDIA em sistemas Fedora).

---
*Documentação gerada com base em testes reais no Fedora 43 (Jan 2026).*
