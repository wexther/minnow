#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ) {}

bool Writer::is_closed() const
{
  // Your code here.
  // return {};

  return closed_;
}

void Writer::push( string data )
{
  // Your code here.
  // (void)data;
  // return;

  if ( closed_ ) {
    return;
  }

  uint64_t push_lenth = min( data.length(), available_capacity() );
  stream_buffer_.append( data, 0, push_lenth );
  bytes_pushed_ += push_lenth;
}

void Writer::close()
{
  // Your code here.

  closed_ = true;
  return;
}

uint64_t Writer::available_capacity() const
{
  // Your code here.
  // return {};

  return capacity_ - stream_buffer_.length();
}

uint64_t Writer::bytes_pushed() const
{
  // Your code here.
  // return {};

  return bytes_pushed_;
}

bool Reader::is_finished() const
{
  // Your code here.
  // return {};

  return stream_buffer_.empty() && closed_;
}

uint64_t Reader::bytes_popped() const
{
  // Your code here.
  // return {};

  return bytes_pushed_ - stream_buffer_.length();
}

string_view Reader::peek() const
{
  // Your code here.
  // return {};

  return string_view( stream_buffer_ );
}

void Reader::pop( uint64_t len )
{
  // Your code here.
  // (void)len;

  stream_buffer_.erase( 0, len );
}

uint64_t Reader::bytes_buffered() const
{
  // Your code here.
  // return {};

  return stream_buffer_.length();
}
