## Elevação Semântica 0002: Fronteiras de Abstração e Projeção Espacial

Esta análise aprofunda a separação de responsabilidades entre a geometria pura ( `sak::point` ) e o mecanismo de exibição ( `tui::renderer` ), focando na modelagem do espaço.

### 1. Responsabilidade da Linearização
A conversão de coordenadas multidimensionais para um índice linear de buffer é uma preocupação de domínio do renderizador. 
- **Entendimento:** `sak::point` deve permanecer agnóstico ao layout de memória para evitar a "Contaminação de Domínio".
- **Abordagem Elevada:** O renderizador deve utilizar a álgebra de pontos fornecida pela `sak` para realizar a linearização de forma declarativa.

### 2. Projeção Escalar como Linguagem de Buffer
A indexação do buffer pode ser expressa como uma Projeção Escalar, elevando a semântica do código:
```cpp
//	a maquinaria de buffer expressa através de álgebra vetorial
point strides = { 1, m_terminal_size[ 0 ] };
point local_position = cell_position - 1;
int index = sak::math::sum( local_position * strides ); 
```
Neste modelo, o `point` fornece a ferramenta algébrica e o renderizador fornece o contexto ( strides ).

### 3. O Conceito de Superfície ( Surface Accessor )
Identificamos a oportunidade de criar uma abstração de "Superfície" ou "Grid" dentro do domínio TUI.
- **Papel:** Atuar como o tradutor final entre o espaço geométrico e o espaço de memória.
- **Interface:** Oferecer acesso via `operator [ ]( point )`, permitindo que o renderizador ignore completamente a aritmética de ponteiros.

### 4. Requisitos Refinados
Para suportar esta camada de abstração, a infraestrutura deve evoluir em:
- **`sak::math`:** Adição de `sign( )` e compatibilidade total de `sum( )` com expressões de pontos.
- **`sak::point`:** Implementação de `is_between( )` e suporte a transformações via `map( )`.


