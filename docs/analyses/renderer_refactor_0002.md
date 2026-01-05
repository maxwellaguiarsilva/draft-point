## Refatoração do Renderizador 0002: Arquitetura de Espaços e Superfícies

Esta análise é sobre a modelagem técnica das transformações de domínio e a abstração de acesso ao buffer.

### 1. Mapeamento de Domínio: Pixels vs. Células
O renderizador opera em dois espaços sobrepostos que exigem tradução explícita:
- **Espaço de Pixels ( Geometria ):** 1..width x 1..2*height.
- **Espaço de Células ( Terminal ):** 1..width x 1..height.

A tradução deve ser feita por transformadores de domínio, eliminando cálculos inline:
```cpp
point cell_position = to_cell_space( pixel_position );
bool is_upper = ( pixel_position[ 1 ] % 2 not_eq 0 );
```

### 2. A Abstração `surface_view`
Em vez de manipular buffers brutos, o renderizador utiliza uma `surface_view`.
- **Semântica de Acesso:** `buffer[ position ]` substitui `buffer[ y * width + x ]`.
- **Evolução do `draw( pixel )`:** O método torna-se um despachante semântico que identifica a célula e o sub-pixel ( up/down ) correspondente, delegando a complexidade de índice para a `surface_view`.

### 3. Eliminação de Estado e Posicionamento Declarativo
O movimento do cursor durante o `refresh( )` deixa de ser controlado por incrementos manuais de `row` e `column`.
- **Derivação Espacial:** A posição no terminal é derivada diretamente do índice do buffer ou de funções de espaço ( `space::from_index` ).
- **Impacto:** Eliminação de erros *off-by-one* e garantia de que a lógica de desenho é imune a mudanças na ordem de iteração do buffer.

### 4. Estado da Abstração de Acesso
Com a `surface_view` e os transformadores de domínio, o `renderer.cpp` deixa de ser um "calculador de endereços" para se tornar um gestor de relações espaciais.


