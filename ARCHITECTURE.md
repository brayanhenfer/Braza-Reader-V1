# BrazaReader - Arquitetura Detalhada

## Visão Geral

BrazaReader é estruturado em **4 camadas principais**:

```
┌─────────────────────────────────────┐
│      UI Layer (Qt5 Widgets)         │
│  (MainWindow, Screens, Sidebar)     │
├─────────────────────────────────────┤
│      Reader Engine (MuPDF)          │
│  (PDFLoader, Renderer, Cache)       │
├─────────────────────────────────────┤
│     Storage Layer (SQLite)          │
│  (Progress, Favorites, Settings)    │
├─────────────────────────────────────┤
│    System Interface (Touch)         │
│  (TouchHandler, Input Processing)   │
└─────────────────────────────────────┘
```

## Camada 1: UI Layer

### Componentes

#### MainWindow
- **Responsabilidade**: Gerenciamento da janela principal e navegação entre telas
- **Arquivo**: `src/ui/mainwindow.h/cpp`
- **Funções principais**:
  - Exibir telas (Library, Reader, Settings, About)
  - Gerenciar menu lateral animado
  - Coordenar eventos de navegação
  - Aplicar temas globais

#### LibraryScreen
- **Responsabilidade**: Exibição e gerenciamento da biblioteca de PDFs
- **Arquivo**: `src/ui/libraryscreen.h/cpp`
- **Funções principais**:
  - Varrer diretório `/library`
  - Exibir livros em grade com miniaturas
  - Gerenciar favoritos
  - Abrir livro para leitura

#### ReaderScreen
- **Responsabilidade**: Renderização e navegação de PDFs
- **Arquivo**: `src/ui/readerscreen.h/cpp`
- **Funções principais**:
  - Carregar e renderizar páginas
  - Manipular scroll vertical
  - Ajustar tamanho de fonte
  - Salvar progresso

#### SettingsScreen
- **Responsabilidade**: Interface de configurações
- **Arquivo**: `src/ui/settingsscreen.h/cpp`
- **Funções principais**:
  - Personalizador de cores
  - Toggle de modo noturno
  - Restauração de padrões

#### AboutScreen
- **Responsabilidade**: Informações da aplicação
- **Arquivo**: `src/ui/aboutscreen.h/cpp`
- **Funções principais**:
  - Exibir versão e informações

#### SidebarMenu
- **Responsabilidade**: Menu de navegação lateral
- **Arquivo**: `src/ui/sidebarmenu.h/cpp`
- **Funções principais**:
  - Navegação entre seções
  - Tema visual personalizado

## Camada 2: Reader Engine

### Componentes

#### PDFLoader
- **Responsabilidade**: Carregamento de documentos PDF via MuPDF
- **Arquivo**: `src/engine/pdfloader.h/cpp`
- **Funções principais**:
  ```cpp
  bool openDocument(const QString& filePath);
  void closeDocument();
  int getPageCount() const;
  fz_document* getDocument() const;
  fz_context* getContext() const;
  ```
- **Ciclo de vida**:
  1. Cria contexto MuPDF
  2. Abre documento
  3. Fornece acesso a páginas
  4. Limpa recursos ao destruir

#### PDFRenderer
- **Responsabilidade**: Renderização de páginas em imagens QPixmap
- **Arquivo**: `src/engine/pdfrenderer.h/cpp`
- **Funções principais**:
  ```cpp
  bool openPDF(const QString& filePath);
  void closePDF();
  QPixmap renderPage(int pageNumber, int width, int height);
  int getPageCount() const;
  ```
- **Processo de renderização**:
  1. Carrega página do documento
  2. Calcula matriz de transformação
  3. Renderiza para pixmap RGB
  4. Converte para QPixmap
  5. Libera recursos

#### PDFCache
- **Responsabilidade**: Cache LRU de páginas renderizadas
- **Arquivo**: `src/engine/pdfcache.h/cpp`
- **Funções principais**:
  ```cpp
  void cachePageImage(int pageNumber, const QPixmap& image);
  QPixmap* getCachedPage(int pageNumber);
  void clearCache();
  bool hasPage(int pageNumber) const;
  void setMaxCacheSize(int maxPages);
  ```
- **Otimizações**:
  - Máximo 5 páginas em memória
  - Evicts página mais antiga quando necessário
  - Reduz renderizações redundantes

## Camada 3: Storage Layer

### Componentes

#### LibraryManager
- **Responsabilidade**: Gerenciamento do diretório de biblioteca
- **Arquivo**: `src/storage/librarymanager.h/cpp`
- **Funções principais**:
  ```cpp
  QStringList scanLibrary();
  bool addBook(const QString& filePath);
  bool removeBook(const QString& filePath);
  ```
- **Características**:
  - Varre `/library` por arquivos `.pdf`
  - Cria diretório se não existir
  - Retorna lista de caminhos absolutos

#### ProgressManager
- **Responsabilidade**: Persistência de histórico de leitura
- **Arquivo**: `src/storage/progressmanager.h/cpp`
- **Banco de dados**:
  ```sql
  CREATE TABLE book_progress (
    id INTEGER PRIMARY KEY,
    title TEXT UNIQUE,
    last_page INTEGER,
    last_read TIMESTAMP
  );
  ```
