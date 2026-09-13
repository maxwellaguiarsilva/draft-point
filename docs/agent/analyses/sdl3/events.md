# Events (SDL3)

Subject markdown for the SDL3 event dispatching and loop architecture.

- Target hierarchy: `include/sak/sdl3/`
- Key dependencies: `sak::pattern::dispatcher`, `sak::pattern::listener_registry`, `sak::geometry` (`g2i`, `g2f`), `sak::sdl3::window`, `sak::sdl3::application`
- Scope: core architectural pattern for event consumption via OOP, RAII, and weak-reference dispatching.

## Overview & Architectural Principles

The SDL3 C API delivers events through a C union (`SDL_Event`) polled sequentially in a loop. Raw events present several architectural liabilities:
- **Primitive obsession:** Geometry data is packed into untyped integer or float fields (`data1`, `data2`, `x`, `y`) with meanings that change per event type.
- **Leaky abstractions:** Client code (such as games or tools) is forced to include C headers, write procedural `switch` statements, and manage event dispatch manually.
- **Coupling & lifetime risks:** Traditional callback systems often suffer from dangling pointers or tight coupling between event producers and consumers.

The `sak::sdl3` event architecture encapsulates the event loop and dispatching through:
1. **Geometry module alias:** Each class exposes the shared `sak::geometry` module through a single alias (`using geometry = ::sak::g2i;`) and references its named concepts (`geometry::position`, `geometry::size`, ...) directly, mirroring `tui::terminal` and `game::renderer`. Concepts are not redeclared per class.
2. **Weak-reference observer pattern:** Subscriptions rely on `sak::pattern::dispatcher< t_listener >`, guaranteeing thread safety, exception isolation, and automatic garbage collection of expired listener instances.
3. **Registrar split:** The host exposes `listeners( )` returning a `sak::pattern::listener_registry< t_listener >&`, which only permits registration. The derived `dispatcher` adds the typed `dispatch` used privately by the host.
4. **Reflected dispatch:** `dispatch< ^^listener::method >( arguments... )` resolves the listener slot by reflection and returns `void`. Listener failures are isolated per subscription and can be delivered to an optional error callback registered on the dispatcher at construction.
5. **Strict naming conventions:** No `on_`, `get_`, or `set_` prefixes. Listener methods reflect the action or state directly (`resize`, `move`, `close_requested`, `quit`, `key_down`).
6. **Nested listener interfaces:** Observers are declared as nested classes (e.g. `window::listener`, `application::listener`), mirroring the pattern established by `tui::renderer::listener`.

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

}
```

`geometry::position`, `geometry::size`, `geometry::line`, and `geometry::rectangle` are all derived from the same underlying `point`, so nothing else needs redeclaring. Integer and float geometry stay separate modules (`g2i` vs `g2f`), which makes any conversion between them an intentional, typed operation.

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
		virtual void resize( const geometry::size& ) { }
		virtual void pixel_resize( const geometry::size& ) { }
		virtual void move( const geometry::position& ) { }

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

		//	keyboard events
		virtual void key_down( const SDL_KeyboardEvent& ) { }
		virtual void key_up( const SDL_KeyboardEvent& ) { }
	};

	//	registration is exposed through the base registry; dispatch stays private to the host
	auto listeners( ) noexcept -> listener_registry< listener >& { return m_dispatcher; }

	auto dispatch( const SDL_WindowEvent& event ) -> void
	{
		switch( event.type )
		{
			case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
				m_dispatcher.dispatch< ^^listener::pixel_resize >( geometry::size{ event.data1, event.data2 } );
				break;
			case SDL_EVENT_WINDOW_RESIZED:
				m_dispatcher.dispatch< ^^listener::resize >( geometry::size{ event.data1, event.data2 } );
				break;
			case SDL_EVENT_WINDOW_MOVED:
				m_dispatcher.dispatch< ^^listener::move >( geometry::position{ event.data1, event.data2 } );
				break;

			case SDL_EVENT_WINDOW_SHOWN:			m_dispatcher.dispatch< ^^listener::show >( );			break;
			case SDL_EVENT_WINDOW_HIDDEN:			m_dispatcher.dispatch< ^^listener::hide >( );			break;
			case SDL_EVENT_WINDOW_MINIMIZED:		m_dispatcher.dispatch< ^^listener::minimize >( );		break;
			case SDL_EVENT_WINDOW_MAXIMIZED:		m_dispatcher.dispatch< ^^listener::maximize >( );		break;
			case SDL_EVENT_WINDOW_RESTORED:			m_dispatcher.dispatch< ^^listener::restore >( );		break;
			case SDL_EVENT_WINDOW_CLOSE_REQUESTED:	m_dispatcher.dispatch< ^^listener::close_requested >( );	break;
			case SDL_EVENT_WINDOW_FOCUS_GAINED:		m_dispatcher.dispatch< ^^listener::focus_gained >( );	break;
			case SDL_EVENT_WINDOW_FOCUS_LOST:		m_dispatcher.dispatch< ^^listener::focus_lost >( );		break;

			default:
				break;
		}
	}

	auto dispatch( const SDL_KeyboardEvent& event ) -> void
	{
		switch( event.type )
		{
			case SDL_EVENT_KEY_DOWN:	m_dispatcher.dispatch< ^^listener::key_down >( event );	break;
			case SDL_EVENT_KEY_UP:		m_dispatcher.dispatch< ^^listener::key_up >( event );		break;

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
	enum class flag : SDL_InitFlags
	{
		 audio		=	SDL_INIT_AUDIO
		,video		=	SDL_INIT_VIDEO
		,joystick	=	SDL_INIT_JOYSTICK
		,haptic		=	SDL_INIT_HAPTIC
		,gamepad	=	SDL_INIT_GAMEPAD
		,events		=	SDL_INIT_EVENTS
		,sensor		=	SDL_INIT_SENSOR
		,camera		=	SDL_INIT_CAMERA
	};

	using	init_flags	=	bitmask< flag >;

	explicit application( const init_flags flags = init_flags{ flag::video } )
		: m_flags( flags )
	{ ensure( SDL_Init( m_flags ), SDL_GetError( ) ); }

	class listener
	{
	public:
		virtual ~listener( ) = default;

		virtual void quit( ) { }
	};

	auto listeners( ) noexcept -> listener_registry< listener >& { return m_dispatcher; }

	auto poll( ) -> bool
	{
		SDL_Event event;
		while( SDL_PollEvent( &event ) )
		{
			if( event.type == SDL_EVENT_QUIT )
			{
				m_dispatcher.dispatch< ^^listener::quit >( );
				m_is_running = false;
				continue;
			}

			const bool is_window_event	=	between( event.type, SDL_EVENT_WINDOW_FIRST, SDL_EVENT_WINDOW_LAST );
			const bool is_key_event		=	between( event.type, SDL_EVENT_KEY_DOWN, SDL_EVENT_KEY_UP );

			if( is_window_event or is_key_event )
				if( auto* raw_window = SDL_GetWindowFromEvent( &event ) )
					if( auto* raw_instance = static_cast< window* >( SDL_GetPointerProperty( SDL_GetWindowProperties( raw_window ), "sak.sdl3.window", nullptr ) ) )
					{
						//	reference alias keeps the dispatch body free of pointer noise
						auto& window_instance	=	*raw_instance;
						if( is_window_event )
							window_instance.dispatch( event.window );
						else
							window_instance.dispatch( event.key );
					}
		}

		return	m_is_running;
	}

	auto run( const function< void( ) >& frame_action ) -> void
	{
		while( poll( ) )
			frame_action( );
	}

	auto quit( ) noexcept -> void { m_is_running = false; }

private:
	init_flags				m_flags;
	dispatcher< listener >	m_dispatcher;
	bool					m_is_running{ true };
};

}
```

