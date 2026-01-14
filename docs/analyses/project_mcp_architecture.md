


# Arquitetura e Operação do Sistema Project-Mcp-Tools

Este documento descreve a arquitetura técnica, componentes e fluxos de trabalho do sistema de ferramentas Model Context Protocol (MCP) do projeto.

## 1. Filosofia de Design: Gold Standard

O sistema está transitando de um modelo acoplado para um padrão agnóstico e modularizado. O objetivo é que o servidor MCP seja apenas um orquestrador que não conhece a implementação interna das ferramentas.

### 1.1. Especificação de Ferramenta
Para ser considerada Gold Standard, uma ferramenta deve seguir rigorosamente estes requisitos:

1.  **Naming**: O arquivo deve usar `snake_case` (ex: `quick_upload.py`). Isso permite que as ferramentas importem umas às outras nativamente via Python.
2.  **Header**: Deve conter o `shebang` (`#!/usr/bin/python3`) seguido pelo cabeçalho de licença padrão do projeto e o campo `File:` com o nome de arquivo correto.
3.  **Entry Point**: Deve usar a função `run_main` do módulo `lib.common`. O `run_main` estabelece um contrato onde o primeiro argumento recebido via CLI (`sys.argv[1]`) deve ser obrigatoriamente uma string JSON, que será decodificada e injetada na função principal da ferramenta.
4.  **Main Function**: Deve ser nomeada `run_<tool_name>` (ex: `run_quick_upload`) e aceitar um único parâmetro de dicionário (`params`) proveniente do JSON decodificado.
5.  **Return**: Em caso de sucesso, deve retornar apenas a string de conteúdo informativo. A responsabilidade por indicar "success" ou "failure" no nível da interface pertence ao Dispatcher (`project-mcp.py`).
6.  **Error Handling**: Para falhas, a ferramenta deve lançar uma `Exception` padrão com uma mensagem clara e descritiva. Esta mensagem *não* deve incluir prefixos como "error:" ou "failure:", pois a rotulagem é responsabilidade do Dispatcher. Isso permite que o `lib.common` capture o erro, imprima a mensagem limpa no `stderr` e saia com status 1, sinalizando a falha ao orquestrador.
7.  **Process Decoupling**: Se uma ferramenta precisar de outra (ex: `quick_upload` chamando `agent_statistic`), ela deve realizar um `import` nativo da função `run_` em vez de disparar um `subprocess`.

### 1.2. O Dispatcher (Servidor MCP)
Localizado em `tools/project-mcp.py`, ele utiliza a biblioteca `FastMCP`.
*   **Agnostic**: Utiliza a função `_invoke_tool`, que assume que o nome da ferramenta MCP é idêntico ao nome do arquivo `.py` em `tools/`.
*   **Legacy**: Ferramentas que ainda não foram migradas usam `_legacy_run_and_format` e dependem do dicionário `_special_tool_config` para mapear subcomandos e nomes de arquivos antigos (`kebab-case`).

---

## 2. Componentes e Status de Migração

### 2.1. Tools (Migradas)
| Ferramenta | Arquivo | Descrição |
| :--- | :--- | :--- |
| `adhoc_tool` | `adhoc_tool.py` | Executa lógica experimental e prototipagem. |
| `quick_upload` | `quick_upload.py` | Ciclo Git (pull, add, commit, push) e registro de estatísticas. |
| `agent_statistic` | `agent_statistic.py` | Registro e consulta de métricas de comportamento do agente. |

### 2.2. Legacy Tools (Pendente Migração)
Estas ferramentas ainda usam o padrão `kebab-case`, subcomandos manuais e não seguem a estrutura da função `run_`.

*   **`file-generator.py`**: (`create_class`, `create_test`). Gerador de boilerplate e templates.
*   **`code-verifier.py`**: (`verify_formatting`). Validador de regras rígidas de estilo (Style Hard Rules).
*   **`project-builder.py`**: (`compile`, `analyze`). Orquestrador de build, cppcheck e formatação.

---

## 3. Workflows

### 3.1. Execution via MCP
O fluxo de execução segue este caminho:
1.  **Call**: `mcp.quick_upload(message="...")` em `project-mcp.py`.
2.  **Dispatch**: `_invoke_tool("quick_upload", {"message": "..."})`.
3.  **Shell**: `python3 tools/quick_upload.py '{"message": "..."}'`. (O Dispatcher envia o dicionário de parâmetros como uma string JSON no primeiro argumento).
4.  **Logic**: `run_main(run_quick_upload)` captura o JSON da CLI, converte para dicionário e executa a tarefa.
5.  **Output**: O retorno limpo da ferramenta é encapsulado pelo rótulo de sucesso do MCP.

### 3.2. Integração entre Ferramentas
Uma ferramenta pode invocar outra sem criar novos processos:
```python
from agent_statistic import run_agent_statistic

def run_my_tool(params):
    # ... logic ...
    return run_agent_statistic({"name": "success"})
```

---

## 4. Guia de Implementação para Novas Ferramentas

Ao criar uma nova ferramenta, use o seguinte template básico:

```python
#!/usr/bin/python3

# [Standard License Header]
# File: tool_name

from lib.common import run_main

def run_tool_name(params):
    #   logic here
    if not params.get( "required_key" ):
        raise Exception( "missing required_key parameter" )
    
    return "result"

if __name__ == "__main__":
    run_main(run_tool_name)
```

---

## 5. Maintenance Notes
*   **Sanitization**: Após migrar a lógica de `adhoc_tool.py` para um script definitivo, `adhoc_tool.py` deve ser limpo.
*   **Refactoring**: O objetivo de longo prazo é eliminar `_special_tool_config` de `project-mcp.py` à medida que todos os scripts forem migrados para o padrão `snake_case` sigam a isonomia de chamadas e retornos.



