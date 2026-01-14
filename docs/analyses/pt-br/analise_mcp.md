


# Migração das ferramentas MCP do projeto

Este documento define o roteiro para a modernização das ferramentas MCP do projeto, eliminando o modelo legado e acoplamento ineficiente entre scripts Python.

## 1. Diagnóstico: O Problema do "Subprocess"

A arquitetura atual utiliza arquivos Python como scripts isolados que se comunicam via `subprocess.run`. No entanto, é fundamental distinguir entre o uso necessário de subprocessos e as transgressões arquiteturais.

### 1.1. Usos Válidos de `subprocess`
O uso de subprocessos é permitido e encorajado nos seguintes cenários:
*   **Dispatcher Principal (`project_mcp.py`):** Deve continuar utilizando `subprocess` para invocar as ferramentas. Isso permite que os desenvolvedores modifiquem o código das ferramentas e vejam as mudanças refletidas imediatamente sem precisar reiniciar o servidor MCP (evitando o cache de `import` do Python).
*   **Binários Externos:** Chamar ferramentas nativas do sistema ou do compilador como `g++`, `clang++`, `make`, `cppcheck`, `git` ou `clang-format`.

### 1.2. A Transgressão: Comunicação Interna "Intra-Tools"
A verdadeira transgressão ocorre quando um script Python que compõe a inteligência das ferramentas MCP invoca outro script Python do projeto via `subprocess`. Isso gera acoplamento ineficiente e dificulta a evolução para um padrão mais organizado.

### 1.3. Transgressões Identificadas
Após análise, as seguintes transgressões foram enumeradas:
1.  **`tools/file-generator.py`**: Invoca `python3 tools/template.py` via `subprocess.run` para renderizar templates.
2.  **`tools/project-builder.py`**: Invoca `python3 tools/code-verifier.py` via `_invoke_subprocess` para formatar o código durante o build.

### 1.4. Impactos Negativos
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
*   **Padronização com outras ferramentas**: Função `run_verify_formatting(params)` para uso via MCP.
*   **Integração**: Consumido por `analyze.py` via `import` direto.

## 3. Requisitos do novo padrão desejado

Para que a migração siga o padrão de qualidade do projeto, os novos arquivos devem:

1.  **Nomenclatura**: Uso estrito de `snake_case` em todos os arquivos e símbolos.
2.  **Ponto de Entrada**: Uso obrigatório de `run_main` do módulo `lib.common`.
3.  **Interface de Função**: Função principal `run_<tool_name>(params: dict) -> str` que retorna a string com a mensagem do resultado final.
4.  **Erros**: Usar `ensure( expression, "mensagem" )` para validações, delegando o encerramento e a formatação da mensagem de erro ao `lib.common`.
5.  **Thread-Safety**: O Kernel deve garantir que a saída de múltiplas threads não corrompa o fluxo de dados, especialmente ao rodar sob o MCP.
    -   Sobre essa questão de `thread-safety`, os seguintes pontos são importantes:
        -   O programador vai ter o fluxo completo encerrado / interrompido assim que qualquer analise, build ou link reportar o primeiro erro.
            -   Ele será orbrigado a tratar os erros um por vez e só verá os demais erros, após ajustar o primeiro e rodar a ferramenta novamente para ver o próximo.
        -   Por isso o sistema identifica quem falhou primeiro e lança a exceção usando "ensure".
        -   As mensagens de sucesso, são protegidas com lock para que não haja `race condition` nelas.

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
| `compile` | `project-builder.py` | `compile.py` | Ferramenta |
| `analyze` | `project-builder.py` | `analyze.py` | Ferramenta |
| `verify_formatting` | `code-verifier.py` | `code_verifier.py` | Ferramenta |
| `create_class` | `file-generator.py` | `file_generator.py` | Ferramenta |
| `create_test` | `file-generator.py` | `file_generator.py` | Ferramenta |
| N/A | `project-builder.py` | `lib/project_kernel.py` | Biblioteca |
| N/A | `template.py` | `lib/template_engine.py`| Biblioteca |

