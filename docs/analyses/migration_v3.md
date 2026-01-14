# Migração para o Padrão Gold Standard (v3)

Este documento consolida as análises anteriores e define o roteiro definitivo para a modernização das ferramentas MCP do projeto, eliminando o legado de subprocessos e acoplamento ineficiente.

## 1. Diagnóstico Final: O Problema do "Subprocess Glue"

A arquitetura atual utiliza arquivos Python como scripts isolados que se comunicam via `subprocess.run`. Isso causa:
*   **Incerteza de Caminhos:** Dependência de caminhos relativos rígidos (ex: `tools/code-verifier.py`).
*   **Overhead de Inicialização:** Cada formatação de arquivo em um build dispara um novo interpretador Python.
*   **Dificuldade de Importação:** Nomes com hífen (`-`) impedem o uso de `import` nativo, forçando o uso de `importlib.util`.
*   **Acoplamento Oculto:** O `project-builder.py` conhece a interface de linha de comando do `code-verifier.py`, mas não sua lógica interna.

## 2. Nova Arquitetura: Kernel e Ferramentas Especializadas

A solução consiste em transformar a "lógica de negócio" do build e da geração de arquivos em bibliotecas (`lib/`) e as ferramentas MCP em meros orquestradores que as consomem nativamente.

### 2.1. O Motor: `tools/lib/project_kernel.py`
Extração das classes core de `project-builder.py`:
*   `project_file`, `cpp`, `hpp`, `binary_builder`: Gerenciamento de metadados e grafo de dependências.
*   `project`: Orquestrador central de estado.
*   **Utilitários**: `DEFAULT_CONFIG`, `deep_update` e `get_cpu_count`.
*   **Mudança Chave:** Esta biblioteca deve ser pura e não invocar outras ferramentas via subprocesso.

### 2.2. A Base de Metadados: `tools/lib/metadata_engine.py` (ou `file_generator.py`)
A lógica de `file-generator.py` (Git metadata, templates de licença) deve ser migrada para um módulo importável. Isso permitirá que:
1.  `code_verifier.py` valide licenças importando a lógica.
2.  `file_generator.py` (ferramenta) exponha `create_class` e `create_test`.

### 2.3. Validação Atômica: `tools/code_verifier.py`
Renomeado de `code-verifier.py`.
*   **Interface Programática:** Expõe a classe `formatter`.
*   **Interface MCP:** `verify_formatting` (para arquivos específicos).
*   **Integração:** Consumido por `analyze.py` via `import`.

## 3. Requisitos de Implementação (Gold Standard)

Para que a migração siga o padrão de qualidade do projeto, os novos arquivos devem:

1.  **Nomenclatura**: Uso estrito de `snake_case`.
2.  **Cabeçalho**: Incluir `shebang` (`#!/usr/bin/python3`), bloco de licença GPLv3 e campo `File: <nome>`.
3.  **Ponto de Entrada**: Uso obrigatório de `run_main` do módulo `lib.common`.
4.  **Interface**: Função principal `run_<tool_name>(params)` aceitando um dicionário.
5.  **Erros**: Usar `ensure( expression, "mensagem" )` para validações, delegando a saída ao `lib.common`.

## 4. Roteiro de Implementação em Fases

### Fase 1: Fundação (Infraestrutura Base)
1.  Renomear `file-generator.py` -> `file_generator.py`.
2.  Criar `tools/lib/project_kernel.py` extraindo a lógica de `project-builder.py`.
3.  Ajustar `project_mcp.py` para apontar para os novos nomes, mesmo que via "legacy mode" temporário.

### Fase 2: Desacoplamento de Qualidade
1.  Renomear `code-verifier.py` -> `code_verifier.py`.
2.  Substituir o uso de `importlib` por `import file_generator`.
3.  Criar `tools/analyze.py`, que importa `project_kernel` e `code_verifier` para executar a análise total sem subprocessos para cada arquivo.

### Fase 3: Orquestração de Build
1.  Criar `tools/compile.py` consumindo o `project_kernel`.
2.  Implementar a lógica de compilação paralela dentro de `compile.py` chamando as classes do kernel.

### Fase 4: Limpeza e Padronização Gold Standard
1.  Remover `project-builder.py`.
2.  Em `project_mcp.py`, eliminar `_legacy_run_and_format` e `_special_tool_config`.
3.  Mover todos os `@mcp.tool` para o padrão `_invoke_tool` (invocação direta de scripts snake_case que aceitam `params: dict`).

## 4. Matriz de Transformação de Comandos

| Comando Atual | Script Legado | Novo Script/Módulo | Tipo |
| :--- | :--- | :--- | :--- |
| `compile` | `project-builder.py` | `compile.py` | Gold Standard |
| `analyze` | `project-builder.py` | `analyze.py` | Gold Standard |
| `verify_formatting` | `code-verifier.py` | `code_verifier.py` | Gold Standard |
| `create_class` | `file-generator.py` | `file_generator.py` | Gold Standard |
| `create_test` | `file-generator.py` | `file_generator.py` | Gold Standard |
| N/A | `project-builder.py` | `lib/project_kernel.py` | Biblioteca |

## 5. Conclusão Técnica

A maturidade "v3" exige que nenhuma ferramenta chame outra ferramenta via `python3 tools/script.py`. A comunicação deve ser via `import`. O `project_mcp.py` deve tornar-se um dispatcher fino, delegando toda a complexidade para scripts que seguem o padrão `run_<name>(params)`.
