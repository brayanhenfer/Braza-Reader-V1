# BrazaReader - Índice Completo

## 📋 Documentação

| Arquivo | Descrição |
|---------|-----------|
| **README.md** | Visão geral completa do projeto, features, instalação |
| **BUILD.md** | Instruções detalhadas de compilação e deployment |
| **ARCHITECTURE.md** | Arquitetura técnica, design de camadas, fluxos de dados |
| **QUICKSTART.md** | Guia de 5 minutos para começar |
| **PROJECT_SUMMARY.md** | Status do projeto, componentes entregues, checklist |
| **EXAMPLES.md** | Exemplos práticos de uso e desenvolvimento |
| **INDEX.md** | Este arquivo - mapa de todos os recursos |

## 🏗️ Configuração de Build

| Arquivo | Descrição |
|---------|-----------|
| **CMakeLists.txt** | Configuração CMake principal para compilação |
| **build.sh** | Script automatizado para compilação desktop |
| **cross-compile-rpi.sh** | Script para cross-compilação para Raspberry Pi |
| **.gitignore** | Padrões Git para ignorar arquivos |

## 📁 Código Fonte - Estrutura

```
src/
├── main.cpp                    → Ponto de entrada da aplicação
├── app.h / app.cpp             → Classe principal BrazaReader
│
├── ui/                         → Camada de Interface de Usuário
│   ├── mainwindow.h/cpp        → Janela principal e navegação
│   ├── libraryscreen.h/cpp     → Tela de biblioteca
│   ├── readerscreen.h/cpp      → Tela de leitura
│   ├── settingsscreen.h/cpp    → Tela de configurações
│   ├── aboutscreen.h/cpp       → Tela sobre
│   └── sidebarmenu.h/cpp       → Menu lateral
│
├── engine/                     → Motor de Leitura PDF
│   ├── pdfloader.h/cpp         → Carregador de PDFs (MuPDF)
│   ├── pdfrenderer.h/cpp       → Renderizador de páginas
│   └── pdfcache.h/cpp          → Cache LRU de páginas
│
├── storage/                    → Camada de Armazenamento
│   ├── librarymanager.h/cpp    → Gerenciador de biblioteca
│   ├── progressmanager.h/cpp   → Histórico de leitura (SQLite)
│   ├── favoritemanager.h/cpp   → Sistema de favoritos (SQLite)
│   └── settingsmanager.h/cpp   → Configurações (INI)
│
└── system/                     → Interface de Sistema
    └── touchhandler.h/cpp      → Manipulador de entrada tátil

resources/
└── resources.qrc               → Arquivo de recursos Qt
```

## 📊 Estatísticas do Projeto

### Arquivos de Código
- **Total**: 31 arquivos (.h e .cpp)
- **Linhas de código**: ~3.500
- **Camadas**: 4 (UI, Engine, Storage, System)
- **Componentes**: 13

### Documentação
- **Arquivos**: 7 (.md)
- **Linhas**: ~2.000+

### Dependências Externas
- Qt5 (Core, Gui, Widgets)
- MuPDF
- SQLite3

## 🎯 Componentes por Responsabilidade

### UI (6 componentes)
1. **MainWindow** - Orquestração e navegação
2. **LibraryScreen** - Exibição de biblioteca
3. **ReaderScreen** - Leitura de PDFs
4. **SettingsScreen** - Configurações
5. **AboutScreen** - Informações
6. **SidebarMenu** - Navegação lateral

### Engine (3 componentes)
1. **PDFLoader** - Carregamento MuPDF
2. **PDFRenderer** - Renderização
3. **PDFCache** - Cache LRU

### Storage (4 componentes)
1. **LibraryManager** - Varredura de biblioteca
2. **ProgressManager** - Histórico SQLite
3. **FavoriteManager** - Favoritos SQLite
4. **SettingsManager** - Config INI

### System (1 componente)
1. **TouchHandler** - Entrada tátil

## 🔄 Fluxos Principais

### Fluxo 1: Abrir Livro
```
LibraryScreen (click)
→ MainWindow (openBook signal)
→ ReaderScreen (openBook)
→ PDFRenderer (openPDF)
→ PDFLoader (openDocument)
→ Renderização (renderPage)
→ Exibição na tela
```

