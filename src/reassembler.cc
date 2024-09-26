#include "reassembler.hh"

using namespace std;

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring )
{
  // Your code here.
  // (void)first_index;
  // (void)data;
  // (void)is_last_substring;

  Writer stream_writer = writer();

  if ( first_index == first_unassembled_index ) {
    stream_writer.push( data );
    first_unassembled_index += data.length();

    if ( is_last_substring )
      stream_writer.close();
    else {

      while ( substring_map_.begin()->first == first_unassembled_index ) {
        stream_writer.push( data );
        first_unassembled_index += data.length();
      }
    }
  } else if ( first_index > first_unassembled_index
              && first_index < first_unassembled_index + stream_writer.available_capacity() ) {
  }
}

uint64_t Reassembler::bytes_pending() const
{
  // Your code here.
  return {};
}
