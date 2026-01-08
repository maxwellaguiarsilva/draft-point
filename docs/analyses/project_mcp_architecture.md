# Arquitetura e Operação do Sistema Project-Mcp-Tools

Este documento descreve a arquitetura técnica, os componentes e os fluxos de trabalho do sistema de ferramentas Model Context Protocol (MCP) do projeto.

## 1. Filosofia de Design: Desacoplamento e Agilidade

O sistema adota um padrão de **Dispatcher-Script**, onde as responsabilidades são divididas para maximizar a agilidade no desenvolvimento:

*   **Dispatcher (Servidor MCP):** Localizado em `tools/project-mcp-tools`, utiliza a biblioteca `FastMCP`. Sua única função é expor a interface das ferramentas e despachar a execução para scripts externos via `subprocess.run`. Utiliza helpers internos (`_call`, `_run_and_format`) para padronizar o tratamento de erros e a comunicação, mantendo-se totalmente agnóstico à lógica de processamento dos dados.
*   **Lógica (Scripts de Ferramentas):** Scripts como `file_generator.py`, `template.py`, `code_verifier.py` e `project-builder.py` contêm a inteligência real.

### Vantagens do Modelo
1.  **Hot-Reloading:** Alterações na lógica dos scripts (Python) são aplicadas instantaneamente na próxima execução da ferramenta, sem necessidade de reiniciar o servidor MCP.
2.  **Isolamento:** Falhas críticas em uma ferramenta não derrubam o processo do servidor MCP.
3.  **Testabilidade:** Cada script de ferramenta pode ser executado e testado de forma independente via terminal.

---

## 2. Componentes do Ecossistema

### 2.1. O Motor de Templates (`template.py`)
Implementa um sistema de substituição de strings e processamento de listas baseado em Regex.

*   **Importação Recursiva (`{{import name}}`):** Permite que templates incluam outros arquivos de `docs/templates/`. A resolução é feita no carregamento inicial.
*   **Substituição Simples (`{{key}}`):** Troca de marcadores por strings.
*   **Processamento de Listas:** Utiliza blocos `{{list_open key}}`, `{{list_item key}}` e `{{list_close key}}`. Suporta múltiplas linhas dentro do bloco (via `re.DOTALL`).

### 2.2. O Gerador de Arquivos (`file_generator.py`)
Responsável pela criação de boilerplate seguindo os padrões do projeto.

*   **Metadados Canônicos:** Antes de criar ou formatar um arquivo, o sistema tenta recuperar o autor, e-mail e data do *primeiro commit* do arquivo via `git log`. Se o arquivo for novo, utiliza a configuração global do Git e a data atual.
*   **`create_class`:** Gera `.hpp` e `.cpp`. Os *header guards* são gerados dinamicamente usando os últimos 9 dígitos de `time.time_ns()`.
*   **`create_test`:** 
    *   **Adhoc:** Localizados em `tests/adhoc/NNNN_name/`. O sistema auto-incrementa o contador `NNNN`.
    *   **Estruturados:** Seguem a hierarquia de `source/` ou `include/`, mapeando para `tests/path/test_path_name.cpp`.

### 2.3. O Verificador de Código (`code_verifier.py`)
Garante a aderência estrita às regras de estilo (Hard Rules) e fornece feedback estruturado.

*   **Controle de Ajuste:** Utiliza um flag booleano `flg_fix` na classe `formatter` para alternar entre modo de correção e modo de verificação.
*   **Modos de Retorno:** 
    1.  **JSON Estruturado:** As flags `--formatting` e `--fix` retornam JSON, permitindo que outros scripts (como o `project-builder.py`) processem os resultados programmaticamente.
    2.  **Relatório Formatado:** O comando `verify_formatting` processa múltiplos arquivos e gera um relatório legível para humanos, centralizando a lógica de apresentação que anteriormente residia no Dispatcher.
