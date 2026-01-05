## Log de Falha Técnica: Problemas de Implementação

Este documento registra falhas técnicas, omissões de informação ou imprevistos que surgiram durante a tentativa de execução dos blueprints, servindo de contexto para o amadurecimento da análise.

### Registro 0001: 2026-01-05
**Falha:** Inconsistência na definição de tipos e ambiguidades de operadores.

**Contexto Técnico:**
1.  **Redefinição de Símbolo:** O uso de `__using( ::sak, ::point )` em conjunto com `using point = cell;` em `tui/geometry.hpp` causou erro de redefinição. A análise deve especificar o uso de nomes qualificados.
2.  **Ambiguidade de `operator ==`:** A introdução de tags no `sak::point` causou ambiguidades na comparação entre `pixel` e `cell` devido à herança de `std::array`. A análise deve prever como tratar essas comparações (ex: conversões explícitas ou sobrecargas específicas).
3.  **Ambiguidade de Domínio:** A distinção entre geometria de pixels e geometria de células no terminal não estava clara, causando erros de tipo no `terminal.cpp` e `renderer.cpp` pela falta de transformadores explícitos.
4.  **Static Analysis (`cppcheck`):** O `cppcheck` exigiu construtores `explicit` e sugeriu funções `static`, o que não foi previsto na estrutura do blueprint original.

**Ação Necessária:**
Refatorar os arquivos `0004.md`, `0005.md` e `0006.md` para endereçar esses pontos antes de qualquer nova tentativa de execução.


