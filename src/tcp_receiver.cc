#include "tcp_receiver.hh"
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <stdexcept>

using namespace std;

void TCPReceiver::receive( TCPSenderMessage message )
{
  // Your code here.
  // (void)message;
  // cout << message.RST << "rst" << endl;

  if ( RST_flag_ || message.RST || reassembler_.writer().has_error() ) {
    RST_flag_ = true;
    reassembler_.reader().set_error();
    // cout << "set1!" << endl;
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

  if ( reassembler_.writer().available_capacity() > UINT16_MAX ) {
    message.window_size = UINT16_MAX;
  } else {
    message.window_size = reassembler_.writer().available_capacity();
  }
  
  message.RST = RST_flag_ || reassembler_.writer().has_error();
  return message;
}