*   **Regras Implementadas:**
    1.  **Licença:** Restaura ou valida o cabeçalho de licença baseado nos metadados canônicos.
    2.  **Espaçamento de `return`:** Exige exatamente um espaço ou tab após a palavra `return`.
    3.  **Newlines Consecutivas:** Máximo de 2 linhas vazias consecutivas permitidas.
    4.  **Final de Arquivo:** Exige exatamente 2 linhas vazias ao final.
    5.  **Includes:** Proíbe linhas vazias entre diretivas `#include`.
    6.  **Espaçamento Interno:** Verificação rigorosa de `( space )` e `[ space ]`.

### 2.4. O Orquestrador de Build e Análise (`project-builder.py`)
Centraliza as operações de compilação, análise estática e formatação de código.

*   **Gestão de Dependências:** Analisa recursivamente os `#include` para determinar a necessidade de recompilação (incremental build).
*   **Paralelismo:** Utiliza `ThreadPoolExecutor` para compilar e linkar múltiplos alvos simultaneamente.
*   **Fluxo de Análise (`analyze`):** Coordena a execução de `format_code()` (via `code_verifier.py`) e `run_cppcheck()`.

---

## 3. Fluxos de Trabalho Principais

### 3.1. Geração de Nova Classe
1.  Agente chama `create_class(class_hierarchy="n1/n2/name")`.
2.  `file_generator.py` monta o dicionário de dados (namespaces, include lists, metadata).
3.  `template.py` renderiza `class-hpp.txt` e `class-cpp.txt`.
4.  Arquivos são gravados em `include/n1/n2/name.hpp` e `source/n1/n2/name.cpp`.

### 3.2. Ciclo de Prototipagem com `adhoc_tool`
Para evoluir o sistema sem alterar o servidor MCP imediatamente:
1.  O agente modifica `tools/adhoc_tool.py` com a nova lógica experimental.
2.  O agente executa a ferramenta `adhoc_tool(params={...})`.

### 3.3. Análise de Integridade e Estilo (`analyze`)
A ferramenta `analyze` do MCP é um atalho para `project-builder.py --analyze`. Este fluxo garante a conformidade total:
1.  **Formatação (`format_code`):** Invoca `code_verifier.py --fix` em todos os arquivos fonte.
2.  **Análise Estática (`run_cppcheck`):** Executa o `cppcheck` com nível de rigor exaustivo.

### 3.4. Compilação e Build (`compile`)
A ferramenta `compile` do MCP invoca o método `run_build()` do orquestrador:
1.  **Pré-verificação:** Executa a análise lógica (`run_cppcheck`).
2.  **Compilação Incremental:** Compila apenas os arquivos alterados ou cujas dependências mudaram.
3.  **Linkagem Paralela:** Gera os binários finais no diretório `dist/`.

---

## 4. Referência de Ferramentas MCP

| Ferramenta | Script de Destino | Descrição |
| :--- | :--- | :--- |
| `create_class` | `file_generator.py` | Cria par `.hpp`/`.cpp` com namespaces e metadados. |
| `create_test` | `file_generator.py` | Cria testes adhoc ou estruturados. |
| `compile` | `project-builder.py` | Realiza análise estática e compilação incremental paralela. |
| `analyze` | `project-builder.py --analyze` | Aplica formatação automática (incluindo espaçamento) e análise estática exaustiva. |
| `verify_formatting` | `code_verifier.py verify_formatting` | Reporta violações de estilo (return, licença, newlines, espaçamento de brackets). |
| `adhoc_tool` | `adhoc_tool.py` | Executa lógica experimental. |

---

## 5. Notas Técnicas de Manutenção
*   **Dispatcher:** O script `project-mcp-tools` deve permanecer agnóstico, delegando toda a lógica de iteração e formatação de relatórios para os scripts de destino.
*   **Regras de Estilo:** Qualquer alteração nas regras de C++ deve ser refletida na classe `formatter` em `code_verifier.py`.
*   **Consistência de Nomes:** Deve-se manter o alinhamento entre o nome da MCP tool, o flag CLI do script e o método interno no orquestrador.