## 6. Conclusão Técnica

A maturidade exige que nenhuma ferramenta chame outra ferramenta Python via `subprocess.run`. A comunicação deve ser via `import`. O `project_mcp.py` torna-se um dispatcher fino, delegando a complexidade para scripts que seguem o padrão `run_<name>(params)`. O Kernel centraliza o conhecimento sobre a estrutura do projeto, permitindo que ferramentas especializadas foquem apenas em suas tarefas.




### 1. Questionamento Crítico: Lacunas de Precisão Técnica

Para que a migração ocorra sem problemas, o documento de migração deveria ser mais explícito nos seguintes pontos:

*   **Contrato de Interface do `lib.common`:** O documento menciona o uso de `run_main`, mas não detalha como ele trata os argumentos. No Gold Standard, o `run_main` espera que o primeiro argumento da CLI seja uma string JSON que será convertida no dicionário `params`. Isso é crucial para que ferramentas chamadas via Kernel funcionem da mesma forma que quando chamadas via CLI/MCP.
*   **Estado e Concorrência no Kernel:** O `project_kernel.py` usará `ThreadPoolExecutor`. O documento precisa definir se o Kernel será **stateless** (recebendo o caminho do projeto em cada chamada) ou se manterá um objeto `ProjectContext`. Para evitar race conditions em builds paralelos, a gestão de escrita em arquivos de log ou cache precisa ser definida.
*   **Tratamento de Exceções em Cadeia:** Quando o `analyze.py` importar e rodar `run_verify_formatting(params)`, uma exceção lançada por este último deve ser capturada pelo orquestrador ou deixada borbulhar até o `run_main` do `analyze.py`? A recomendação Gold Standard sugere que o `run_main` final capture tudo, mas o Kernel precisa de granularidade para saber qual arquivo falhou sem interromper todo o processo (em caso de análise paralela).

---

### 2. Análise Complementar e Sugestões de Refinamento

Para complementar o documento de migração, sugiro a inclusão dos seguintes tópicos técnicos:

#### 2.1. Estrutura de Importação Protegida
Para evitar que a simples importação de um módulo execute lógica (efeitos colaterais), deve-se reforçar a regra de que todo script Gold Standard deve ter sua lógica principal protegida por:
```python
if __name__ == "__main__":
    run_main(run_tool_name)
```
Isso permite que o Kernel importe `run_tool_name` sem disparar o `run_main` do módulo importado.

#### 2.2. O Fluxo de Dados do `analyze`
A ferramenta `analyze` é a mais complexa, pois combina `cppcheck` (análise estática) e `code_verifier` (formatação). 
*   **Proposta de Fluxo:** 
    1.  Kernel mapeia os arquivos.
    2.  Kernel despacha threads para `cppcheck` (subprocesso de binário).
    3.  Kernel despacha chamadas nativas para `code_verifier.run_verify_formatting`.
    4.  Kernel consolida os retornos de string em um relatório único.

#### 2.3. Prevenção de Dependência Circular
Existe um risco de dependência circular entre `file_generator.py` e `template_engine.py` (um gera arquivos usando templates, o outro renderiza templates usando metadados do projeto). 
*   **Solução:** O `template_engine.py` deve ser estritamente uma "Pure Library" que não conhece a estrutura de pastas do projeto, apenas processa strings e arquivos de template em disco.

#### 2.4. Mapeamento de Erros "Ensure"
O documento deve padronizar que o `ensure(condition, message)` do `lib.common` (ou equivalente Python que será criado para espelhar o `sak::ensure` do C++) deve ser a forma primária de validação de parâmetros, garantindo que o retorno para o MCP seja sempre limpo.

### 3. Veredito da Análise

O documento `@docs/analyses/migration.md` é um excelente roteiro estratégico. Com a adição das especificações sobre **binários externos**, **assinatura do `run_main`** e **estratégia de concorrência no Kernel**, ele passará de um guia de intenções para uma especificação técnica executável.



