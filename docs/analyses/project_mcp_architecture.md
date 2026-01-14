# Arquitetura e Operação do Sistema Project-Mcp-Tools

Este documento descreve a arquitetura técnica, componentes e fluxos de trabalho do sistema de ferramentas Model Context Protocol (MCP) do projeto.

## 1. Filosofia de Design: Gold Standard

O sistema opera sob um padrão agnóstico e modularizado. O servidor MCP atua como um orquestrador que não conhece a implementação interna das ferramentas, delegando a execução para scripts especializados.

### 1.1. Especificação de Ferramenta
Para ser considerada Gold Standard, uma ferramenta segue rigorosamente estes requisitos:

1.  **Naming**: O arquivo usa `snake_case` (ex: `quick_upload.py`). Isso permite que as ferramentas importem umas às outras nativamente via Python.
2.  **Header**: Contém o `shebang` (`#!/usr/bin/python3`) seguido pelo cabeçalho de licença padrão do projeto e o campo `File:` com o nome de arquivo correto.
3.  **Entry Point**: Utiliza a função `run_main` do módulo `lib.common`. O `run_main` estabelece um contrato onde o primeiro argumento recebido via CLI (`sys.argv[1]`) deve ser obrigatoriamente uma string JSON, que será decodificada e injetada na função principal da ferramenta.
4.  **Main Function**: Nomeada `run_<tool_name>` (ex: `run_quick_upload`) e aceita um único parâmetro de dicionário (`params`) proveniente do JSON decodificado.
5.  **Return**: Em caso de sucesso, retorna apenas a string de conteúdo informativo. A responsabilidade por indicar "success" ou "failure" no nível da interface pertence ao Dispatcher (`project_mcp.py`).
6.  **Error Handling**: Para falhas, a ferramenta lança uma `Exception` (geralmente via `ensure`) com uma mensagem clara e descritiva. O `lib.common` captura o erro, imprime a mensagem limpa no `stderr` e sai com status 1, sinalizando a falha ao orquestrador.
7.  **Process Decoupling**: Se uma ferramenta precisa de outra, ela realiza um `import` nativo da função `run_` em vez de disparar um `subprocess`.

### 1.2. O Dispatcher (Servidor MCP)
Localizado em `tools/project_mcp.py`, ele utiliza a biblioteca `FastMCP`.
*   **Agnostic**: Utiliza a função `_invoke_tool`, que despacha a execução para os scripts em `tools/`.
*   **Mapping**: Associa comandos MCP aos respectivos scripts, permitindo que uma única ferramenta (como `file_generator.py`) atenda a múltiplos comandos (`create_class`, `create_test`).

---

## 2. Componentes e Ferramentas

Todas as ferramentas foram migradas para o padrão `snake_case` e utilizam a infraestrutura modular em `tools/lib/`.

### 2.1. Ferramentas Disponíveis
| Ferramenta | Arquivo | Descrição |
| :--- | :--- | :--- |
| `analyze` | `analyze.py` | Análise estática (cppcheck) e formatação automática. |
| `compile` | `compile.py` | Compilação e linkagem paralela com grafo de dependências. |
| `code_verifier` | `code_verifier.py` | Validação e correção de regras de estilo e formatação. |
| `file_generator`| `file_generator.py`| Geração de classes (`create_class`) e testes (`create_test`). |
| `quick_upload` | `quick_upload.py` | Ciclo Git (pull, add, commit, push) e métricas. |
| `agent_statistic`| `agent_statistic.py`| Registro de métricas de comportamento do agente. |
| `include_tree` | `include_tree.py` | Análise recursiva da árvore de includes. |
| `adhoc_tool` | `adhoc_tool.py` | Execução de lógica experimental. |

### 2.2. Bibliotecas de Suporte (`tools/lib/`)
*   **`common.py`**: Contrato `run_main`, `ensure` e utilitários de subprocesso.
*   **`project_core.py`**: Motor de build, mapeamento de arquivos e grafo de dependências.
*   **`metadata_provider.py`**: Provedor de metadados canônicos (licenças, autores, datas).
*   **`template_engine.py`**: Renderização de templates para geração de arquivos.

---

## 3. Workflows

### 3.1. Execução via MCP
1.  **Call**: O agente chama `mcp.compile( )`.
2.  **Dispatch**: `_invoke_tool("compile")` em `project_mcp.py`.
3.  **Shell**: Executa `python3 tools/compile.py '{}'`.
4.  **Logic**: `run_main(run_compile)` processa a requisição e interage com o `project_core`.
5.  **Output**: O retorno limpo é encapsulado pelo rótulo de sucesso do MCP.

### 3.2. Integração entre Ferramentas
As ferramentas agora interagem por meio de importações nativas, eliminando subprocessos Python internos:
```python
from code_verifier import run_code_verifier

def run_analyze(params):
    #   ...
    run_code_verifier({"files": files, "flg_auto_fix": True})
    #   ...
```

---

## 4. Guia de Implementação para Novas Ferramentas

Ao criar uma nova ferramenta, utilize o template:

```python
#!/usr/bin/python3

# [Standard License Header]
# File: tool_name

from lib.common import run_main

def run_tool_name(params):
    #   logic here
    return "result"

if __name__ == "__main__":
    run_main(run_tool_name)
```

---

## 5. Notas de Manutenção
*   **Refactoring**: O objetivo de eliminar o código legado e hífens nos nomes de arquivos foi atingido.
*   **Performance**: O uso de `project_core` com paralelismo otimiza o build e a análise.