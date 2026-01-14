


Após a análise do arquivo `tools/include-analyzer.py` e da configuração atual em `tools/project-mcp.py`, identifiquei os passos necessários para a migração ao padrão **Gold Standard**.

### Relatório de Diagnóstico: Migração `include_tree`

A ferramenta atual reside em `include-analyzer.py` (kebab-case) e opera sob o modelo legado, com processamento manual de argumentos e tratamento de erros via `stderr` direto.

#### 1. Reestruturação de Arquivos e Metadados
- **Renomeação**: O arquivo `tools/include-analyzer.py` deve ser renomeado para `tools/include_tree.py` para seguir o padrão `snake_case`.
- **Cabeçalho**: O campo `File: include-analyzer` no cabeçalho de licença deve ser atualizado para `File: include_tree`.
- **Shebang**: Manter `#!/usr/bin/python3` no topo do arquivo.

#### 2. Mudanças no Código (Padrão Gold Standard)
- **Importações**: 
    - Remover `import json` e `import sys` (não serão mais necessários no escopo global).
    - Adicionar `from lib.common import run_main, ensure`.
- **Assinatura da Função Principal**:
    - Renomear/ajustar a função de entrada para `run_include_tree(params)`, aceitando um único dicionário.
    - Utilizar `ensure(params.get("file_path"), "file_path parameter is required")` para validação.
- **Tratamento de Erros**:
    - Eliminar o bloco `try-except` manual no `if __name__ == "__main__"`.
    - Permitir que as exceções levantadas pela lógica interna ou pelo `ensure` propaguem naturalmente até o `run_main`.
    - Remover prefixos como `error:` das mensagens de exceção.
- **Ponto de Entrada**:
    - Substituir o bloco `__main__` atual por:
      ```python
      if __name__ == "__main__":
          run_main(run_include_tree)
      ```

#### 3. Desacoplamento no Dispatcher (`tools/project-mcp.py`)
- **Remoção de Configuração**: Retirar a entrada `"include_tree": {"script": "include-analyzer"}` do dicionário `_special_tool_config`.
- **Simplificação da Tool**: Atualizar a função decorada com `@mcp.tool()` para utilizar o novo orquestrador agnóstico:
  ```python
  @mcp.tool()
  def include_tree(file_path: str) -> str:
      args = {"file_path": file_path}
      return _invoke_tool("include_tree", args)
  ```

#### 4. Observações de Estilo (Hard Rules)
- Manter o uso de `snake_case` para a classe `include_tree` e métodos, em conformidade com a regra de "no capital letters" do projeto.
- Manter o espaçamento interno em `( )` e `[ ]` (ex: `params.get( "file_path" )`), conforme verificado no código atual e exigido pelas regras de formatação.


