# Bash Coding Style Guidelines

Este documento herda a filosofia, as regras de nomenclatura (`snake_case`) e de espaçamento (` ( ) `, ` [ ] `) definidas em [GEMINI.md](../../../GEMINI.md).

## 1. Adaptações do GEMINI.md
*	**Comentários:** Utilize `#` seguido de **TAB**. O conteúdo deve ser estritamente em letras minúsculas, sem ponto final e com uma frase por linha.
*	**Espaçamento:** Aplica-se a `[[ ... ]]`, `( ... )`, `[ ... ]`, `$( ... )` e `$(( ... ))`.
*	**Strings:** Literais devem ser em minúsculas e sem ponto final.
*	**Indentação:** Uso obrigatório de **tabs**.

## 2. Regras Específicas de Bash
*	**Variáveis:** Use `local` dentro de funções. Use `declare` para arrays (`-A` ou `-a`).
*	**Expansões:** Sempre utilize aspas duplas e chaves: `"${variable_name}"`.
*	**Funções:** Definidas como `name( )` com a chave de abertura na linha seguinte.
*	**Controle de Fluxo:** `then` ou `do` devem estar sempre em uma nova linha.

## 3. Ferramentas e Validação
*	Validação obrigatória com `shellcheck` antes de considerar a tarefa concluída.