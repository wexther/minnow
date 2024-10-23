#include "wrapping_integers.hh"
#include <stdexcept>
using namespace std;

Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  // Your code here.
  // (void)n;
  // (void)zero_point;
  // return Wrap32 { 0 };

  return zero_point + n;
}

// 当返回值溢出时抛出错误
uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  // Your code here.
  // (void)zero_point;
  // (void)checkpoint;
  // return {};

  uint64_t ans = raw_value_ - zero_point.raw_value_ + ( checkpoint & 0xffffffff00000000 );
  int64_t judge = ans - checkpoint;
  if ( judge < -0x80000000LL ) {
    // if ( ans >= 0xffffffff00000000 )
    //   throw runtime_error( "wrap 2 uint64_t overflow" );
    ans += 0x100000000;
  } else if ( judge > 0x80000000 && ans >= 0x100000000 ) {
    ans -= 0x100000000;
  }
  return ans;
}
