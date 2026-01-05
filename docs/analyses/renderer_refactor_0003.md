## Refatoração do Renderizador 0003: Segurança de Tipo e Iteração Espacial

Padrões idiomáticos e de segurança que garantem a integridade do design elevado.

### 1. Segurança de Domínio via Tipagem Forte
Para impedir o uso acidental de coordenadas de pixel em funções que esperam células, utilizamos *tags* de tipo aninhadas na classe `renderer`:
- `using pixel = ::sak::point< int, 2, struct pixel_tag >;`
- `using cell  = ::sak::point< int, 2, struct cell_tag >;`

Esta distinção em tempo de compilação força o uso de transformadores explícitos, tornando o código auto-documentado e robusto contra erros de lógica espacial.

### 2. Iteração com Consciência Espacial
A iteração sobre os buffers de exibição é elevada para um modelo de "dados com contexto":
```cpp
//	iteração elevada: sem índices manuais, apenas dados e posição
for( auto&& [ position, back_cell ] : m_back.elements( ) )
{
    auto& front_cell = m_front[ position ];
    if( back_cell not_eq front_cell )
    {
        m_terminal.move_cursor( position );
        m_terminal.draw_cell( back_cell );
        front_cell = back_cell;
    }
}
```
O método `.elements( )` encapsula a derivação da posição, permitindo que o `renderer` foque puramente na política de sincronização.

### 3. Geometria como Fluxo ( Generators )
Algoritmos complexos como o Bresenham são removidos do corpo do renderizador e transformados em geradores de pontos:
```cpp
void renderer::draw( line const& data )
{
    for( pixel pixel : geometry::trace_line( data.start, data.end ) )
        draw( pixel );
}
```
A infraestrutura `geometry::trace_line` fornece uma `std::ranges::view`, tratando a geometria como um fluxo de dados.

### 4. Orquestração e Maturidade
O renderizador atua como um orquestrador de políticas de exibição. A complexidade matemática e de acesso reside em bibliotecas testadas ( `sak` e `geometry` ), resultando em um código focado na intenção de design.


