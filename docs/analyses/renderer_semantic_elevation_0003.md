## Elevação Semântica v3: Modelagem Algébrica e Abstração Funcional

Este documento detalha a implementação técnica das abstrações discutidas para o `renderer.cpp`, movendo a lógica de "rastreamento de estado" para "funções de posição".

### 1. Mapeamento de Domínio: Espaço de Pixels vs. Espaço de Células
O renderizador TUI lida com dois espaços distintos que hoje estão misturados. A elevação exige uma tradução clara:

- **Espaço de Pixels ( Geometria ):** Coordenadas `pixel[ 0 ]` ( 1..width ) e `pixel[ 1 ]` ( 1..2*height ).
- **Espaço de Células ( Terminal ):** Coordenadas `cell[ 0 ]` ( 1..width ) e `cell[ 1 ]` ( 1..height ).

**Fórmulas de Tradução:**
```cpp
point cell_position = { pixel[ 0 ], ( pixel[ 1 ] + 1 ) / 2 };
bool is_upper_pixel = ( pixel[ 1 ] % 2 not_eq 0 );
```
Esta separação permite que o `draw( point )` se torne um despachante semântico, eliminando cálculos "inline" de `row` e `column`.

### 2. Eliminação de Estado no `refresh( )`
Atualmente, o `refresh( )` rastreia `row` e `column` manualmente. A elevação funcional propõe derivar a posição do cursor diretamente do índice do buffer, tratando o movimento como uma função do espaço:

**Posição como Função do Índice:**
Dado um `index` no buffer linear e a `width` do terminal:
- `column = ( index % width ) + 1`
- `row    = ( index / width ) + 1`

Isso permite que o loop de `zip( m_back, m_front )` utilize um enumerador ou contador simples para determinar a posição do cursor de forma declarativa, eliminando os incrementos manuais e o risco de erros *off-by-one* no controle de fluxo.

### 3. Aritmética de Direção ( Bresenham Vetorial )
O algoritmo de Bresenham pode ser totalmente despojado de sua "maquinaria" de escalares ( `x_start, y_start`, etc. ) ao utilizar as novas capacidades da `sak::point`:

**Derivação de Vetores de Controle:**
- `delta = ( data.end - data.start ).map( sak::math::abs )`
- `step  = ( data.end - data.start ).map( sak::math::sign )`

O loop de desenho passa a operar sobre um único objeto `point current_position`, cujos incrementos são decididos pela álgebra vetorial dos erros, mantendo o código imune à quantidade de dimensões e focado na intenção de "caminhar no espaço".

### 4. Requisitos de Infraestrutura em `sak::math`
Para viabilizar as transformações acima sem vazar lógica de domínio, `sak::math` deve fornecer funções puras ( Niebloids ) compatíveis com `sak::point::map`:

- **`sak::math::sign( )`:** Retorna -1, 0 ou 1.
- **`sak::math::abs( )`:** Retorna o valor absoluto.
- **`sak::math::projection( )`:** Especialização de `sum( a * b )` para clareza em linearização de buffers.

### 5. Conclusão: Visual Serenity via Álgebra
Ao adotar estas fórmulas, o arquivo `renderer.cpp` deixa de ser uma sequência de instruções de "como manipular o cursor e o buffer" para se tornar uma declaração de "como os espaços geométricos se relacionam com a superfície de exibição". A maquinaria é substituída por relações matemáticas.


