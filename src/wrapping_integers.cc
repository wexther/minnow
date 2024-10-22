#include "wrapping_integers.hh"

using namespace std;

Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  // Your code here.
  // (void)n;
  // (void)zero_point;
  // return Wrap32 { 0 };
  return zero_point + n;
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  // Your code here.
  // (void)zero_point;
  // (void)checkpoint;
  // return {};
  uint64_t ans = raw_value_ - zero_point.raw_value_ + ( checkpoint & 0xffffffff00000000 );
  if ( ans < checkpoint && checkpoint - ans > 0x80000000 ) {
    if ( ans >= 0xffffffff00000000 )
      throw( "wrap 2 uint64_t overflow" );
    ans += 0x100000000;
  } else if ( ans > checkpoint && ans - checkpoint > 0x80000000 && ans >= 0x100000000 )
    ans -= 0x100000000;
  return ans;
}
