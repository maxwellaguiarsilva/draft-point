




#### 2.3. Prevenção de Dependência Circular
Existe um risco de dependência circular entre `file_generator.py` e `template_engine.py` (um gera arquivos usando templates, o outro renderiza templates usando metadados do projeto). 
*   **Solução:** O `template_engine.py` deve ser estritamente uma "Pure Library" que não conhece a estrutura de pastas do projeto, apenas processa strings e arquivos de template em disco.
-   Muito das coisas que atualmente esses dois arquivos fazem são regras de negócio das ferramentas MCP.
-   O fato de que mais de uma vai precisar do mesmo recurso indica que o local correto desses códigos são dentro de `lib`
    -   Um bom exemplo é a questão dos metadados.
Veredito: Quero que você analise essa estrutura do código atual e planeje a refatoração. Adicione os detalhes do plano de ação no documento.


#### 2.4. Mapeamento de Erros "Ensure"
O documento deve padronizar que o `ensure(condition, message)` do `lib.common` deve ser a forma primária de validação de qualquer coisa, garantindo que o retorno para o MCP seja sempre limpo.