### Fluxo 2: Navegar Páginas
```
ReaderScreen (wheelEvent/touchEvent)
→ currentPage++
→ PDFCache (hasPage?)
├─ Sim: use cache
└─ Não: PDFRenderer (renderPage)
→ ProgressManager (saveProgress)
→ SQLite atualizado
```

### Fluxo 3: Favoritos
```
LibraryScreen (star click)
→ FavoriteManager (toggleFavorite)
→ SQLite (INSERT/DELETE)
→ LibraryScreen (refresh)
→ Tela atualizada
```

## 📦 Estrutura de Runtime

### Diretórios Necessários
```
/library/                                 # PDFs
~/.local/share/brazareader/              # Dados da app
├── progress.db                           # SQLite - histórico
├── favorites.db                          # SQLite - favoritos
└── settings.ini                          # Configurações
```

## 🔧 Configurações por Arquivo

### settings.ini (~/.local/share/brazareader/)
```ini
[ui]
menu_color=#1e6432
night_mode=false
font_size=12
```

### progress.db (SQLite)
```sql
CREATE TABLE book_progress (
  id INTEGER PRIMARY KEY,
  title TEXT UNIQUE,
  last_page INTEGER,
  last_read TIMESTAMP
);
```

### favorites.db (SQLite)
```sql
CREATE TABLE favorites (
  id INTEGER PRIMARY KEY,
  title TEXT UNIQUE,
  added_date TIMESTAMP
);
```

## 🚀 Como Usar Este Índice

### Para Compilar
1. Leia **QUICKSTART.md**
2. Execute `./build.sh`

### Para Entender Arquitetura
1. Leia **ARCHITECTURE.md**
2. Estude diagramas em camadas
3. Trace fluxos de dados

### Para Desenvolver Novo Recurso
1. Consulte **ARCHITECTURE.md** - Entenda a camada
2. Veja **EXAMPLES.md** - Exemplos práticos
3. Encontre arquivo em **Código Fonte - Estrutura**
4. Implemente seguindo padrões existentes

### Para Cross-Compilar
1. Leia **BUILD.md** seção "Cross-Compilation"
2. Execute `cross-compile-rpi.sh`
3. Consulte **EXAMPLES.md** exemplo 12

### Para Deploy
1. **BUILD.md** - Instruções deployment
2. **EXAMPLES.md** exemplo 3 - Pi setup
3. **EXAMPLES.md** exemplo 4 - Sincronização

## 📈 Roadmap de Desenvolvimento

### Versão 1.0 (Atual) ✅
- [x] Leitura básica de PDF
- [x] Biblioteca e navegação
- [x] Favoritos
- [x] Histórico de leitura
- [x] Configurações

### Versão 1.1 (Recomendado)
- [ ] Miniaturas de PDF reais (não placeholder)
- [ ] Buscas por título
- [ ] Coleções organizadas

### Versão 2.0 (Futuro)
- [ ] Anotações por página
- [ ] Bookmarks
- [ ] Zoom de página
- [ ] Busca textual

## ⚠️ Notas Importantes

### Performance
- Cache máximo: 5 páginas
- Alvo render: <300ms
- Memória esperada: 50-100MB

### Compatibilidade
- Arquitetura: ARMv6 hard-float (RPi Zero)
- Qt5 mínimo: 5.12
- CMake mínimo: 3.16
- C++17 requerido

### Segurança
- Sem conexão internet
- Sem transmissão dados
- Offline completo

## 📞 Suporte Rápido

### "Onde está X?"
- **Interface para PDF**: `src/ui/readerscreen.h/cpp`
- **Carregamento PDF**: `src/engine/pdfloader.h/cpp`
- **Salvamento progresso**: `src/storage/progressmanager.h/cpp`
- **Configurações**: `src/storage/settingsmanager.h/cpp`

### "Como adiciono Y?"
1. Procure em **ARCHITECTURE.md**
2. Veja **EXAMPLES.md**
3. Copie padrão de componente similar
4. Siga convenções de código

### "Por que Z não funciona?"
1. Verifique **BUILD.md** - Dependências
2. Consulte **EXAMPLES.md** - Troubleshooting
3. Verifique **QUICKSTART.md** - Setup

---

**Última atualização**: 2026-03-01
**Versão**: 1.0.0
**Status**: ✅ Completo e Pronto para Produção
