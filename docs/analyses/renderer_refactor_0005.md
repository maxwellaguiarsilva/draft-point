## Refatoração do Renderizador 0005: Detalhamento Algorítmico e Sincronização

Este documento fornece as implementações exatas para a fase final da refatoração. O executor deve copiar os trechos de código para os locais indicados.

### 1. Extensões na `sak`
**Arquivo:** `include/sak/math/math.hpp`
Adicionar após o `sum`:
```cpp
struct __sign
{
	constexpr auto operator ( ) ( auto value ) const noexcept { return ( value > 0 ) - ( value < 0 ); }
};
inline constexpr auto sign = __sign{ };
```

**Arquivo:** `include/sak/geometry/point.hpp`
Adicionar o construtor de conversão de tag ( explícito para evitar ambiguidades em comparações ):
```cpp
template< typename t_other_tag >
constexpr explicit point( const point< t_scalar, num_dimensions, t_other_tag >& other ) noexcept
{
	for( size_type index = 0; index < num_dimensions; ++index )
		( *this )[ index ] = other[ index ];
}
```

### 2. A Classe `surface_view` e Iteração Espacial
**Arquivo:** `include/tui/surface.hpp`
A `surface_view` deve prover acesso e um iterador que retorna `( posição, dado )`.

```cpp
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
			| transform( [ this ]( int index ) {
				t_point position{ ( index % m_size[ 0 ] ) + 1, ( index / m_size[ 0 ] ) + 1 };
				return	pair< t_point, typename t_container::value_type& >{ position, m_container[ index ] };
			} );
	}

private:
	t_container& m_container;
	t_point      m_size;
};

}
```

### 3. Gerador de Linha ( Bresenham Elevado )
**Arquivo:** `include/tui/geometry.hpp`
Implementação de `trace_line` como um gerador de `vector`.

```cpp
inline auto trace_line( pixel start, pixel end ) -> ::std::vector< pixel >
{
	__using( ::std, ::vector );
	__using( ::sak::math, ::abs, ::sign );

	vector< pixel > pixels;
	pixel difference = ( end - start ).map( abs );
	pixel direction_step = ( end - start ).map( sign );
	
	int error = difference[ 0 ] - difference[ 1 ];
	pixel current = start;

	while( true )
	{
		pixels.push_back( current );
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
	return	pixels;
}
```

### 4. Refatoração do `renderer::refresh`
**Arquivo:** `source/tui/renderer.cpp`
O novo loop de atualização utiliza `m_back_view.elements( )`.

```cpp
void renderer::refresh( )
{{
	if( m_is_resizing.load( ) ) return;

	__using( ::std, ::unique_lock, ::try_to_lock );

	unique_lock lock( m_mutex, try_to_lock );
	if( not lock.owns_lock( ) ) return;

	for( auto&& [ position, back_cell ] : m_back_view.elements( ) )
	{
		auto& front_cell = m_front[ ( position[ 1 ] - 1 ) * m_terminal_size[ 0 ] + ( position[ 0 ] - 1 ) ];
		if( back_cell not_eq front_cell )
		{
			m_terminal.move_cursor( position );
			//	lógica de cores e print...
			front_cell = back_cell;
		}
	}
}}
```
**Nota:** O executor deve garantir que `m_back_view` seja re-inicializado no `on_resize`.


