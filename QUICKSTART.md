# BrazaReader - Guia Rápido

## Começar em 5 Minutos

### 1. Instalar Dependências
```bash
sudo apt-get update
sudo apt-get install -y cmake qt5-qmake qtbase5-dev libmupdf-dev libsqlite3-dev g++ make pkg-config
```

### 2. Clonar e Navegar
```bash
git clone https://github.com/seu-usuario/brazareader.git
cd brazareader
```

### 3. Compilar
```bash
chmod +x build.sh
./build.sh
```

### 4. Criar Diretórios
```bash
mkdir -p /library
mkdir -p ~/.local/share/brazareader
```

### 5. Executar
```bash
cd build
./bin/brazareader
```

## Estrutura de Pastas Esperadas

```
/
├── library/                    # Coloque seus PDFs aqui
│   ├── livro1.pdf
│   ├── livro2.pdf
│   └── ...
└── ~/.local/share/brazareader/
    ├── progress.db            # Histórico (criado automaticamente)
    ├── favorites.db           # Favoritos (criado automaticamente)
    └── settings.ini           # Configurações (criado automaticamente)
```

## Controles

### Na Biblioteca
- **☰ (Hamburguer)**: Abre menu lateral
- **⭐ (Estrela)**: Marca como favorito
- **✎ (Lápis)**: Editar livro
- **Scroll**: Navegar pela lista

### Na Leitura
- **← (Seta)**: Volta para biblioteca
- **Scroll vertical**: Próxima/página anterior
- **Aa**: Alterna tamanho de fonte
- **Toque no topo**: Esconde/mostra barra

### Menu Lateral
- **Biblioteca**: Todos os livros
- **Favoritos**: Apenas favoritos
- **Configurações**: Personalize cores e tema
- **Sobre**: Informações da app

## Configurações Básicas

As configurações são salvas em `~/.local/share/brazareader/settings.ini`

```ini
[ui]
menu_color=#1e6432
night_mode=false
font_size=12
```

Edite diretamente ou use a interface de configurações.

## Resolução de Problemas

### "Erro: biblioteca Qt não encontrada"
```bash
sudo apt-get install qtbase5-dev
```

### "Erro: MuPDF não encontrado"
```bash
sudo apt-get install libmupdf-dev libmupdf0
```

### "PDF não aparece na biblioteca"
- Copie PDFs para `/library/`
- Reinicie a aplicação
- Verifique permissões: `ls -la /library/`

### "Aplicação muito lenta"
- Feche outros aplicativos
- Verifique espaço em disco
- Reduz número de páginas em cache (código)

## Compilação Cruzada para Raspberry Pi

```bash
# Configurar sysroot
export SYSROOT=/caminho/para/rpi/sysroot

# Cross-compilar
chmod +x cross-compile-rpi.sh
./cross-compile-rpi.sh

# Transferir para RPi
scp build-rpi/bin/brazareader pi@raspberrypi:/usr/local/bin/

# SSH para RPi e executar
ssh pi@raspberrypi
brazareader
```

## Modo Desenvolvimento

### Editar Código
```bash
# Todos os fontes estão em src/
src/
├── ui/          # Interface
├── engine/      # PDF rendering
├── storage/     # Banco de dados
└── system/      # Entrada/eventos
```

### Recompile Após Mudanças
```bash
cd build
cmake --build .
./bin/brazareader
```

### Debug
```bash
cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .
gdb ./bin/brazareader
```

## Recursos

- **README.md** - Visão geral completa
- **BUILD.md** - Instruções detalhadas de compilação
- **ARCHITECTURE.md** - Arquitetura técnica
- **CMakeLists.txt** - Configuração de build

## Próximos Passos

1. ✅ Compile e teste na sua máquina
2. 📱 Teste em Raspberry Pi
3. 🎨 Personalizar cores e tema
4. 📚 Adicionar seus PDFs
5. 🔄 Compartilhe feedback!

## Suporte

- Issues: Abra no GitHub
- Discussões: Seção de Discussion
- Contribuições: Pull Requests bem-vindas

---

**Dúvidas? Leia a [documentação completa](BUILD.md) ou abra uma issue!**
