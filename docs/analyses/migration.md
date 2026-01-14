# Migração para o Padrão Gold Standard (v3)

Este define o roteiro para a modernização das ferramentas MCP do projeto, eliminando o legado de subprocessos e acoplamento ineficiente entre scripts Python.

## 1. Diagnóstico: O Problema do "Subprocess Glue"

A arquitetura atual utiliza arquivos Python como scripts isolados que se comunicam via `subprocess.run`. Isso causa:
*   **Incerteza de Caminhos:** Dependência de caminhos relativos rígidos (ex: `tools/code-verifier.py`).
*   **Overhead de Inicialização:** Cada formatação de arquivo em um build dispara um novo interpretador Python.
*   **Dificuldade de Importação:** Nomes com hífen (`-`) impedem o uso de `import` nativo, forçando o uso de `importlib.util`.
*   **Acoplamento Oculto:** O `project-builder.py` conhece a interface de linha de comando do `code-verifier.py`, mas não sua lógica interna.

## 2. Nova Arquitetura: Kernel e Ferramentas Especializadas

A solução consiste em transformar a "lógica de negócio" do build e da geração de arquivos em bibliotecas (`lib/`) e as ferramentas MCP em orquestradores que as consomem nativamente via `import`.

### 2.1. O Motor: `tools/lib/project_kernel.py`
Extração das classes core de `project-builder.py`:
*   **Classes**: `project_file`, `cpp`, `hpp`, `binary_builder`, `project`.
*   **Responsabilidade**: Gerenciamento de metadados, grafo de dependências e **orquestração de paralelismo** (ThreadPoolExecutor).
*   **Utilitários**: `DEFAULT_CONFIG`, `deep_update` e `get_cpu_count`.
*   **Mudança Chave:** Esta biblioteca deve ser pura e não invocar outras ferramentas Python via subprocesso.

### 2.2. O Motor de Metadados e Templates
*   **`tools/lib/template_engine.py`**: O atual `tools/template.py` deve ser refatorado para expor uma função `render(template_name, data)` importável.
*   **`tools/file_generator.py`**: Além de ser uma ferramenta, deve expor a lógica de Git (`get_canonical_metadata`) para que o `code_verifier` possa validar licenças sem subprocessos.

### 2.3. Validação Atômica: `tools/code_verifier.py`
Renomeado de `code-verifier.py`.
*   **Interface Programática**: Expõe a classe `formatter` para uso interno pelo Kernel ou ferramentas de análise.
*   **Interface Gold Standard**: Função `run_verify_formatting(params)` para uso via MCP.
*   **Integração**: Consumido por `analyze.py` via `import` direto.

## 3. Requisitos de Implementação (Gold Standard)

Para que a migração siga o padrão de qualidade do projeto, os novos arquivos devem:

1.  **Nomenclatura**: Uso estrito de `snake_case` em todos os arquivos e símbolos.
2.  **Ponto de Entrada**: Uso obrigatório de `run_main` do módulo `lib.common`.
3.  **Interface de Função**: Função principal `run_<tool_name>(params: dict) -> str` que retorna o resultado final (em vez de apenas imprimir).
4.  **Erros**: Usar `ensure( expression, "mensagem" )` para validações, delegando o encerramento e a formatação da mensagem de erro ao `lib.common`.
5.  **Thread-Safety**: O Kernel deve garantir que a saída de múltiplas threads não corrompa o fluxo de dados, especialmente ao rodar sob o MCP.

## 4. Roteiro de Implementação em Fases

### Fase 1: Fundação (Infraestrutura Base)
1.  Renomear `file-generator.py` -> `file_generator.py` e `code-verifier.py` -> `code_verifier.py`.
2.  Criar `tools/lib/project_kernel.py` extraindo a lógica de `project-builder.py`.
3.  Refatorar `tools/template.py` para ser importável.
4.  Ajustar `project_mcp.py` para apontar para os novos nomes (Legacy Mode).

### Fase 2: Desacoplamento e Analizadores
1.  Substituir o uso de `importlib` em `code_verifier.py` por `import file_generator`.
2.  Substituir chamadas de subprocesso do `template.py` por chamadas de função direta.
3.  Criar `tools/analyze.py`, que importa o Kernel e o `code_verifier` para executar a análise total sem disparar novos processos Python.

### Fase 3: Orquestração de Build e Limpeza
1.  Criar `tools/compile.py` como entry-point para o Kernel.
2.  Remover `project-builder.py`.
3.  Em `project_mcp.py`, migrar todos os `@mcp.tool` para o padrão de invocação direta de módulos.

## 5. Matriz de Transformação de Comandos

| Comando Atual | Script Legado | Novo Script/Módulo | Tipo |
| :--- | :--- | :--- | :--- |
| `compile` | `project-builder.py` | `compile.py` | Gold Standard |
| `analyze` | `project-builder.py` | `analyze.py` | Gold Standard |
| `verify_formatting` | `code-verifier.py` | `code_verifier.py` | Gold Standard |
| `create_class` | `file-generator.py` | `file_generator.py` | Gold Standard |
| `create_test` | `file-generator.py` | `file_generator.py` | Gold Standard |
| N/A | `project-builder.py` | `lib/project_kernel.py` | Biblioteca |
| N/A | `template.py` | `lib/template_engine.py`| Biblioteca |

## 6. Conclusão Técnica

A maturidade exige que nenhuma ferramenta chame outra ferramenta Python via `subprocess.run`. A comunicação deve ser via `import`. O `project_mcp.py` torna-se um dispatcher fino, delegando a complexidade para scripts que seguem o padrão `run_<name>(params)`. O Kernel centraliza o conhecimento sobre a estrutura do projeto, permitindo que ferramentas especializadas foquem apenas em suas tarefas.


