#include "tcp_sender.hh"
#include "tcp_config.hh"

using namespace std;

uint64_t TCPSender::sequence_numbers_in_flight() const
{
  // Your code here.
  // return {};

  return next_seq_ - acked_no_;
}

uint64_t TCPSender::consecutive_retransmissions() const
{
  // Your code here.
  // return {};

  return consecutive_retransmissions_;
}

void TCPSender::push( const TransmitFunction& transmit )
{
  // Your code here.
  // (void)transmit;

  uint16_t not_acked_size = next_seq_ - acked_no_;

  if ( swnd_ == 0 && not_acked_size == 0 ) {
    send_msg_with( 1, next_seq_, transmit );
  } else {
    while ( not_acked_size < swnd_ && !input_.reader().peek().empty() ) {
      send_msg_with( swnd_ - not_acked_size, next_seq_, transmit );
      not_acked_size = next_seq_ - acked_no_;
    }
    if ( not_acked_size < swnd_ && input_.reader().peek().empty()
         && ( next_seq_ == 0 || ( input_.reader().is_finished() && !FIN_sent ) ) ) {
      send_msg_with( swnd_ - not_acked_size, next_seq_, transmit );
    }
  }
}

TCPSenderMessage TCPSender::make_empty_message() const
{
  // Your code here.
  // return {};

  return TCPSenderMessage( Wrap32::wrap( next_seq_, isn_ ), {}, {}, {}, input_.has_error() );
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  // Your code here.
  // (void)msg;

  if ( msg.ackno.has_value() ) {
    uint64_t new_acked_no_ = msg.ackno.value().unwrap( isn_, acked_no_ );
    map<uint64_t, TCPSenderMessage>::iterator it = tcp_buffer_.find( new_acked_no_ );
    if ( it != tcp_buffer_.end() ) {
      acked_no_ = new_acked_no_;

      timer_.set_RTO( initial_RTO_ms_ );
      if ( it != tcp_buffer_.begin() ) {
        timer_.start();
      }

      consecutive_retransmissions_ = 0;
      tcp_buffer_.erase( tcp_buffer_.begin(), it );
    } else if ( new_acked_no_ == next_seq_ ) {
      acked_no_ = new_acked_no_;

      timer_.set_RTO( initial_RTO_ms_ );
      timer_.stop();

      consecutive_retransmissions_ = 0;
      tcp_buffer_.erase( tcp_buffer_.begin(), it );
    }
  }
  swnd_ = msg.window_size;
  if ( msg.RST ) {
    input_.set_error();
  }
}

void TCPSender::tick( uint64_t ms_since_last_tick, const TransmitFunction& transmit )
{
  // Your code here.
  // (void)ms_since_last_tick;
  // (void)transmit;
  // (void)initial_RTO_ms_;

  if ( timer_.expire_with_time_goes( ms_since_last_tick ) ) {
    transmit( tcp_buffer_[acked_no_] );
    if ( swnd_ != 0 ) {
      ++consecutive_retransmissions_;
      timer_.double_RTO();
    }
    timer_.start();
  }
}

// my addition
void TCPSender::send_msg_with( uint16_t msg_max_length, uint64_t abs_sqeno, const TransmitFunction& transmit )
{
  TCPSenderMessage msg;

  Reader& reader = input_.reader();
  msg.seqno = Wrap32::wrap( abs_sqeno, isn_ );

  if ( abs_sqeno == 0 ) {
    msg.SYN = true;
  }

  uint16_t msg_length = min( static_cast<size_t>( msg_max_length - msg.SYN ), TCPConfig::MAX_PAYLOAD_SIZE );
  msg_length = min( msg_length, static_cast<uint16_t>( reader.peek().length() ) );
  msg.payload = reader.peek().substr( 0, msg_length );
  reader.pop( msg_length );

  if ( msg_length < msg_max_length - msg.SYN && reader.is_finished() && !FIN_sent ) {
    msg.FIN = true;
    FIN_sent = true;
  }
  msg.RST = input_.has_error();

  transmit( msg );

  tcp_buffer_[abs_sqeno] = msg;
  next_seq_ += msg.sequence_length();
  if ( !timer_.is_running() ) {
    timer_.start();
  }
}

bool Timer::expire_with_time_goes( uint64_t time_ms )
{
  current_timestamp_ += time_ms;
  return current_timestamp_ >= expire_timestamp_;
}
