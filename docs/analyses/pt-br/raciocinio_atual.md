### 1. Próximos Passos e Refinamentos

#### A. Refinamento de Metadados de Regras (Abstração de Rule)
A transição de tuplas `( pattern, replacement, message )` para uma `dataclass Rule` na `base_verifier.py` permitirá:
-   **Clareza Semântica:** Substituir índices (`rule[0]`) por nomes (`rule.pattern`).
-   **Flexibilidade:** Adicionar campos como `flags` (ex: `re.MULTILINE`), `is_exclusive` (para disparar `_apply_with_exclusion` automaticamente) e `description` para documentação interna.
-   **Validação:** O construtor da `Rule` pode validar o regex em tempo de definição.

#### B. Expansão e Parametrização de Regras Comuns
Mover regras compartilhadas para `common_rules` na `base_verifier`, mas com suporte a parâmetros:
-   **`consecutive_newlines`:** Universal para o projeto (máximo 2 linhas vazias).
-   **`return_spacing`:** Unificar a lógica na base, mas suportando dois modos de operação: **Fixo** (ex: Python exige sempre 2 espaços) e **Contextual/Pareado** (ex: C++, que exige que o caractere após o `return` seja idêntico ao caractere de indentação capturado antes dele, seja espaço ou tab). A abstração deve permitir que o motor de regras utilize grupos de captura ou funções de substituição para manter essa coerência.
-   **`trailing_whitespace`:** Garantir que todas as linguagens herdem a mesma lógica de limpeza de fim de linha e fim de arquivo.

#### C. Validação de Shebangs e Cabeçalhos Genéricos
Atualmente, `_validate_license` é rígida. A melhoria deve focar em:
-   **`allowed_shebangs`:** Uma lista em vez de uma string única. Se o arquivo já tiver um shebang válido (ex: `#!/usr/bin/env python3` vs `#!/usr/bin/python3`), ele deve ser preservado.
-   **Ordem de Preferência:** Se nenhum shebang for encontrado em arquivos que exigem (scripts), usar o primeiro da lista de permitidos.
-   **Separação de Preocupações:** Isolar a lógica de detecção de shebang da lógica de validação de licença para facilitar testes unitários.

#### D. Desacoplamento do Mecanismo de Exclusão
A lógica de `_apply_with_exclusion` hoje depende de encontrar `\n\n\n` (fim do cabeçalho). Isso deve ser generalizado para que a "zona de exclusão" ou "zona de aplicação" possa ser definida por outros critérios, permitindo aplicar regras apenas no corpo do código sem afetar o cabeçalho de licença de forma mais robusta.
