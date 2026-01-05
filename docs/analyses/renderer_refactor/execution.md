## Guia de Execução Mecânica: Refatoração do Renderizador

Este documento contém instruções estritamente operacionais. Siga cada passo sem interpretar o contexto ou a lógica.

### Passo 1: Matemática Base
**Arquivo:** `include/sak/math/math.hpp`

No final do arquivo, antes do fechamento do namespace `sak::math`, adicione ( se ainda não existirem ):
```cpp
struct __sign
{
	constexpr auto operator ( ) ( auto value ) const noexcept { return ( value > 0 ) - ( value < 0 ); }
};
inline constexpr auto sign = __sign( );

struct __abs
{
	constexpr auto operator ( ) ( auto value ) const noexcept { return value < 0 ? -value : value; }
};
inline constexpr auto abs = __abs( );
```

### Passo 2: Infraestrutura de Geometria
**Arquivo:** `include/sak/geometry/point.hpp`

1.  **Assinatura da Classe:** Altere para:
```cpp
template< is_arithmetic t_scalar = int, size_t num_dimensions = 2, typename t_tag = void >
class point final : private array< t_scalar, num_dimensions >
```

2.  **Trait e Concept:** Atualize para suportar `t_tag`:
```cpp
template< is_arithmetic t_scalar, size_t num_dimensions, typename t_tag >
struct __is_point< point< t_scalar, num_dimensions, t_tag > > : true_type { };
```

3.  **Construtores:** Use inicialização por chaves `{ }` para o `super_type`:
```cpp
template< typename... t_args >
	requires( sizeof...( t_args ) == num_dimensions
		and ( ( is_arithmetic< t_args > or convertible_to< t_args, t_scalar > ) and ... )
	)
constexpr explicit point( t_args... a_args )
	: super_type{ static_cast< t_scalar >( a_args )... }
{
}

template< typename t_other_tag >
constexpr explicit point( const point< t_scalar, num_dimensions, t_other_tag >& other ) noexcept
{
	for( size_type index = 0; index < num_dimensions; ++index )
		( *this )[ index ] = other[ index ];
}
```

4.  **Operadores de Comparação:** Substitua o `operator ==` atual:
```cpp
friend constexpr auto operator == ( const point& left, const point& right ) noexcept -> bool 
{ 
	return	left.is_all( right, equal_to ); 
}

friend constexpr auto operator != ( const point& left, const point& right ) noexcept -> bool 
{ 
	return	not ( left == right ); 
}
```

### Passo 3: Definições de Domínio TUI
**Arquivo:** `include/tui/geometry.hpp`

Substitua todo o conteúdo após as macros de include por:
```cpp
#include <generator>

namespace tui {

namespace pixel {
	using point     = ::sak::point< int, 2, struct pixel_tag >;
	using geometry  = ::sak::geometry< point >;
	using line      = geometry::line;
	using rectangle = geometry::rectangle;
}

namespace cell {
	using point     = ::sak::point< int, 2, struct cell_tag >;
}

inline constexpr auto to_cell( pixel::point const& position ) noexcept -> cell::point
{
	return	cell::point( position[ 0 ], ( position[ 1 ] + 1 ) / 2 );
}

inline constexpr auto is_upper( pixel::point const& position ) noexcept -> bool
{
	__using( ::sak::math, ::is_odd );
	return	is_odd( position[ 1 ] );
}

namespace pixel {
	auto trace_line( point start, point end ) -> ::std::generator< point >;
}

} 
```

### Passo 4: Implementação do Gerador
**Arquivo:** `source/tui/geometry.cpp`

