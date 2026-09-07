# Events (SDL3)

Subject markdown for the SDL3 event dispatching and loop architecture.

- Target hierarchy: `include/sak/sdl3/`
- Key dependencies: `sak::pattern::dispatcher`, `sak::geometry::point`, `sak::sdl3::window`, `sak::sdl3::application`
- Scope: core architectural pattern for event consumption via OOP, RAII, and weak-reference dispatching.

## Overview & Architectural Principles

The SDL3 C API delivers events through a C union (`SDL_Event`) polled sequentially in a loop. Raw events present several architectural liabilities:
- **Primitive obsession:** Geometry data is packed into untyped integer or float fields (`data1`, `data2`, `x`, `y`) with meanings that change per event type.
- **Type mismatch across sub-systems:** Window events deal in discrete integer pixels (`Sint32`), while mouse/touch events operate in continuous subpixel coordinates (`float`). Generic vector aliases (`vec2`) risk type dilution or accidental mixing.
- **Leaky abstractions:** Client code (such as games or tools) is forced to include C headers, write procedural `switch` statements, and manage event dispatch manually.
- **Coupling & lifetime risks:** Traditional callback systems often suffer from dangling pointers or tight coupling between event producers and consumers.

The `sak::sdl3` event architecture encapsulates the event loop and dispatching through:
1. **Scoped domain aliases:** Each class defines its own strongly typed semantics (`window::size`, `window::position`, `mouse::position`, `mouse::motion`), avoiding primitive obsession and making coordinate conversions explicit.
2. **Weak-reference observer pattern:** Subscriptions rely on `sak::pattern::dispatcher< t_listener >`, guaranteeing thread safety, exception isolation, and automatic garbage collection of expired listener instances.
3. **Strict naming conventions:** No `on_`, `get_`, or `set_` prefixes. Listener methods reflect the action or state directly (`resize`, `move`, `close_requested`, `quit`).
4. **Nested listener interfaces:** Observers are declared as nested classes (e.g. `window::listener`, `application::listener`), mirroring the pattern established by `tui::renderer::listener`.

---

## Scoped Type Aliases

Instead of a single global vector type, semantic aliases live inside their respective owner classes:

```cpp
namespace sak::sdl3 {

class window
{
public:
	using	size		=	::sak::g2i::point;
	using	position	=	::sak::g2i::point;
	//	...
};

class mouse
{
public:
	using	position	=	::sak::g2f::point;
	using	motion		=	::sak::g2f::point;
	//	...
};

}
```

This guarantees:
- **Discreteness where required:** Window boundaries and buffer sizes remain integer points (`g2i::point`).
- **Precision where required:** Mouse cursor coordinates and high-frequency delta movements preserve subpixel float precision (`g2f::point`).
- **Explicit conversions:** Converting between mouse space and window grid requires an intentional, typed conversion instead of implicit arithmetic errors.

---

## Window Events

### Nested Interface: `window::listener`

The interface provides default empty implementations (no-op). Observers override only the events relevant to their domain:

```cpp
namespace sak::sdl3 {

class window
{
public:
	using	size		=	::sak::g2i::point;
	using	position	=	::sak::g2i::point;

	class listener
	{
	public:
		virtual ~listener( ) = default;

		//	geometry events with strongly typed domain aliases
		virtual void resize( const size& /*new_size*/ ) { }
		virtual void pixel_resize( const size& /*pixel_size*/ ) { }
		virtual void move( const position& /*new_position*/ ) { }

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
			case SDL_EVENT_WINDOW_RESIZED:
				( void )m_dispatcher( &listener::resize, size{ event.data1, event.data2 } );
				break;

			case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
				( void )m_dispatcher( &listener::pixel_resize, size{ event.data1, event.data2 } );
				break;

			case SDL_EVENT_WINDOW_MOVED:
				( void )m_dispatcher( &listener::move, position{ event.data1, event.data2 } );
				break;

			case SDL_EVENT_WINDOW_SHOWN:
				( void )m_dispatcher( &listener::show );
				break;

			case SDL_EVENT_WINDOW_HIDDEN:
				( void )m_dispatcher( &listener::hide );
				break;

			case SDL_EVENT_WINDOW_MINIMIZED:
				( void )m_dispatcher( &listener::minimize );
				break;

			case SDL_EVENT_WINDOW_MAXIMIZED:
				( void )m_dispatcher( &listener::maximize );
				break;

			case SDL_EVENT_WINDOW_RESTORED:
				( void )m_dispatcher( &listener::restore );
				break;

			case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
				( void )m_dispatcher( &listener::close_requested );
				break;

			case SDL_EVENT_WINDOW_FOCUS_GAINED:
				( void )m_dispatcher( &listener::focus_gained );
				break;

			case SDL_EVENT_WINDOW_FOCUS_LOST:
				( void )m_dispatcher( &listener::focus_lost );
				break;

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

			if( event.type >= SDL_EVENT_WINDOW_FIRST and event.type <= SDL_EVENT_WINDOW_LAST )
				if( auto* raw_window = SDL_GetWindowFromEvent( &event ) )
					if( auto* window_instance = static_cast< window* >( SDL_GetPointerProperty( SDL_GetWindowProperties( raw_window ), "sak.window", nullptr ) ) )
						window_instance->dispatch( event.window );
		}

		return	m_is_running;
	}

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
class scene_controller final
	: public window::listener
	, public application::listener
{
public:
	explicit scene_controller( application& app )
		: m_app( app )
	{ }

	void resize( const window::size& new_size ) override
	{
		gl_viewport( 0, 0, new_size[ 0 ], new_size[ 1 ] );
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
