## MODEL ✨

I have read and understood the `llm_interaction_principles.md` and `mcp_tools_debt.md` documents. I will strictly adhere to the interaction philosophy of prioritizing principles (DRY, SRP, Visual Serenity) over delivery, making no unilateral decisions, and working in small, iterative steps while maintaining the established coding style.

Would you like to start by addressing one of the technical debts identified in `mcp_tools_debt.md`, or is there another task you would like me to prioritize?

---

## USER 🧑‍💻

analyze how we can resolve the first item on the list of technical debts.

---

## MODEL ✨

Para resolver o primeiro item da lista de débitos técnicos (**Dynamic Tool Discovery**), proponho uma estratégia fundamentada nos princípios de **DRY** e **SRP**, eliminando a necessidade de manter o `tool_registry` estático em `tools/project_mcp.py`.

### Análise da Solução Proposta

Atualmente, existe uma duplicação de informações: a definição das ferramentas está no MCP server, enquanto a lógica e a validação de parâmetros estão nos scripts individuais. A solução ideal é tornar cada script a "fonte da verdade" sobre sua própria interface.

#### 1. Elevação Semântica das Ferramentas
Cada ferramenta (ex: `create_class.py`) deve ser refatorada para usar **Type Hints** e **Docstrings** em sua função principal (`run_<nome>`). 
- A **Docstring** será extraída automaticamente como a descrição da ferramenta no MCP.
- A **Assinatura da Função** definirá os parâmetros, tipos e valores padrão.

Exemplo de como ficaria a função em `tools/cpp/create_class.py`:
```python
def run_create_class( 
     class_hierarchy: str
    ,include_list: list[ str ] = [ ]
    ,using_list: list[ str ] = [ ]
    ,create_header_only: bool = False 
) -> str:
    """creates a new c++ class with corresponding .hpp and .cpp files..."""
    # lógica da ferramenta usando os argumentos nomeados
```

#### 2. Descoberta Dinâmica no MCP Server
O arquivo `tools/project_mcp.py` passará a realizar um "crawl" no diretório `tools/`:
1. Percorre os subdiretórios (`cpp`, `git`, `llm`, `python`).
2. Para cada arquivo `.py`, utiliza o módulo `importlib` para carregar o script como um módulo.
3. Localiza a função padronizada `run_<nome_do_arquivo>`.
4. Utiliza a biblioteca `inspect` do Python para extrair os metadados (docstring e assinatura) diretamente do objeto da função.
5. Registra a ferramenta no `FastMCP` dinamicamente.

#### 3. Adaptação do `tools/lib/common.py`
A função `run_mcp_tool` deve ser atualizada para suportar essa nova forma de chamada:
- Ela detectará se a função de ação aceita argumentos nomeados (`**kwargs` ou nomes específicos).
- Ao ser executada via sub-processo pelo MCP server, ela injetará os parâmetros JSON recebidos diretamente na função.

### Benefícios
- **DRY Extremo**: Para adicionar uma nova ferramenta, basta criar o arquivo `.py` com a função `run_` e os metadados corretos. O MCP a reconhecerá automaticamente no próximo restart.
- **Visual Serenity**: O código das ferramentas fica mais limpo, sem extrações manuais de `params.get( "chave" )`.
- **Single Responsibility Principle**: Cada ferramenta é responsável por definir seu contrato e sua lógica.

### Próximos Passos Sugeridos
1. Atualizar `tools/lib/common.py` com a lógica de extração de metadados via `inspect`.
2. Refatorar uma ferramenta "piloto" (sugiro `cpp/create_class.py`) para validar o modelo.
3. Implementar o loop de descoberta em `tools/project_mcp.py`.


