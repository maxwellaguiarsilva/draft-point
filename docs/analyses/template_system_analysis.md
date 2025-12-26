# Análise do Sistema de Templates e Proposta de Extensão

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

O servidor MCP expõe ferramentas que utilizam o motor de templates para automação de boilerplate. A ferramenta `create_class` é o principal exemplo:

1.  **Coleta de Metadados:** Obtém informações do usuário via `git config` (nome, e-mail), data/hora atual e gera `header_guards` únicos usando `time.time_ns()`.
2.  **Preparação de Dados:** Monta um dicionário `data` com nomes de classes, namespaces e listas de includes/usings.
3.  **Geração de Arquivos:** Invoca o template `class-hpp` para o cabeçalho e `class-cpp` para a implementação, salvando-os nas pastas `include/` e `source/` respectivamente, respeitando a hierarquia informada.

### 1.3. Templates Existentes

*   `file-header.txt`: Padronização de licença GPL e informações de autoria/criação.
*   `class-hpp.txt`: Estrutura de classe C++ com `pragma once`, `header guards`, namespaces e suporte a `sak/sak.hpp`.
*   `class-cpp.txt`: Estrutura de implementação C++.

---

## 2. Análise do Sistema de Testes

O projeto organiza testes em duas categorias principais:

### 2.1. Testes Adhoc (`tests/adhoc/`)
*   **Estrutura:** `tests/adhoc/<contador>_<nome_do_teste>/<contador>_<nome_do_teste>.cpp`.
*   **Contador:** Incremental (ex: `0001`, `0002`, `0003`).
*   **Propósito:** Experimentos rápidos, verificações pontuais ou demonstrações de funcionalidades sem vínculo estrito com a hierarquia das classes do sistema.

### 2.2. Testes Estruturados (`tests/<hierarchy>/`)
*   **Estrutura:** `tests/<caminho_da_classe>/test_<caminho_com_sublinhado>.cpp`.
*   **Exemplo:** A classe em `sak/geometry/coordinate.hpp` tem seu teste em `tests/sak/geometry/test_sak_geometry_coordinate.cpp`.
*   **Propósito:** Testes de unidade e integração que seguem a arquitetura do código fonte.

---

## 3. Proposta: Ferramenta `create-test`

Para automatizar a criação de testes, propõe-se a adição de uma nova ferramenta ao servidor MCP.

### 3.1. Especificação da Ferramenta

*   **Nome:** `create_test`
*   **Parâmetros:**
    *   `hierarchy` (str): Nome do teste ou caminho da classe a ser testada (ex: `math/matrix` ou `my_experiment`).
    *   `flg_adhoc` (bool, default: `False`): Se `True`, cria um teste adhoc. Se `False`, cria um teste estruturado.

### 3.2. Lógica de Implementação

#### Caso `flg_adhoc = True`:
1.  Listar diretórios em `tests/adhoc/`.
2.  Identificar o maior contador atual e incrementar.
3.  Criar o diretório `tests/adhoc/NNNN_<hierarchy>/`.
4.  Gerar o arquivo `tests/adhoc/NNNN_<hierarchy>/NNNN_<hierarchy>.cpp`.

#### Caso `flg_adhoc = False`:
1.  Processar a `hierarchy` para determinar o caminho (ex: `sak/math`).
2.  Criar a estrutura de pastas em `tests/sak/math/`.
3.  Gerar o arquivo `tests/sak/math/test_sak_math_matrix.cpp` (substituindo `/` por `_` no nome do arquivo).

### 3.3. Novo Template Sugerido (`test-cpp.txt`)

```cpp
{{import file-header}}

//	--------------------------------------------------
#include <print>
#include <string>
#include <vector>
#include <cstdlib>

{{list_open include_list}}#include <{{list_item include_list}}>
{{list_close include_list}}
//	--------------------------------------------------

auto main( const int argument_count, const char* argument_values[ ] ) -> int
{{
	using	::std::string;
	using	::std::vector;
	using	::std::println;

	const vector< string > arguments( argument_values, argument_values + argument_count );
	
	println( "Iniciando testes para: {{hierarchy}}..." );

	// TODO: Implementar testes aqui

    return	EXIT_SUCCESS;
}};
```

Esta ferramenta garantirá que todos os testes sigam o padrão do projeto, incluindo cabeçalhos corretos e a estrutura de `main` exigida pelas convenções locais.
