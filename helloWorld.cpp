// The first example "Hello World"
// in Boost.Proto Users' Guide 1.57.0
//
// http://www.boost.org/doc/libs/1_57_0/doc/
// html/proto/
// users_guide.html#boost_proto.users_guide.getting_started.hello_world

#include <iostream>
#include <boost/proto/proto.hpp>
#include <boost/typeof/std/ostream.hpp>
using namespace boost;

proto::terminal< std::ostream & >::type cout_ = { std::cout };

template< typename Expr >
void evaluate( Expr const & expr )
{
    proto::default_context ctx;
    proto::eval(expr, ctx);
}

int main()
{
    evaluate( cout_ << "hello" << ',' << " world" );
    return 0;
}
