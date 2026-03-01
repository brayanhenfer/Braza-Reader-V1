# BrazaReader - Exemplos de Uso

## Exemplo 1: Compilação Básica

```bash
# Instalar dependências
sudo apt-get install cmake qt5-qmake qtbase5-dev libmupdf-dev libsqlite3-dev

# Clonar projeto
git clone https://github.com/usuario/brazareader.git
cd brazareader

# Compilar
chmod +x build.sh
./build.sh

# Criar diretórios
mkdir -p /library
mkdir -p ~/.local/share/brazareader

# Executar
cd build
./bin/brazareader
```

## Exemplo 2: Adicionar PDFs à Biblioteca

```bash
# Copiar PDFs para biblioteca
cp ~/Documentos/*.pdf /library/

# Verificar
ls -la /library/

# Executar BrazaReader e os PDFs aparecerão na biblioteca
./bin/brazareader
```

## Exemplo 3: Usar em Raspberry Pi Zero

### Preparação no Desktop
```bash
# Cross-compilar
export SYSROOT=/opt/rpi-sysroot
./cross-compile-rpi.sh

# Resultado: build-rpi/bin/brazareader
```

### Deployment no Pi
```bash
# No Raspberry Pi
mkdir -p /library
mkdir -p ~/.local/share/brazareader

# Copiar binário
scp build-rpi/bin/brazareader pi@raspberrypi:/usr/local/bin/

# SSH e executar
ssh pi@raspberrypi
/usr/local/bin/brazareader &
```

## Exemplo 4: Copiar PDFs para Pi via Network

```bash
# No desktop - criar script de sincronização
cat > sync-pdfs.sh << 'EOF'
#!/bin/bash
PDFS_DIR="/library"
RPI_IP="192.168.1.100"
RPI_USER="pi"

scp $PDFS_DIR/*.pdf $RPI_USER@$RPI_IP:/library/
echo "PDFs sincronizados!"
EOF

chmod +x sync-pdfs.sh
./sync-pdfs.sh
```

## Exemplo 5: Personalizar Configurações

### Via Interface (Graphical)
1. Executar BrazaReader
2. Toque no ☰ (menu)
3. Configurações
4. Mudar cor do menu, ativar modo noturno

### Via Arquivo (Manual)
```bash
# Editar configurações manualmente
nano ~/.local/share/brazareader/settings.ini

# Conteúdo:
[ui]
menu_color=#1e5032
night_mode=true
font_size=16
```

## Exemplo 6: Verificar Histórico de Leitura

```bash
# Banco de dados SQLite
sqlite3 ~/.local/share/brazareader/progress.db

# Ver estrutura
.schema book_progress

# Ver histórico de um livro
SELECT * FROM book_progress WHERE title LIKE '%livro%';

# Limpar histórico de um livro
DELETE FROM book_progress WHERE title = 'meu_livro';

# Ver todas as páginas lidas
SELECT title, last_page, last_read FROM book_progress;
```

## Exemplo 7: Gerenciar Favoritos

```bash
# Ver database de favoritos
sqlite3 ~/.local/share/brazareader/favorites.db

# Ver estrutura
.schema favorites

# Listar favoritos
SELECT title FROM favorites ORDER BY added_date DESC;

# Adicionar favorito manualmente (se necessário)
INSERT INTO favorites (title) VALUES ('meu_livro');

# Remover favorito manualmente
DELETE FROM favorites WHERE title = 'meu_livro';
```

## Exemplo 8: Debug Mode Compilation

```bash
# Compilar com símbolos de debug
cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .

# Executar com gdb
gdb ./bin/brazareader

# No gdb:
(gdb) run
(gdb) break main
(gdb) next
```

## Exemplo 9: Limpar Cache

```bash
# Limpar base de dados de progresso
rm ~/.local/share/brazareader/progress.db

# Limpar favoritos
rm ~/.local/share/brazareader/favorites.db

# Limpar configurações
rm ~/.local/share/brazareader/settings.ini

# Próxima execução recriará com padrões
./bin/brazareader
```

## Exemplo 10: Analisar Performance

