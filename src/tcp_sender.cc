#include "tcp_sender.hh"
#include "tcp_config.hh"

using namespace std;

uint64_t TCPSender::sequence_numbers_in_flight() const
{
  // Your code here.
  // return {};

  return next_seq_ - ackno_;
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

  Reader& reader = input_.reader();
  while ( next_seq_ - ackno_ < swnd_ && next_seq_ - ackno_ < reader.bytes_buffered()) {
    TCPSenderMessage msg;
    msg.seqno = Wrap32::wrap( next_seq_, isn_ );

    uint16_t msg_length = min( swnd_ - next_seq_ + ackno_, static_cast<uint64_t>( TCPConfig::MAX_PAYLOAD_SIZE ) );
    if ( swnd_ == 0 ) {
      msg_length = 1;
    }

    msg.payload = reader.peek().substr( next_seq_ - ackno_, msg_length );
    if ( next_seq_ == 0 ) {
      msg.SYN = true;
    }

    next_seq_ += msg.sequence_length();
  }
}

TCPSenderMessage TCPSender::make_empty_message() const
{
  // Your code here.
  return {};
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  // Your code here.
  (void)msg;
}

void TCPSender::tick( uint64_t ms_since_last_tick, const TransmitFunction& transmit )
{
  // Your code here.
  (void)ms_since_last_tick;
  (void)transmit;
  (void)initial_RTO_ms_;
}

// my addition
void Timer::start()
{
  expire_timestamp_ = current_timestamp_ + RTO_ms_;
  is_running_ = true;
}

void Timer::stop()
{
  expire_timestamp_ = UINT64_MAX;
  is_running_ = false;
}

bool Timer::expire_with_time_goes( uint64_t time_ms )
{
  current_timestamp_ += time_ms;
  return current_timestamp_ > expire_timestamp_;
}
