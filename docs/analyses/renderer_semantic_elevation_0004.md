## Elevação Semântica 0004: A Abstração da Superfície e a Geometria de Acesso

Esta análise aprofunda a evolução do `renderer.cpp`, transicionando de um manipulador de buffers para um gestor de superfícies geométricas e eliminando a aritmética de ponteiros do nível de aplicação.

### 1. A Entidade "Superfície" ( Surface )
O `renderer` gerencia a complexidade de buffers e cálculos de índices lineares. A elevação em curso estabelece uma visão ( view ) que trata o buffer como uma entidade bidimensional nativa.

- **Conceito:** Uma `surface_view` que encapsula o gerenciamento de memória e as dimensões ( strides ).
- **Interface:** `cell& operator [ ]( point const& position )`.
- **Semântica:** `buffer[ { x, y } ]` em vez de `buffer[ y * width + x ]`.

### 2. Aritmética de Transformação de Espaço
A manipulação da "frequência espacial" dobrada no eixo Y ( 2 pixels por célula ) deixa de ser resolvida com operações `inline` e passa a ser delegada a funções de transformação de domínio.

**Transformadores de Domínio:**
- `to_cell_space( point const& pixel ) -> point`: Aplica `{ pixel[ 0 ], ( pixel[ 1 ] + 1 ) / 2 }`.
- `get_sub_cell_index( point const& pixel ) -> size_t`: Retorna 0 ( up ) ou 1 ( down ).

Dessa forma, o método `draw( point const& pixel )` torna-se puramente declarativo:
```cpp
auto cell_position = to_cell_space( pixel );
auto& cell = m_back_surface[ cell_position ];
if( is_upper( pixel ) )
    cell.up = m_color;
else
    cell.down = m_color;
```

### 3. O Fim do Rastreamento de Cursor no `refresh( )`
A sincronização do estado do terminal com o buffer evolui para que a posição seja uma propriedade intrínseca do índice, removendo incrementos manuais de linha e coluna.

```cpp
for( auto index : ::std::ranges::views::iota( 0, count ) )
{
    point position = space::from_index( index, width );
    if( back[ index ] not_eq front[ index ] )
    {
        m_terminal.move_to( position );
        //	... draw ...
    }
}
```
Isso garante imunidade a mudanças na ordem de iteração e elimina erros de borda ( off-by-one ).

### 4. Requisitos para `sak::point` e `sak::math`
Para suportar Bresenham e transformações sem ruído:
- **`sak::point::map( mapping_function )`**: Essencial para `delta = ( end_point - start_point ).map( math::abs )`.
- **`sak::math::sign` e `sak::math::abs`**: Implementados como Niebloids para composição via `map`.
- **`sak::point::is_between( minimum, maximum )`**: Para validações de domínio simplificadas.

### 5. Conclusão: O Renderer como Orquestrador
Com estas mudanças, o `renderer.cpp` consolida-se como um executor de "políticas de exibição". A complexidade matemática reside na `sak` e a complexidade de acesso na abstração de `surface`, permitindo que o sistema escale em funcionalidades e robustez, mantendo a clareza e a previsibilidade técnica em cada nova camada de implementação.