Implemente o `trace_line` usando `__using` para evitar ruído de escopo:
```cpp
namespace tui::pixel {

auto trace_line( point start, point end ) -> ::std::generator< point >
{
	__using( ::sak::math, ::abs, ::sign );

	point difference = ( end - start ).map( abs );
	point direction_step = ( end - start ).map( sign );
	
	int error = difference[ 0 ] - difference[ 1 ];
	point current = start;

	while( true )
	{
		co_yield current;
		if( current == end ) break;
		
		int error_doubled = 2 * error;
		if( error_doubled > -difference[ 1 ] )
		{
			error -= difference[ 1 ];
			current[ 0 ] += direction_step[ 0 ];
		}
		if( error_doubled < difference[ 0 ] )
		{
			error += difference[ 0 ];
			current[ 1 ] += direction_step[ 1 ];
		}
	}
}

}
```

### Passo 5: Visão de Superfície
**Arquivo:** `include/tui/surface.hpp`

```cpp
#pragma once
#include <sak/using.hpp>
#include <ranges>
#include <utility>

namespace tui {

template< typename t_container, typename t_point >
class surface_view
{
public:
	surface_view( t_container& container, t_point size )
		: m_container( container ), m_size( size ) { }

	auto operator []( t_point const& position ) noexcept -> auto&
	{
		return	m_container[ ( position[ 1 ] - 1 ) * m_size[ 0 ] + ( position[ 0 ] - 1 ) ];
	}

	auto elements( )
	{
		__using( ::std::views, ::iota, ::transform );
		__using( ::std, ::pair );

		return	iota( 0, static_cast< int >( m_container.size( ) ) )
			| transform( [ this ] ( int index ) {
				t_point position{ ( index % m_size[ 0 ] ) + 1, ( index / m_size[ 0 ] ) + 1 };
				return	pair< t_point, typename t_container::value_type& >( position, m_container[ index ] );
			} );
	}

private:
	t_container& m_container;
	t_point      m_size;
};

}
```

### Passo 6: Alinhamento do Terminal
**Arquivos:** `include/tui/terminal.hpp` e `source/tui/terminal.cpp`

Substitua toda ocorrência de `point` por `cell::point` nas assinaturas de métodos que lidam com coordenadas do terminal.

### Passo 7: Membros do Renderizador
**Arquivo:** `include/tui/renderer.hpp`

1.  Inclua `#include <optional>`.
2.  Atualize os métodos `draw`:
```cpp
void draw( pixel::point const& data ) noexcept;
void draw( pixel::line const& data );
void draw( pixel::rectangle const& data, bool fill = true );
```
3.  Na seção `private`, use `std::optional` para as visões:
```cpp
cell::point	m_terminal_size;
::std::optional< surface_view< buffer, cell::point > > m_front_view;
::std::optional< surface_view< buffer, cell::point > > m_back_view;
```

### Passo 8: Lógica do Renderizador
**Arquivo:** `source/tui/renderer.cpp`

1.  **No `on_resize`:**
```cpp
m_terminal_size = size;
m_front.assign( size[ 0 ] * size[ 1 ], { 0, 0 } );
m_back.assign( size[ 0 ] * size[ 1 ], { 0, 0 } );

m_front_view.emplace( m_front, size );
m_back_view.emplace(  m_back,  size );
```

2.  **No `draw( pixel::point const& position )`:**
```cpp
if( not is_inside( position ) ) return; 

auto target_cell = to_cell( position );
auto& data = ( *m_back_view )[ target_cell ]; 

if( is_upper( position ) )
	data.up = m_color;
else
	data.down = m_color;
```

3.  **No `refresh( )`:**
```cpp
for( auto&& [ position, back_cell ] : m_back_view->elements( ) )
{
	auto& front_cell = ( *m_front_view )[ position ];
	if( back_cell not_eq front_cell )
	{
		m_terminal.move_cursor( position );
		//	... manter lógica de escrita da célula ...
		front_cell = back_cell;
	}
}
```

4.  **No `is_inside`:**
```cpp
auto renderer::is_inside( pixel::point const& position ) const noexcept -> bool
{
	return	position[ 0 ] >= 1 and position[ 0 ] <= m_terminal_size[ 0 ] 
	   and position[ 1 ] >= 1 and position[ 1 ] <= 2 * m_terminal_size[ 1 ];
}
```