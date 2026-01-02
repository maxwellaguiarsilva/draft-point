## 🛠 Protocolo de Operação (Draft-Point)

Este arquivo é a fonte única de verdade para o comportamento do Agente. As instruções aqui são imperativas e priorizadas sobre qualquer outro documento.

### 1. Diretrizes de Comunicação e Identidade
- **Idioma:** Conversa em `pt-br`. Código, comentários e mensagens de commit estritamente em `en-us`.
- **Qualidade:** Erros de gramática ou digitação são proibidos. Se um nome for semanticamente incorreto, aponte-o imediatamente.
- **Interação:** Siga as solicitações de forma objetiva, realizando exatamente o que foi pedido sem análises de contexto desnecessárias.

### 2. Estilo de Código C++ (Hard Rules)
- **Modernidade:** Projeto C++23. Prefira `ranges`, `views` e `Niebloids`.
- **Naming:** Sempre `snake_case`. Membros de classe prefixados com `m_`.
- **Operadores Lógicos:** Proibido o uso de `&&, ||, !`. Use obrigatoriamente `and, or, not`.
- **Espaçamento:** Espaço interno obrigatório em `( )` e `[ ]`. Ex: `if( condition )`, `array[ index ]`.
- **Controle de Fluxo:** Sem chaves `{}` para `if, for, while` de uma única linha.
- **Incremento:** Prefira sempre pré-incremento (`++i`) e pré-decremento (`--i`).
- **Main Function:** Único lugar onde chaves duplas são obrigatórias: `int main( ) {{ ... }}`.
- **Headers:** Namespaces declarados como `namespace n {` na mesma linha. Sem indentação extra para o conteúdo do namespace.
- **Indireção:** Prefira `const&` (referência constante) para evitar o ruído do operador `->`. Use `using` para evitar o operador de escopo `::` (exceto para funções sensíveis a ADL como `move` ou `forward`).
- **Métodos:** Marque métodos que não alteram o estado com `const` e `noexcept`.
- **Comentários Permitidos:** Apenas para explicar comportamentos sutis ou em testes.
    - Devem começar com `//` seguido de um **TAB** (`	`).
    - Escritos inteiramente em minúsculas, sem ponto final. Cada frase em uma linha nova.

### 3. Elevação Semântica (Filosofia)
- **Data over Machinery:** Oculte a "maquinaria" (ponteiros, loops manuais) para deixar o "dado" falar.
- **Visual Serenity:** Reduza o ruído visual. O código deve ser uma declaração de intenção ("o que"), não um manual de instruções para a CPU ("como").
- **Composição:** Se uma lógica pode ser nomeada, ela deve ser um utilitário na biblioteca `sak`. Evite lambdas ad-hoc; prefira compor entidades nomeadas.

### 4. Fluxo de Trabalho e Ferramentas (Workflow)
- **Automação (MCP):** Use sempre `create_class` e `create_test`. O código gerado por essas ferramentas é o **Padrão Ouro**.
- **Mimetismo:** O Agente deve observar a estrutura, a ordem de includes e os comentários gerados pelos templates e mimetizá-los rigorosamente. Não altere o boilerplate automático.
- **Testes:** Use sempre `sak::ensure` em vez de `cassert`.
- **Verificação:** 
    1.  Não compile até ser solicitado. Use a ferramenta `compile` para build.
    2.  Use a ferramenta `check` (cppcheck) após alterações e trate todos os avisos.
    3.  Use a ferramenta `fix_newlines` (obrigatório 2 linhas vazias no fim de arquivos `.cpp` e `.hpp`).

### 5. Biblioteca `sak` (Swiss Army Knife)
- Utilitários genéricos e independentes de domínio (matemática, geometria, patterns).
- Não deve conter lógica de negócio ou dependências de hardware.
