# Documentação do Sistema de Templates e Ferramentas MCP

## 1. Funcionamento do Sistema de Templates

O projeto utiliza um sistema de templates customizado, leve e extremamente flexível, implementado em Python no arquivo `tools/template.py`. O sistema baseia-se em substituição de strings e processamento de listas através de expressões regulares.

### 1.1. Core Engine (`template.py`)

A classe `template` oferece as seguintes funcionalidades:

*   **Importação Recursiva (`{{import name}}`):** Permite a composição de templates. Um template pode incluir outro (ex: `{{import file-header}}`), facilitando a manutenção de cabeçalhos e estruturas comuns. A resolução é recursiva, permitindo múltiplos níveis de aninhamento.
*   **Substituição Simples (`{{key}}`):** Substitui marcadores pelo valor correspondente fornecido no dicionário de dados (`data`).
*   **Processamento de Listas:**
    *   `{{list_open key}}`: Início do bloco de lista.
    *   `{{list_item key}}`: Marcador que será substituído por cada item da lista.
    *   `{{list_close key}}`: Fim do bloco de lista.
    *   O motor utiliza `re.DOTALL`, permitindo que o conteúdo entre `list_open` e `list_close` contenha múltiplas linhas e formatação complexa.

### 1.2. Integração com MCP (`project-tools-mcp`)

O servidor MCP expõe ferramentas que utilizam o motor de templates para automação de boilerplate e gerenciamento do ciclo de vida do projeto:

*   **`create_class`:** Gera arquivos `.hpp` (em `include/`) e `.cpp` (em `source/`) para uma nova classe C++. Automatiza metadados (autor, data), header guards únicos e suporte a namespaces.
*   **`create_test`:** Gera arquivos de teste C++ baseados no template `test-cpp.txt`, suportando tanto testes Adhoc quanto estruturados.
*   **`compile`:** Interface para o script `tree-builder.py`, permitindo a compilação do projeto diretamente via MCP.

### 1.3. Templates Disponíveis (`docs/templates/`)

*   `file-header.txt`: Padronização de licença GPL e informações de autoria/criação.
*   `class-hpp.txt`: Estrutura de classe C++ com `pragma once`, `header guards`, namespaces e suporte a `sak/sak.hpp`.
*   `class-cpp.txt`: Estrutura de implementação C++.
*   `test-cpp.txt`: Estrutura base para testes, incluindo boilerplate para processamento de argumentos e `println`.

---

## 2. Automação de Testes (`create_test`)

O sistema de testes é automatizado pela ferramenta `create_test`, que organiza os arquivos em duas categorias:

### 2.1. Testes Adhoc
*   **Caminho:** `tests/adhoc/NNNN_<hierarchy>/NNNN_<hierarchy>.cpp`.
*   **Lógica:** O sistema identifica o maior contador numérico (`NNNN`) existente em `tests/adhoc/` e incrementa-o para o novo teste.
*   **Uso:** Experimentos rápidos e verificações pontuais.

### 2.2. Testes Estruturados
*   **Caminho:** `tests/<path>/test_<path_com_sublinhado>_<nome>.cpp`.
*   **Lógica:** Segue a hierarquia do código fonte (ex: `sak/geometry/point` vira `tests/sak/geometry/test_sak_geometry_point.cpp`).
*   **Uso:** Testes de unidade e integração vinculados à arquitetura do projeto.

---

## 3. Fluxo de Geração de Código

Toda geração de código via MCP segue o seguinte fluxo:

1.  **Coleta de Metadados:** Obtém `user.name` e `user.email` via `git config`.
2.  **Preparação de Dados:** Monta o dicionário `data` com as informações específicas da ferramenta (hierarquia, listas de includes, etc).
3.  **Processamento:** O motor de templates resolve importações e substitui as chaves.
4.  **Escrita:** Os arquivos são criados nas pastas correspondentes, criando a estrutura de diretórios necessária automaticamente.