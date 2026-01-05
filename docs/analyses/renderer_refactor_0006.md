## Refatoração do Renderizador 0006: Finalização do Blueprint e Estrutura de Dados

Este documento encerra o detalhamento técnico, focando nas dependências matemáticas e na estrutura da classe `renderer`.

### 1. Complemento Matemático
**Arquivo:** `include/sak/math/math.hpp`
Adicionar o operador `abs` para suportar o mapeamento de pontos:
```cpp
struct __abs
{
	constexpr auto operator ( ) ( auto value ) const noexcept { return value < 0 ? -value : value; }
};
inline constexpr auto abs = __abs{ };
```

### 2. Estrutura da Classe `renderer`
**Arquivo:** `include/tui/renderer.hpp`

O executor deve atualizar a seção `private` e incluir o novo header:
```cpp
#include <tui/surface.hpp>
// ...
private:
	terminal&	m_terminal;
	buffer		m_front;
	buffer		m_back;
	cell_point	m_terminal_size;
	
	//	novas visões de superfície
	surface_view< buffer, cell_point > m_front_view;
	surface_view< buffer, cell_point > m_back_view;
```

### 3. Implementação dos Métodos de Desenho Elevados
**Arquivo:** `source/tui/renderer.cpp`

**draw( pixel_line const& data )**
```cpp
void renderer::draw( pixel_line const& data )
{
	for( pixel const& position : trace_line( data.start, data.end ) )
		draw( position );
}
```

**draw( pixel_rectangle const& data, bool fill )**
```cpp
void renderer::draw( pixel_rectangle const& data, bool fill )
{
	//	utilizar a lógica de iteração sobre pixels do retângulo,
	//	chamando draw( pixel{ horizontal, vertical } ) para cada ponto.
}
```

### 4. Verificação de Segurança ( is_inside )
Adicionar como método privado em `renderer`:
```cpp
auto renderer::is_inside( pixel const& position ) const noexcept -> bool
{
	return	position[ 0 ] >= 1 and position[ 0 ] <= m_terminal_size[ 0 ] 
	   and position[ 1 ] >= 1 and position[ 1 ] <= 2 * m_terminal_size[ 1 ];
}
```

### Conclusão do Blueprint
Com a conclusão deste documento, todas as peças da "maquinaria" foram substituídas por abstrações semânticas. O executor deve agora aplicar as mudanças na ordem:
1. `sak/math/math.hpp`
2. `sak/geometry/point.hpp`
3. `tui/geometry.hpp`
4. `tui/surface.hpp` ( criação )
5. `tui/renderer.hpp`
6. `tui/renderer.cpp`


