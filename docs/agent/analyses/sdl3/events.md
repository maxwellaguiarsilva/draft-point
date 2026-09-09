# Events (SDL3)

Subject markdown for the SDL3 event dispatching and loop architecture.

- Target hierarchy: `include/sak/sdl3/`
- Key dependencies: `sak::pattern::dispatcher`, `sak::geometry` (`g2i`, `g2f`), `sak::sdl3::window`, `sak::sdl3::application`
- Scope: core architectural pattern for event consumption via OOP, RAII, and weak-reference dispatching.

## Overview & Architectural Principles

The SDL3 C API delivers events through a C union (`SDL_Event`) polled sequentially in a loop. Raw events present several architectural liabilities:
- **Primitive obsession:** Geometry data is packed into untyped integer or float fields (`data1`, `data2`, `x`, `y`) with meanings that change per event type.
- **Leaky abstractions:** Client code (such as games or tools) is forced to include C headers, write procedural `switch` statements, and manage event dispatch manually.
- **Coupling & lifetime risks:** Traditional callback systems often suffer from dangling pointers or tight coupling between event producers and consumers.

The `sak::sdl3` event architecture encapsulates the event loop and dispatching through:
1. **Geometry module alias:** Each class exposes the shared `sak::geometry` module through a single alias (`using geometry = ::sak::g2i;`) and references its named concepts (`geometry::position`, `geometry::size`, ...) directly, mirroring `tui::terminal` and `game::renderer`. Concepts are not redeclared per class.
2. **Weak-reference observer pattern:** Subscriptions rely on `sak::pattern::dispatcher< t_listener >`, guaranteeing thread safety, exception isolation, and automatic garbage collection of expired listener instances.
3. **Strict naming conventions:** No `on_`, `get_`, or `set_` prefixes. Listener methods reflect the action or state directly (`resize`, `move`, `close_requested`, `quit`).
4. **Nested listener interfaces:** Observers are declared as nested classes (e.g. `window::listener`, `application::listener`), mirroring the pattern established by `tui::renderer::listener`.

---

## Geometry Alias

Geometry concepts are not redeclared as per-concept aliases (`window::size`, `window::position`, ...). Each owner instead exposes the whole integer (or float) geometry module through one alias, exactly as `tui::terminal` and `game::renderer` already do:

```cpp
namespace sak::sdl3 {

class window
{
public:
	using	geometry	=	::sak::g2i;
	//	...
};

class mouse
{
public:
	using	geometry	=	::sak::g2f;
	//	...
};

}
```

`geometry::position`, `geometry::size`, `geometry::line`, and `geometry::rectangle` are all derived from the same underlying `point`, so nothing else needs redeclaring. Integer and float geometry stay separate modules (`g2i` vs `g2f`), which keeps mouse space distinct from the window grid and makes any conversion between them an intentional, typed operation.

---

## Window Events

### Nested Interface: `window::listener`

The interface provides default empty implementations (no-op). Observers override only the events relevant to their domain:

```cpp
namespace sak::sdl3 {

class window
{
public:
	using	geometry	=	::sak::g2i;

	class listener
	{
	public:
		virtual ~listener( ) = default;

		//	geometry events
		virtual void resize( const geometry::size& new_size ) { }
		virtual void pixel_resize( const geometry::size& pixel_size ) { }
		virtual void move( const geometry::position& new_position ) { }

		//	visibility and state transitions
		virtual void show( ) { }
		virtual void hide( ) { }
		virtual void minimize( ) { }
		virtual void maximize( ) { }
		virtual void restore( ) { }
		virtual void close_requested( ) { }

		//	focus transitions
		virtual void focus_gained( ) { }
		virtual void focus_lost( ) { }
	};

	//	subscription operator matching the project convention
	void operator +=( const shared_ptr< listener >& subject )
	{
		m_dispatcher += subject;
	}

	auto dispatch( const SDL_WindowEvent& event ) -> void
	{
		switch( event.type )
		{
			case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
                ( void )m_dispatcher( &listener::pixel_resize, geometry::size{ event.data1, event.data2 } );
                break;
			case SDL_EVENT_WINDOW_RESIZED:
                ( void )m_dispatcher( &listener::resize, geometry::size{ event.data1, event.data2 } );
                break;
			case SDL_EVENT_WINDOW_MOVED:
                ( void )m_dispatcher( &listener::move, geometry::position{ event.data1, event.data2 } );
                break;

			case SDL_EVENT_WINDOW_SHOWN:            ( void )m_dispatcher( &listener::show );            break;
			case SDL_EVENT_WINDOW_HIDDEN:           ( void )m_dispatcher( &listener::hide );            break;
			case SDL_EVENT_WINDOW_MINIMIZED:        ( void )m_dispatcher( &listener::minimize );        break;
			case SDL_EVENT_WINDOW_MAXIMIZED:        ( void )m_dispatcher( &listener::maximize );        break;
			case SDL_EVENT_WINDOW_RESTORED:         ( void )m_dispatcher( &listener::restore );         break;
			case SDL_EVENT_WINDOW_CLOSE_REQUESTED:  ( void )m_dispatcher( &listener::close_requested ); break;
			case SDL_EVENT_WINDOW_FOCUS_GAINED:     ( void )m_dispatcher( &listener::focus_gained );    break;
			case SDL_EVENT_WINDOW_FOCUS_LOST:       ( void )m_dispatcher( &listener::focus_lost );      break;

			default:
				break;
		}
	}

private:
	dispatcher< listener >	m_dispatcher;
};

}
```

