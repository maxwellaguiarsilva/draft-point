#include <sak/pattern/dispatcher.hpp>
#include <memory>


namespace sak {
namespace pattern {


__using( ::std::, make_shared, shared_ptr )


struct mock_listener
{
	bool called = false;
	void on_event( ) { called = true; }
};


void test_dispatcher_basic_notification( )
{
	dispatcher< mock_listener > l_dispatcher;
	auto l_listener = make_shared< mock_listener >( );
	
	l_dispatcher += l_listener;
	auto l_result = l_dispatcher( &mock_listener::on_event );
	
	sak::ensure( l_result.has_value( ) );
	sak::ensure( l_listener->called );
}


} }


int main( )
{{
	::sak::pattern::test_dispatcher_basic_notification( );
	return 0;
}}