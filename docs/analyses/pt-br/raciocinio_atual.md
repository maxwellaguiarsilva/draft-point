### 1. Itens Concluídos

#### A. Refinamento de Metadados de Regras (Abstração de `rule`)
A transição de tuplas `( pattern, replacement, message )` para uma `dataclass rule` na `base_verifier.py` foi realizada, trazendo os seguintes benefícios:
-   **Clareza Semântica:** Uso de atributos nomeados (`rule_obj.pattern`) em vez de índices.
-   **Flexibilidade:** Suporte nativo a `flags` de regex e o campo `is_exclusive`.
-   **Automação:** A lógica de exclusão (evitar aplicar regras em comentários ou strings) agora pode ser disparada automaticamente pela propriedade `is_exclusive`.
-   **Conformidade de Estilo:** A classe foi nomeada como `rule` (em minúsculas) para aderir às convenções de codificação do projeto.

---

### 2. Próximos Passos e Refinamentos

#### B. Expansão e Parametrização de Regras Comuns
Mover regras compartilhadas para `common_rules` na `base_verifier`, mas com suporte a parâmetros:
-   **`consecutive_newlines`:** Universal para o projeto (máximo 2 linhas vazias).
-   **`return_spacing`:** Unificar a lógica na base, mas suportando dois modos de operação: **Fixo** (ex: Python exige sempre 2 espaços) e **Contextual/Pareado** (ex: C++, que exige que o caractere após o `return` seja idêntico ao caractere de indentação capturado antes dele, seja espaço ou tab). A abstração deve permitir que o motor de regras utilize grupos de captura ou funções de substituição para manter essa coerência.

        ##### Plano de Ação para `return_spacing`:
        1.  **Abstração via Factory (`base_verifier.py`)**: Criar um método `_rule_return_spacing( mode, value )` que retorne uma instância de `rule`.
        2.  **Modo Fixo (`fixed`)**: 
            - Regex: `r"(\breturn)[ \t]+(?!\s*;)"`
            - Substituição: `r"\1" + (" " * value)`
            - Aplicável ao Python (value=2).
        3.  **Modo Pareado (`paired`)**:
            - Regex: `r"([ \t])return\b(?![ \t]*;)[ \t]*"`
            - Substituição: `r"\1return\1"`
            - Caso o `return` não tenha indentação (ex: início do arquivo), a factory deve prover um fallback para um espaço simples ou disparar um erro de estilo mais genérico.
        4.  **Tratamento de Exceções**: Ambas as regex devem manter o lookahead negativo `(?!\s*;)` para evitar processar retornos vazios (`return;`).
        5.  **Refatoração dos Verificadores**: Atualizar `cpp_verifier.py` e `python_verifier.py` para consumir a factory, removendo as regex manuais.
-   **`trailing_whitespace`:** Garantir que todas as linguagens herdem a mesma lógica de limpeza de fim de linha e fim de arquivo.

#### C. Validação de Shebangs e Cabeçalhos Genéricos
Atualmente, `_validate_license` é rígida. A melhoria deve focar em:
-   **`allowed_shebangs`:** Uma lista em vez de uma string única. Se o arquivo já tiver um shebang válido (ex: `#!/usr/bin/env python3` vs `#!/usr/bin/python3`), ele deve ser preservado.
-   **Ordem de Preferência:** Se nenhum shebang for encontrado em arquivos que exigem (scripts), usar o primeiro da lista de permitidos.
-   **Separação de Preocupações:** Isolar a lógica de detecção de shebang da lógica de validação de licença para facilitar testes unitários.

#### D. Desacoplamento do Mecanismo de Exclusão
A lógica de `_apply_with_exclusion` hoje depende de encontrar `\n\n\n` (fim do cabeçalho). Isso deve ser generalizado para que a "zona de exclusão" ou "zona de aplicação" possa ser definida por outros critérios, permitindo aplicar regras apenas no corpo do código sem afetar o cabeçalho de licença de forma mais robusta.