---

## Application Events

### Nested Interface: `application::listener`

Events belonging to the overall application lifecycle (such as quit signals) belong to `application::listener`:

```cpp
namespace sak::sdl3 {

__using( ::sak::math::, between )

class application
{
public:
	class listener
	{
	public:
		virtual ~listener( ) = default;
		virtual void quit( ) { }
	};

	void operator +=( const shared_ptr< listener >& subject )
	{
		m_dispatcher += subject;
	}

	auto poll( ) -> bool
	{
		SDL_Event event;
		while( SDL_PollEvent( &event ) )
		{
			if( event.type == SDL_EVENT_QUIT )
			{
				( void )m_dispatcher( &listener::quit );
				m_is_running = false;
				continue;
			}

			if( between( event.type, SDL_EVENT_WINDOW_FIRST, SDL_EVENT_WINDOW_LAST ) )
				if( auto* raw_window = SDL_GetWindowFromEvent( &event ) )
					if( auto* raw_instance = static_cast< window* >( SDL_GetPointerProperty( SDL_GetWindowProperties( raw_window ), "sak.sdl3.window", nullptr ) ) )
					{
						//	reference alias keeps the dispatch body free of `->` noise (see style-guide indirection)
						auto& window_instance	=	*raw_instance;	//	non-null guaranteed in this scope
						window_instance.dispatch( event.window );
					}
	}

		return	m_is_running;
	}

	//	possible alternative (user to evaluate): single-return helper that resolves the null case once,
	//	binding a reference at the call site instead of an intermediate variable per dispatch site
	//	auto& window_of( SDL_Window* handle )
	//	{
	//		void* p	=	SDL_GetPointerProperty( SDL_GetWindowProperties( handle ), "sak.sdl3.window", nullptr );
	//		ensure( p not_eq nullptr, "window not owned by sak" );
	//		return	*static_cast< window* >( p );
	//	}
	//	//	usage:	window_of( raw_window ).dispatch( event.window );

	auto stop( ) noexcept -> void { m_is_running = false; }

private:
	dispatcher< listener >	m_dispatcher;
	bool					m_is_running{ true };
};

}
```

---

## Consumer Pattern (Developer Experience)

The application developer focuses strictly on domain logic, inheriting from listener interfaces without touching SDL internals or raw C structures:

```cpp
__using_constexpr( window::geometry::, width, height )

class scene_controller final
	: public window::listener
	, public application::listener
{
public:
	using	geometry	=	window::geometry;

	explicit scene_controller( application& app )
		: m_app( app )
	{ }

	void resize( const geometry::size& size ) override
	{
		gl_viewport( 0, 0, width( size ), height( size ) );
	}

	void close_requested( ) override
	{
		m_app.stop( );
	}

	void quit( ) override
	{
		//	persist save data or clean up external resources
	}

private:
	application&	m_app;
};
```

Wiring observers in `main`:

```cpp
auto main( ) -> int
{
	__using( ::sak::, exit_success )
	__using( ::sak::sdl3::, application, window )
	__using( ::std::, make_shared )

	application app;
	window main_window( "demo", 800, 600 );

	auto controller = make_shared< scene_controller >( app );
	main_window += controller;
	app += controller;

	while( app.poll( ) )
	{
		//	render frame
		main_window.swap( );
	}

	return	exit_success;
}
```