```bash
# Verificar tempo de renderização (em readerscreen.cpp)
auto start = std::chrono::high_resolution_clock::now();
QPixmap pageImage = renderer->renderPage(currentPage, 800, 600);
auto end = std::chrono::high_resolution_clock::now();
auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
qDebug() << "Render time:" << duration.count() << "ms";

# Monitorar uso de memória em tempo real
watch -n 0.1 'ps aux | grep brazareader'

# Ou usar valgrind
valgrind --tool=massif ./bin/brazareader
ms_print massif.out.* | head -50
```

## Exemplo 11: Script de Teste Automático

```bash
#!/bin/bash

echo "=== BrazaReader Test Suite ==="

# Test 1: Verify dependencies
echo "Test 1: Checking dependencies..."
pkg-config --exists Qt5Core && echo "✓ Qt5 OK" || echo "✗ Qt5 Missing"
pkg-config --exists mupdf && echo "✓ MuPDF OK" || echo "✗ MuPDF Missing"
pkg-config --exists sqlite3 && echo "✓ SQLite3 OK" || echo "✗ SQLite3 Missing"

# Test 2: Build
echo "Test 2: Building..."
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . || exit 1
echo "✓ Build successful"

# Test 3: Create required directories
echo "Test 3: Setting up directories..."
mkdir -p /library ~/.local/share/brazareader
echo "✓ Directories ready"

# Test 4: Check binary
echo "Test 4: Checking binary..."
file bin/brazareader | grep -q ELF && echo "✓ Binary is ELF" || echo "✗ Invalid binary"
bin/brazareader --version 2>/dev/null && echo "✓ Binary executable" || echo "✗ Binary not executable"

# Test 5: Database initialization
echo "Test 5: Database test..."
sqlite3 ~/.local/share/brazareader/progress.db ".schema" && echo "✓ Progress DB OK" || echo "✗ Progress DB Error"
sqlite3 ~/.local/share/brazareader/favorites.db ".schema" && echo "✓ Favorites DB OK" || echo "✗ Favorites DB Error"

echo "=== Tests Complete ==="
```

## Exemplo 12: Cross-Compilation Toolchain Setup

```bash
# Instalar toolchain ARM
sudo apt-get install gcc-arm-linux-gnueabihf g++-arm-linux-gnueabihf

# Preparar sysroot (exemplo com Buildroot)
cd /tmp
git clone https://git.buildroot.net/buildroot
cd buildroot

# Configurar para Raspberry Pi Zero
make raspberrypi_defconfig BR2_PRIMARY_SITE=https://buildroot.org/downloads
make

# Sysroot localizado em: output/host/arm-buildroot-linux-gnueabihf/sysroot

# Usar para cross-compile
export SYSROOT=/tmp/buildroot/output/host/arm-buildroot-linux-gnueabihf/sysroot
../brazareader/cross-compile-rpi.sh
```

## Exemplo 13: CI/CD Pipeline (GitHub Actions)

```yaml
# .github/workflows/build.yml
name: Build BrazaReader

on: [push, pull_request]

jobs:
  build:
    runs-on: ubuntu-latest

    steps:
    - uses: actions/checkout@v2

    - name: Install dependencies
      run: |
        sudo apt-get update
        sudo apt-get install -y cmake qt5-qmake qtbase5-dev libmupdf-dev libsqlite3-dev

    - name: Build
      run: |
        mkdir build && cd build
        cmake ..
        cmake --build .

    - name: Test
      run: |
        cd build
        file bin/brazareader
```

## Exemplo 14: Containerização (Docker)

```dockerfile
# Dockerfile
FROM ubuntu:20.04

RUN apt-get update && apt-get install -y \
    cmake g++ make pkg-config \
    qt5-qmake qtbase5-dev \
    libmupdf-dev libsqlite3-dev

WORKDIR /app
COPY . .

RUN ./build.sh

CMD ["./build/bin/brazareader"]
```

## Exemplo 15: Performance Tuning

```cpp
// src/app.cpp - Otimizações opcionais
// Reduzir cache para hardware muito limitado
PDFCache cache(3);  // em vez de 5

// Renderização com qualidade reduzida para performance
QPixmap pageImage = renderer->renderPage(currentPage, 400, 300);

// Usar imagens comprimidas
pageImage.save("cache.jpg", "JPEG", 85);
```

---

**Mais exemplos e casos de uso em discussões do GitHub!**
