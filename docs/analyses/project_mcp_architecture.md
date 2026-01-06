# Arquitetura e Operação do Sistema Project-Tools MCP

Este documento descreve a arquitetura técnica, os componentes e os fluxos de trabalho do sistema de ferramentas Model Context Protocol (MCP) do projeto.

## 1. Filosofia de Design: Desacoplamento e Agilidade

O sistema adota um padrão de **Dispatcher-Script**, onde as responsabilidades são divididas para maximizar a agilidade no desenvolvimento:

*   **Dispatcher (Servidor MCP):** Localizado em `tools/project-tools-mcp`, utiliza a biblioteca `FastMCP`. Sua única função é expor a interface das ferramentas e despachar a execução para scripts externos via `subprocess.run`.
*   **Lógica (Scripts de Ferramentas):** Scripts como `file_generator.py`, `template.py`, `ensure_code_formatting.py` e `project-builder.py` contêm a inteligência real.

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

### 2.3. O Formatador e Verificador (`ensure_code_formatting.py`)
Garante a aderência estrita às regras de estilo (Hard Rules).

*   **Regras Implementadas:**
    1.  **Licença:** Restaura ou valida o cabeçalho de licença baseado nos metadados canônicos.
    2.  **Espaçamento de `return`:** Exige exatamente um espaço ou tab (combinando com a indentação) após a palavra `return`.
    3.  **Newlines Consecutivas:** Máximo de 2 linhas vazias consecutivas permitidas.
    4.  **Final de Arquivo:** Exige exatamente 2 linhas vazias ao final, sem espaços residuais.
    5.  **Includes:** Proíbe linhas vazias entre diretivas `#include`.
    6.  **Espaçamento Interno:** Verificação rigorosa de `( space )` e `[ space ]`.

### 2.4. O Orquestrador de Build e Qualidade (`project-builder.py`)
Centraliza as operações de compilação, análise estática e correção de estilo.

*   **Gestão de Dependências:** Analisa recursivamente os `#include` para determinar a necessidade de recompilação (incremental build).
*   **Paralelismo:** Utiliza `ThreadPoolExecutor` para compilar e linkar múltiplos alvos simultaneamente, otimizando o uso dos cores da CPU.
*   **Interface Unificada:** Atua como o ponto de entrada principal para as ferramentas `compile` e `check` do MCP.

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
3.  Após validação, a lógica é migrada para um script definitivo e registrada no servidor MCP (exigindo reinicialização do servidor neste passo final).

### 3.3. Verificação de Integridade e Estilo (`check`)
A ferramenta `check` do MCP é um atalho para `project-builder.py --check`. Este fluxo garante a "Sanidade Total" antes de um commit:
1.  **Correção de Estilo (`fix_format`):** Invoca `ensure_code_formatting.py --fix` em todos os arquivos fonte. Se violações forem encontradas, o script as corrige automaticamente e as reporta.
2.  **Análise Estática (`check_code`):** Executa o `cppcheck` com nível de rigor exaustivo. Qualquer aviso é tratado como erro (exit code 1).

### 3.4. Compilação e Build (`compile`)
A ferramenta `compile` do MCP invoca `project-builder.py` sem argumentos. O fluxo é otimizado para velocidade:
1.  **Pré-verificação:** Executa a análise estática (`cppcheck`). Diferente do fluxo `check`, este passo **não** executa o formatador automático para evitar efeitos colaterais inesperados durante o ciclo de desenvolvimento rápido.
2.  **Compilação Incremental:** Compila apenas os arquivos `.cpp` cujas dependências (arquivos `.hpp`) ou o próprio conteúdo foram alterados desde a última build.
3.  **Linkagem Paralela:** Gera os binários finais no diretório `dist/`.

---

## 4. Referência de Ferramentas MCP

| Ferramenta | Script de Destino | Descrição |
| :--- | :--- | :--- |
| `create_class` | `file_generator.py` | Cria par `.hpp`/`.cpp` com namespaces e metadados. |
| `create_test` | `file_generator.py` | Cria testes adhoc ou estruturados. |
| `compile` | `project-builder.py` | Realiza análise estática e compilação incremental paralela. |
| `check` | `project-builder.py --check` | Aplica correções de estilo automáticas e análise estática exaustiva. |
| `verify_spacing` | `ensure_code_formatting.py` | Verifica especificamente a regra `( )` e `[ ]`. |
| `verify_rules` | `ensure_code_formatting.py` | Reporta violações de regras (return, newlines, etc). |
| `adhoc_tool` | `adhoc_tool.py` | Executa lógica experimental. |

---

## 5. Notas Técnicas de Manutenção
*   **Adição de Templates:** Novos arquivos `.txt` em `docs/templates/` ficam disponíveis automaticamente para o `template.py`.
*   **Novas Ferramentas:** Devem ser decoradas com `@mcp.tool()` em `tools/project-tools-mcp`. Lembre-se que o cliente (Gemini CLI) só reconhece novas assinaturas após o reinício do servidor.
*   **Regras de Estilo:** Qualquer alteração nas regras de C++ deve ser refletida na classe `formatter` em `ensure_code_formatting.py`.