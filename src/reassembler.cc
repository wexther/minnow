#include "reassembler.hh"

using namespace std;

// 认为只有一个子串is_last_substring为true且没有任何字符越界
void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring )

{
  // Your code here.
  // (void)first_index;
  // (void)data;
  // (void)is_last_substring;

  Writer& stream_writer = output_.writer();
  if ( is_last_substring )
    end_index = first_index + data.length();

  if ( first_index + data.length() >= stream_writer.bytes_pushed()
       && stream_writer.bytes_pushed() + stream_writer.available_capacity() >= first_index ) {

    uint64_t avaliable_len = stream_writer.bytes_pushed() + stream_writer.available_capacity() - first_index;
    if ( data.length() > avaliable_len )
      data.erase( avaliable_len );

    if ( first_index <= stream_writer.bytes_pushed() ) {
      stream_writer.push( data.erase( 0, stream_writer.bytes_pushed() - first_index ) );

      auto insert_itr = store_map_.upper_bound( stream_writer.bytes_pushed() );
      if ( insert_itr != store_map_.begin() ) {
        --insert_itr;
        store_map_.erase( store_map_.begin(), insert_itr );
        stream_writer.push( insert_itr->second.erase( 0, stream_writer.bytes_pushed() - insert_itr->first ) );
        bytes_pending_ -= stream_writer.bytes_pushed() - insert_itr->first;
        store_map_.erase( insert_itr );
      }

      if ( stream_writer.bytes_pushed() == end_index )
        stream_writer.close();

    } else {
      auto cur = store_map_.find( first_index );
      if ( cur != store_map_.end() ) {
        if ( data.length() > cur->second.length() ) {
          bytes_pending_ += data.length() - cur->second.length();
          cur->second = data;
        }
      } else {
        auto insert_result = store_map_.insert( { first_index, data } );
        cur = insert_result.first;
        bytes_pending_ += data.length();
        if ( cur != store_map_.begin() ) {
          auto pre = cur;
          --pre;
          if ( pre->first + pre->second.length() >= first_index ) {
            uint64_t overlap_lenth = pre->first + pre->second.length() - first_index;
            if ( overlap_lenth < data.length() ) {
              bytes_pending_ -= overlap_lenth;
              pre->second.append( data, overlap_lenth );
            } else
              bytes_pending_ -= data.length();

            store_map_.erase( cur );
            cur = pre;
          }
        }
      }
      auto next = cur;
      ++next;
      while ( next != store_map_.end() && cur->first + cur->second.length() >= next->first ) {
        uint64_t overlap_lenth = cur->first + cur->second.length() - next->first;
        if ( overlap_lenth < next->second.length() ) {
          bytes_pending_ -= overlap_lenth;
          cur->second.append( next->second, overlap_lenth );
        } else
          bytes_pending_ -= next->second.length();

        store_map_.erase( next );
        next = cur;
        ++next;
      }
    }
  }
}

uint64_t Reassembler::bytes_pending() const
{
  // Your code here.
  // return {};
  return bytes_pending_;
}
