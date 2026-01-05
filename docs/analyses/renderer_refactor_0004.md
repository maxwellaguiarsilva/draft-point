## Refatoração do Renderizador 0004: Blueprint de Implementação

Este documento é o guia de execução mecânica para a refatoração do sistema de renderização. Nenhuma decisão de design deve ser tomada durante a implementação; siga as assinaturas e lógicas descritas.

### 1. Infraestrutura: Elevação da `sak::point`
**Arquivo:** `include/sak/geometry/point.hpp`

**Alteração 1: Adição de Tag de Domínio**
Modificar a assinatura da classe para aceitar um terceiro parâmetro de template:
```cpp
template< is_arithmetic t_scalar = int, size_t num_dimensions = 2, typename t_tag = void >
class point final : private array< t_scalar, num_dimensions >
```
Atualizar o trait `__is_point` e a concept `is_point` para serem variádicos ou ignorarem a tag na verificação de "point-ness".

**Alteração 2: Método `map`**
Adicionar o método `map` para transformações unárias:
```cpp
__using( ::std::ranges, ::transform );

template< typename t_operation >
constexpr auto map( t_operation&& operation ) const noexcept
{
	point< decltype( operation( t_scalar{ } ) ), num_dimensions, t_tag > result;
	transform( *this, result.begin( ), operation );
	return	result;
}
```

### 2. Definições de Domínio TUI
**Arquivo:** `include/tui/geometry.hpp`

Substituir as definições atuais de `point` por tipos tagueados e adicionar transformadores:
```cpp
namespace tui {

__using( ::sak, ::point );
__using( ::sak::math, ::is_odd );

struct pixel_tag;
struct cell_tag;

using	pixel	=	point< int, 2, pixel_tag >;
using	cell	=	point< int, 2, cell_tag >;

//	transformadores de domínio
inline constexpr auto to_cell( pixel const& position ) noexcept -> cell
{
	return	cell{ position[ 0 ], ( position[ 1 ] + 1 ) / 2 };
}

inline constexpr auto is_upper( pixel const& position ) noexcept -> bool
{
	return	is_odd( position[ 1 ] );
}

}
```

### 3. Abstração de Acesso: `surface_view`
**Arquivo:** `include/tui/surface.hpp` ( Novo Arquivo )

Implementar uma visão que mapeia coordenadas 2D para um buffer linear:
```cpp
namespace tui {

template< typename t_data, typename t_point_type >
class surface_view
{
public:
	surface_view( t_data* data, t_point_type size ) 
		: m_data( data ), m_size( size ) { }

	auto operator []( t_point_type const& position ) noexcept -> t_data&
	{
		//	linearização elevada: vertical * width + horizontal
		return	m_data[ ( position[ 1 ] - 1 ) * m_size[ 0 ] + ( position[ 0 ] - 1 ) ];
	}

private:
	t_data*      m_data;
	t_point_type m_size;
};

}
```

### 4. Orquestração no Renderizador
**Arquivo:** `source/tui/renderer.cpp`

**Alteração 1: Método `draw( pixel const& )`**
Remover cálculos manuais de índice. Utilizar transformadores e `surface_view`.
```cpp
void renderer::draw( pixel const& position ) noexcept
{
	__using( ::std, ::lock_guard );

	auto lock = lock_guard( m_mutex );
	if( not is_inside( position ) ) return; //	implementar is_inside com base em m_terminal_size

	auto target_cell = to_cell( position );
	auto& data = m_back_view[ target_cell ]; //	m_back_view deve ser membro da classe

	if( is_upper( position ) )
		data.up = m_color;
	else
		data.down = m_color;
}
```

**Alteração 2: Ciclo de Sincronização (`refresh`)**
Eliminar contadores `row`/`column`. Utilizar um iterador espacial ( a ser implementado na `surface_view` ou via gerador ).

### 5. Geometria como Fluxo
**Arquivo:** `include/tui/geometry.hpp`

Adicionar o gerador `trace_line` utilizando o algoritmo de Bresenham, retornando um `std::ranges::view` de `pixel`.
O executor deve mover a lógica atual de `renderer::draw( line )` para esta função.