- **Funções principais**:
  ```cpp
  int getLastPage(const QString& bookTitle) const;
  void saveProgress(const QString& bookTitle, int pageNumber);
  void clearProgress(const QString& bookTitle);
  ```

#### FavoriteManager
- **Responsabilidade**: Gerenciamento de livros favoritos
- **Arquivo**: `src/storage/favoritemanager.h/cpp`
- **Banco de dados**:
  ```sql
  CREATE TABLE favorites (
    id INTEGER PRIMARY KEY,
    title TEXT UNIQUE,
    added_date TIMESTAMP
  );
  ```
- **Funções principais**:
  ```cpp
  bool isFavorite(const QString& bookTitle) const;
  void toggleFavorite(const QString& bookTitle);
  QStringList getAllFavorites() const;
  ```

#### SettingsManager
- **Responsabilidade**: Persistência de configurações
- **Arquivo**: `src/storage/settingsmanager.h/cpp`
- **Arquivo de configuração**: `~/.local/share/brazareader/settings.ini`
- **Funções principais**:
  ```cpp
  QColor getMenuColor() const;
  void setMenuColor(const QColor& color);
  bool getNightMode() const;
  void setNightMode(bool enabled);
  int getFontSize() const;
  void setFontSize(int size);
  void resetToDefaults();
  ```

## Camada 4: System Interface

### Componentes

#### TouchHandler
- **Responsabilidade**: Processamento de eventos de entrada tátil
- **Arquivo**: `src/system/touchhandler.h/cpp`
- **Sinais**:
  ```cpp
  void swipeUp();
  void swipeDown();
  void swipeLeft();
  void swipeRight();
  void tap(QPoint position);
  ```
- **Detecção de gestos**:
  - Limiar de 50 pixels para swipe
  - Diferenciação entre vertical e horizontal
  - Eventos de tap para cliques

## Fluxo de Dados

### Abertura de Livro
```
LibraryScreen → MainWindow → ReaderScreen
  ↓
ReaderScreen::openBook(filePath)
  ↓
PDFRenderer::openPDF(filePath)
  ↓
PDFLoader::openDocument()
  ↓
MuPDF Context/Document inicializado
  ↓
renderCurrentPage()
  ↓
PDFRenderer::renderPage()
  ↓
PDFCache::cachePageImage()
  ↓
QPixmap exibido na tela
```

### Navegação de Páginas
```
ReaderScreen::wheelEvent() ou touchEvent()
  ↓
currentPage++ / currentPage--
  ↓
renderCurrentPage()
  ↓
PDFCache::hasPage() ?
  ├─ Sim: use cache
  └─ Não: PDFRenderer::renderPage()
  ↓
ProgressManager::saveProgress()
  ↓
SQLite atualizado
```

### Favoritos
```
LibraryScreen::onFavoriteToggled()
  ↓
FavoriteManager::toggleFavorite()
  ↓
SQLite INSERT/DELETE
  ↓
LibraryScreen::loadLibrary() (refresh)
  ↓
Tela atualizada com novo estado
```

## Gerenciamento de Memória

### Estratégias

1. **Stack allocation**: Objetos pequenos como coordenadas
2. **Smart pointers**: `std::unique_ptr` para propriedade exclusiva
3. **Cache LRU**: Máximo 5 páginas em RAM
4. **Pixel data lifecycle**:
   - Alocado em renderização
   - Convertido para QPixmap imediatamente
   - Liberado após conversão

### Limites de Recursos

| Recurso | Limite | Objetivo |
|---------|--------|----------|
| Cache de páginas | 5 | Equilíbrio memória/performance |
| Tamanho de imagem | ~2MB por página | Típico para 800x600 RGB |
| RAM total esperado | 50-100MB | Compatibilidade Pi Zero |
| Banco de dados | Ilimitado | Um arquivo por tipo |

## Performance

### Benchmarks Esperados

| Operação | Tempo |
|----------|-------|
| Abrir PDF | < 2s |
| Renderizar página | < 300ms |
| Cache hit | < 10ms |
| Scroll página | < 100ms |
| Salvar progresso | < 50ms |

### Otimizações Implementadas

1. **Renderização assíncrona**: Pages renderizadas on-demand
2. **Cache eficiente**: LRU previne thrashing
3. **Conversão otimizada**: Direct MuPDF → QPixmap
4. **Eventos batched**: Toque processado em lotes
5. **Lazy loading**: Biblioteca carregada sob demanda

## Dependências

### Externas
- **Qt5**: Core, Gui, Widgets
- **MuPDF**: Renderização PDF
- **SQLite3**: Persistência de dados

### Internas
Nenhuma dependência circular entre camadas

## Extensibilidade

### Pontos de Extensão

1. **Novos formatos**: Implementar `Reader` interface
2. **Novos storage backends**: Herdar de `Manager`
3. **Temas**: Estender `SettingsManager`
4. **Gestos**: Adicionar em `TouchHandler`

### Futuros Recursos

- [ ] Anotações por página
- [ ] Coleções personalizadas
- [ ] Busca textual
- [ ] Bookmarks
- [ ] Sincronização local
