# BrazaReader

**BrazaReader** é um leitor de PDF minimalista e eficiente, otimizado para dispositivos com hardware limitado, especialmente Raspberry Pi Zero.

## Características Principais

### Interface
- ☰ Menu lateral animado para navegação
- Design minimalista e intuitivo
- Interface touchscreen capacitiva
- Orientação retrato fixa
- Tema escuro com cores personalizáveis

### Funcionalidades
- 📚 Leitura offline de arquivos PDF
- ⭐ Sistema de favoritos
- 📖 Histórico de leitura (última página lida)
- 🎨 Temas configuráveis com modo noturno
- 🔤 Ajuste de tamanho de fonte
- 💾 Persistência de configurações e progresso

### Desempenho
- Abertura de livros em menos de 2 segundos
- Renderização de página em menos de 300ms
- Máximo 5 páginas em cache de memória
- Renderização eficiente via MuPDF
- Consumo mínimo de CPU e memória

### Estrutura
- Arquitetura modular em camadas
- Separação entre UI, Engine e Storage
- Controle total de memória
- SQLite para persistência de dados

## Requisitos de Sistema

### Mínimos
- ARMv6 com hard-float (Raspberry Pi Zero)
- 512 MB de RAM
- 100 MB de espaço livre
- Touchscreen capacitivo

### Recomendado
- Raspberry Pi Zero W
- 1 GB de RAM
- Tela 7" com resolução 800x480

## Instalação

### Dependências Ubuntu/Debian
```bash
sudo apt-get install -y \
    qt5-qmake qtbase5-dev \
    libmupdf-dev libsqlite3-dev \
    cmake g++ make pkg-config
```

### Compilação
```bash
mkdir build && cd build
cmake ..
cmake --build . -j$(nproc)
./bin/brazareader
```

Veja [BUILD.md](BUILD.md) para instruções detalhadas de compilação cruzada.

## Estrutura de Diretórios

```
/library/                      # Armazenamento de PDFs
~/.local/share/brazareader/
├── progress.db               # Histórico de leitura
├── favorites.db              # Livros favoritos
└── settings.ini              # Configurações
```

## Navegação

### Biblioteca
- Visualização em grade com miniaturas
- Busca e organização de livros
- Botão de favoritos (☆) para cada livro

### Leitor
- Scroll vertical contínuo
- Controle de tamanho de fonte (Aa)
- Visualização de página atual

### Configurações
- Cor do menu
- Modo noturno
- Restauração de padrões

### Menu Lateral
- Toque no ícone ☰ para abrir
- Biblioteca
- Favoritos
- Configurações
- Sobre

## Controles

| Ação | Descrição |
|------|-----------|
| Toque em ☰ | Abre/fecha menu lateral |
| Scroll vertical | Navega entre páginas |
| Aa | Alterna tamanho de fonte |
| ← | Volta para biblioteca |
| ⭐ | Adiciona/remove favorito |

## Desenvolvimento

### Arquitetura em Camadas

**UI Layer (Qt5 Widgets)**
- MainWindow: Janela principal e navegação
- LibraryScreen: Exibição de livros
- ReaderScreen: Tela de leitura
- SettingsScreen: Configurações
- SidebarMenu: Menu de navegação

**Reader Engine**
- PDFLoader: Carregamento de arquivos
- PDFRenderer: Renderização via MuPDF
- PDFCache: Cache de páginas em memória

**Storage Layer**
- LibraryManager: Gerenciamento de biblioteca
- ProgressManager: Histórico de leitura
- FavoriteManager: Sistema de favoritos
- SettingsManager: Configurações de usuário

**System Interface**
- TouchHandler: Manipulação de entrada tátil

## Desempenho

### Métricas Alvo
- Abertura de PDF: < 2s
- Renderização de página: < 300ms
- Uso de memória: < 100MB
- Cache: 3-5 páginas máximo

### Otimizações
- Renderização assíncrona
- Cache LRU para páginas
- Liberação imediata de pixmaps
- Batching de eventos de toque

## Segurança

- Sem acesso externo - funciona completamente offline
- Sem funções online integradas
- Sem transmissão de dados
- Leitura apenas de arquivos locais

## Resolução de Problemas

### Problema: PDF não renderiza
- Verifique se o arquivo está em `/library/`
- Confirme que o PDF é válido
- Verifique permissões de leitura

### Problema: Lentidão
- Reduza número de páginas em cache
- Feche outros aplicativos
- Verifique espaço em disco

### Problema: Touchscreen não funciona
- Configure drivers de touchscreen
- Verifique calibração do dispositivo
- Teste com `evtest`

## Licença

BrazaReader - Leitor PDF otimizado para dispositivos limitados

## Contribuições

Contribuições são bem-vindas. Envie pull requests para melhorias, correções de bugs ou novas funcionalidades.

## Contato

Para questões, sugestões ou relatórios de bugs, abra uma issue no repositório.

---

**BrazaReader v1.0.0** - Leitura offline otimizada para Raspberry Pi
