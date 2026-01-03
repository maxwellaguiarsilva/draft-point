### 1. C++ Coding Style (Hard Rules)
- **Modernity:** C++23 project. Prefer `ranges`, `views`, and `Niebloids`.
- **Naming:** Always `snake_case`. Class members prefixed with `m_`.
- **Logical Operators:** Use of `&&, ||, !` is forbidden. Mandatory use of `and, or, not`.
- **Spacing:** Mandatory internal spacing in `( )` and `[ ]`. E.g.: `if( condition )`, `array[ index ]`.
- **Flow Control:** No braces `{}` for single-line `if, for, while`.
- **Increment:** Always prefer pre-increment (`++i`) and pre-decrement (`--i`).
- **Main Function:** Only place where double braces are mandatory: `{{ ... }}`.
- **Headers:** Namespaces declared as `namespace n {` on the same line. No extra indentation for namespace content.
- **Indirection:** Prefer `const&` (constant reference) to avoid `->` operator noise. Use `using` to avoid the scope operator `::` (except for ADL-sensitive functions like `move` or `forward`).
- **Methods:** Mark methods that do not change state with `const` and `noexcept`.
- **Allowed Comments:** Only to explain subtle behaviors or in tests.
    - Must start with `//` followed by a **TAB** (`	`).
    - Written entirely in lowercase, without a trailing period. Each phrase on a new line.

### 2. Semantic Elevation (Philosophy)
- **Data over Machinery:** Hide the "machinery" (pointers, manual loops) to let the "data" speak.
- **Visual Serenity:** Reduce visual noise. Code should be a statement of intent ("what"), not a CPU instruction manual ("how").
- **Com composição:** Se uma lógica pode ser nomeada, ela deve ser um utilitário na biblioteca `sak`. Evite lambdas ad-hoc; prefira compor entidades nomeadas.

### 4. Fluxo de Trabalho e Ferramentas (Workflow)
- **Automação (MCP):** Use sempre `create_class` e `create_test`. O código gerado por essas ferramentas é o **Padrão Ouro**.
- **Mimetismo:** Observe a estrutura, a ordem de includes e os comentários gerados pelos templates e mimetizá-los rigorosamente. Não altere o boilerplate automático.
- **Testes:** Use sempre `sak::ensure` em vez de `cassert`.
- **Verificação:** 
    1.  Não compile até ser solicitado. Use a ferramenta `compile` para build.
    2.  Use a ferramenta `check` (cppcheck) após alterações e trate todos os avisos.
    3.  Use a ferramenta `fix_format` (obrigatório para manter a consistência do código).

### 5. Biblioteca `sak` (Swiss Army Knife)
- Generic and domain-independent utilities (math, geometry, patterns).
- Must not contain business logic or hardware dependencies.