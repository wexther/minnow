#include "tcp_receiver.hh"
#include <algorithm>
#include <cstdint>
#include <stdexcept>

using namespace std;

void TCPReceiver::receive( TCPSenderMessage message )
{
  // Your code here.
  // (void)message;

  if ( message.RST ) {
    reassembler_.reader().set_error();
    return;
  }

  if ( SYN_flag_ ) {
    reassembler_.insert(
      message.seqno.unwrap( ISN_, reassembler_.writer().bytes_pushed() + 1 ) - 1, message.payload, message.FIN );
  } else if ( message.SYN ) {
    SYN_flag_ = true;
    ISN_ = message.seqno;
    reassembler_.insert( 0, message.payload, message.FIN );
  }
}

TCPReceiverMessage TCPReceiver::send() const
{
  // Your code here.
  // return {};

  TCPReceiverMessage message;

  if ( SYN_flag_ ) {
    message.ackno
      = Wrap32::wrap( reassembler_.writer().bytes_pushed() + 1 + reassembler_.writer().is_closed(), ISN_ );
  }

  message.window_size = min( reassembler_.writer().available_capacity(), static_cast<uint64_t> UINT16_MAX );

  message.RST = reassembler_.writer().has_error();
  return message;
}