Window and keyboard events are routed to the owning `sak::sdl3::window` through the `"sak.sdl3.window"` pointer property set at construction, so every window instance dispatches to its own listeners regardless of which window produced the event.

---

## Consumer Pattern (Developer Experience)

The application developer focuses strictly on domain logic, inheriting from listener interfaces without touching SDL internals or raw C structures:

```cpp
__using_inline( window::geometry::, width, height )

class scene_controller final
	: public window::listener
	, public application::listener
{
public:
	using	geometry	=	window::geometry;

	explicit scene_controller( application& target_application )
		: m_application( target_application )
	{ }

	void resize( const geometry::size& size ) override
	{
		gl_viewport( 0, 0, width( size ), height( size ) );
	}

	void key_down( const SDL_KeyboardEvent& event ) override
	{
		if( event.key == SDLK_ESCAPE )
			m_application.quit( );
	}

	void close_requested( ) override
	{
		m_application.quit( );
	}

	void quit( ) override
	{
		//	persist save data or clean up external resources
	}

private:
	application&	m_application;
};
```

Wiring observers in `main` through the `listeners( )` registry:

```cpp
auto main( ) -> int
{
	__using( ::sak::, exit_success )
	__using( ::sak::sdl3::, application, window )
	__using( ::std::, make_shared )

	application app;
	window main_window( "demo", { 800, 600 } );

	auto controller = make_shared< scene_controller >( app );
	main_window.listeners( ) += controller;
	app.listeners( ) += controller;

	app.run( [ & ]( )
	{
		//	render frame
		main_window.swap( );
	} );

	return	exit_success;
}
```
