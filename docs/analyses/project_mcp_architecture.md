# Arquitetura e Operação do Sistema Project-Mcp-Tools

Este documento descreve a arquitetura técnica, os componentes e os fluxos de trabalho do sistema de ferramentas Model Context Protocol (MCP) do projeto.

## 1. Filosofia de Design: Padrão Ouro (Gold Standard)

O sistema está em transição de um modelo acoplado para um padrão agnóstico e idiomático denominado **Gold Standard**. O objetivo é que o servidor MCP seja apenas um orquestrador que não conhece a implementação interna das ferramentas.

### 1.1. Especificação de uma Ferramenta Gold Standard
Para ser considerada Gold Standard, uma ferramenta deve seguir rigorosamente estes requisitos:

1.  **Nomenclatura**: O arquivo deve usar `snake_case` (ex: `quick_upload.py`). Isso permite que ferramentas importem umas às outras nativamente via Python.
2.  **Cabeçalho**: Deve conter o `shebang` (`#!/usr/bin/python3`) seguido pelo cabeçalho de licença padrão do projeto e o campo `File:` com o nome correto do arquivo.
3.  **Ponto de Entrada**: Deve utilizar a função `run_main` do módulo `lib.common`.
4.  **Função Principal**: Deve ser nomeada como `run_<nome_da_ferramenta>` (ex: `run_quick_upload`) e aceitar um único parâmetro do tipo dicionário (`params`).
5.  **Retorno**: Deve retornar apenas a string de conteúdo informativo. A responsabilidade de indicar "sucesso" ou "falha" no nível de interface é do Dispatcher (`project-mcp.py`).
6.  **Desacoplamento de Processo**: Se uma ferramenta precisar de outra (ex: `quick_upload` chamando `agent_statistic`), ela deve fazer o `import` nativo da função `run_` em vez de disparar um `subprocess`.

### 1.2. O Dispatcher (Servidor MCP)
Localizado em `tools/project-mcp.py`, utiliza a biblioteca `FastMCP`.
*   **Agnóstico**: Para ferramentas Gold Standard, ele utiliza a função `_run_and_format`, que assume que o nome da ferramenta MCP é idêntico ao nome do arquivo `.py` em `tools/`.
*   **Legado**: Ferramentas que ainda não foram migradas utilizam `_legacy_run_and_format` e dependem do dicionário `_special_tool_config` para mapear subcomandos e nomes de arquivos antigos (`kebab-case`).

---

## 2. Componentes e Status de Migração

### 2.1. Ferramentas Gold Standard (Migradas)
| Ferramenta | Arquivo | Descrição |
| :--- | :--- | :--- |
| `adhoc_tool` | `adhoc_tool.py` | Executa lógica experimental e prototipagem. |
| `quick_upload` | `quick_upload.py` | Ciclo git (pull, add, commit, push) e registro de estatísticas. |
| `agent_statistic` | `agent_statistic.py` | Registro e consulta de métricas de comportamento do agente. |

### 2.2. Ferramentas Legadas (Pendentes de Migração)
Estas ferramentas ainda usam o padrão `kebab-case`, subcomandos manuais e não seguem a estrutura de função `run_`.

*   **`file-generator.py`**: (`create_class`, `create_test`). Gerador de boilerplate e templates.
*   **`code-verifier.py`**: (`verify_formatting`). Validador de Hard Rules de estilo.
*   **`project-builder.py`**: (`compile`, `analyze`). Orquestrador de build, cppcheck e formatação.

---

## 3. Fluxos de Trabalho Gold Standard

### 3.1. Execução via MCP
O fluxo de execução segue o caminho:
1.  **Chamada**: `mcp.quick_upload(message="...")` no `project-mcp.py`.
2.  **Despacho**: `_run_and_format("quick_upload", {"message": "..."})`.
3.  **Shell**: `python3 tools/quick_upload.py '{"message": "..."}'`.
4.  **Lógica**: `run_main(run_quick_upload)` processa o JSON e executa a tarefa.
5.  **Output**: O retorno limpo da ferramenta é encapsulado pelo label de sucesso do MCP.

### 3.2. Integração entre Ferramentas
Uma ferramenta Gold Standard pode invocar outra sem criar novos processos:
```python
from agent_statistic import run_agent_statistic

def run_my_tool(params):
    # ... lógica ...
    return run_agent_statistic({"name": "success"})
```

---

## 4. Guia de Implementação para Novas Ferramentas

Ao criar uma nova ferramenta, utilize o seguinte template básico:

```python
#!/usr/bin/python3

# [Cabeçalho de Licença Padrão]
# File: nome_da_ferramenta

from lib.common import run_main

def run_nome_da_ferramenta(params):
    # Lógica aqui
    return "resultado"

if __name__ == "__main__":
    run_main(run_nome_da_ferramenta)
```

---

## 5. Notas de Manutenção
*   **Higienização**: Após migrar uma lógica de `adhoc_tool.py` para um script definitivo, o `adhoc_tool.py` deve ser limpo.
*   **Refatoração**: O objetivo a longo prazo é eliminar o `_special_tool_config` do `project-mcp.py` à medida que todos os scripts forem migrados para o padrão Gold Standard em `snake_case`.