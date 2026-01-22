### 1. Mecanismo de Execução de Regras (Machinery vs. Data) [CONCLUÍDO]
Implementado `run_rules()` na `base_verifier.py`. As subclasses agora definem regras em `_get_rules()` e o motor as aplica automaticamente.

### 2. Simetria de Configuração [CONCLUÍDO]
Criado `tools/python/python_lib/python_config.py`. Verificadores agora utilizam seus respectivos arquivos de configuração para `comment_string` e outras propriedades.

### 3. Generalização da Lógica de Exclusão [CONCLUÍDO]
Movida a lógica de `_bracket_spacing` para `_apply_with_exclusion` na `base_verifier.py`. Isso permite que qualquer linguagem use regras que ignoram blocos específicos (comentários, strings) de forma declarativa via `bracket_ignore`.

### 4. Próximos Passos Consistentens

#### A. Refinamento de Metadados de Regras
Atualmente, as regras são tuplas `( pattern, replacement, message )`. Seria mais robusto usar uma estrutura de dados nomeada (ex: `Rule` dataclass ou dicionário) para evitar confusão com a ordem dos parâmetros e facilitar a adição de flags opcionais por regra.

#### B. Expansão de Regras Comuns
Mover regras de `return_spacing` e `consecutive_newlines` que são idênticas semanticamente para `common_rules` na `base_verifier`, parametrizando apenas o que muda (ex: número de espaços no return).

#### C. Validação de Shebang Genérica
Melhorar `_validate_license` para lidar com shebangs de forma mais genérica, permitindo que cada verfificador defina uma lista de shebangs permitidos.