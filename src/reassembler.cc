#include "reassembler.hh"
#include <stdexcept>

using namespace std;

// 认为有唯一的字符串末且没有任何字符越界，否则抛出错误
void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring )
{
  // Your code here.
  // (void)first_index;
  // (void)data;
  // (void)is_last_substring;

  Writer& stream_writer = output_.writer();
  if ( stream_writer.has_error() ) {
    return;
  }

  if ( is_last_substring && end_index == UINT64_MAX ) {
    end_index = first_index + data.length();
  }

  // 子串可能存在需要处理的信息
  first_unassambled_index_ = stream_writer.bytes_pushed();
  uint64_t legal_capacity = first_unassambled_index_ + stream_writer.available_capacity();

  if ( first_index + data.length() >= first_unassambled_index_ && legal_capacity >= first_index ) {
    data.erase( min( legal_capacity - first_index, data.length() ) );

    // 直接插入ByteStream
    if ( first_index <= first_unassambled_index_ ) {
      stream_writer.push( data.erase( 0, first_unassambled_index_ - first_index ) );
      first_unassambled_index_ = stream_writer.bytes_pushed();

      if ( first_unassambled_index_ > end_index ) {
        stream_writer.set_error();
        throw runtime_error( "Reassmembler Bytes overpush" );
      } else if ( first_unassambled_index_ == end_index ) {
        stream_writer.close();
      } else {
        map<uint64_t, string>::iterator insert_itr = store_map_.upper_bound( first_unassambled_index_ );

        if ( insert_itr != store_map_.begin() ) {
          --insert_itr;

          bytes_pending_ -= insert_itr->second.length();
          stream_writer.push( insert_itr->second.erase( 0, first_unassambled_index_ - insert_itr->first ) );
          first_unassambled_index_ = stream_writer.bytes_pushed();

          if ( first_unassambled_index_ == end_index ) {
            stream_writer.close();
          } else if ( first_unassambled_index_ > end_index ) {
            stream_writer.set_error();
            throw runtime_error( "Reassmembler Bytes overpush" );
          }

          for ( map<uint64_t, string>::iterator it = store_map_.begin(); it != insert_itr; it++ ) {
            bytes_pending_ -= it->second.length();
            store_map_.erase( it );
          }
          store_map_.erase( insert_itr );
        }
      }
    } else { // 储存
      pair<map<uint64_t, string>::iterator, bool> insert_result = store_map_.insert( { first_index, data } );
      map<uint64_t, string>::iterator cur = insert_result.first;
      if ( insert_result.second ) {
        bytes_pending_ += data.length();

        if ( cur != store_map_.begin() ) {
          map<uint64_t, string>::iterator pre = cur;
          --pre;
          if ( pre->first + pre->second.length() >= first_index ) {
            map_merge( pre, cur );
          }
        }
      } else {
        if ( data.length() > cur->second.length() ) {
          bytes_pending_ += data.length() - cur->second.length();
          cur->second = data;
        }
      }

      map<uint64_t, string>::iterator next = cur;
      ++next;
      while ( next != store_map_.end() && cur->first + cur->second.length() >= next->first ) {
        map_merge( cur, next );
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

inline void Reassembler::map_merge( map<uint64_t, string>::iterator& cur, map<uint64_t, string>::iterator& next )
{
  uint64_t overlap_lenth = cur->first + cur->second.length() - next->first;
  if ( overlap_lenth < next->second.length() ) {
    bytes_pending_ -= overlap_lenth;
    cur->second.append( next->second, overlap_lenth );
  } else {
    bytes_pending_ -= next->second.length();
  }

  store_map_.erase( next );
  next = cur;
}
