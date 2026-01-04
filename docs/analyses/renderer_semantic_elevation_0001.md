## Elevação Semântica 0001: Diagnóstico e Oportunidades Geométricas

Esta análise inicial foca nas oportunidades de redução de ruído visual e "maquinaria" algorítmica em `source/tui/renderer.cpp` através do uso da classe `sak::point`.

### 1. Diagnóstico: O Abismo entre o "O Quê" e o "Como"
Atualmente, o `renderer.cpp` opera em um nível predominantemente mecânico. Enquanto outros módulos ( como `game.cpp` ) já utilizam `sak::point` como entidade matemática, o renderizador frequentemente desmembra o ponto em coordenadas escalares para realizar cálculos manuais. Isso resulta em um vazamento de abstração invertido: o renderizador recria lógicas que deveriam ser propriedades inerentes da álgebra de vetores.

### 2. Oportunidades Identificadas

#### A. Bresenham Vetorial
No método `draw( const line& data )`, a extração manual de `x1, y1, x2, y2` obscurece o algoritmo, que é essencialmente uma operação de incremento vetorial.
- **Proposta:** Utilizar `point` para representar o `delta`, o `step` e a `position` atual.
- **Impacto:** Redução da manipulação de múltiplas variáveis escalares para operações diretas em objetos `point`.

#### B. Validação de Limites Espaciais
A verificação de limites em `draw( const point& pixel )` é feita via comparações escalares:
`if( x < 1 or x > width or y < 1 or y > 2 * height )`
- **Proposta:** Utilizar métodos como `pixel.is_between( min_bound, max_bound )`.
- **Impacto:** Elevação da intenção semântica para: "o pixel está dentro da área visível?".

#### C. Projeção de Buffer
O cálculo de índice `index = ( row - 1 ) * width + ( column - 1 )` é pura maquinaria de endereçamento.
- **Proposta:** Tratar o tamanho do terminal como um "passo" ( stride ) vetorial.
- **Impacto:** Abstração da conversão 2D para 1D, permitindo que o renderizador foque na geometria e não no layout de memória.

### 3. Expansões Necessárias em `sak::point`
Para viabilizar este amadurecimento, a classe `point` deve receber recursos matemáticos agnósticos:
1.  **Transformação Unária ( `map` ):** Aplicar funções ( como `abs` ou `sign` ) a todos os componentes.
2.  **Reduções Lógicas:** Inclusão de `is_between( )` e `clamp( )`.
3.  **Projeção Escalar:** Auxiliar para soma de produtos ( útil para índices de buffer ).


