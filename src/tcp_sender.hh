#pragma once

#include "byte_stream.hh"
#include "tcp_receiver_message.hh"
#include "tcp_sender_message.hh"

#include <cstdint>
#include <functional>
// #include <list>
#include <map>
// #include <memory>
#include <optional>
// #include <queue>

// my addition
class Timer
{
public:
  explicit Timer( uint64_t RTO_ms ) : RTO_ms_( RTO_ms ) {}
  void start() { expire_timestamp_ = current_timestamp_ + RTO_ms_; }
  void stop() { expire_timestamp_ = UINT64_MAX; }
  void set_RTO( uint64_t RTO_ms ) { RTO_ms_ = RTO_ms; }
  void double_RTO() { RTO_ms_ *= 2; }
  bool expire_with_time_goes( uint64_t time_ms );
  bool is_running() const { return expire_timestamp_ != UINT64_MAX; }

private:
  uint64_t RTO_ms_ {};
  uint64_t current_timestamp_ {};
  uint64_t expire_timestamp_ { UINT64_MAX };
};

class TCPSender
{
public:
  /* Construct TCP sender with given default Retransmission Timeout and possible ISN */
  TCPSender( ByteStream&& input, Wrap32 isn, uint64_t initial_RTO_ms )
    : input_( std::move( input ) ), isn_( isn ), initial_RTO_ms_( initial_RTO_ms ), timer_( initial_RTO_ms )
  {}

  /* Generate an empty TCPSenderMessage */
  TCPSenderMessage make_empty_message() const;

  /* Receive and process a TCPReceiverMessage from the peer's receiver */
  void receive( const TCPReceiverMessage& msg );

  /* Type of the `transmit` function that the push and tick methods can use to send messages */
  using TransmitFunction = std::function<void( const TCPSenderMessage& )>;

  /* Push bytes from the outbound stream */
  void push( const TransmitFunction& transmit );

  /* Time has passed by the given # of milliseconds since the last time the tick() method was called */
  void tick( uint64_t ms_since_last_tick, const TransmitFunction& transmit );

  // Accessors
  uint64_t sequence_numbers_in_flight() const;  // How many sequence numbers are outstanding?
  uint64_t consecutive_retransmissions() const; // How many consecutive *re*transmissions have happened?
  Writer& writer() { return input_.writer(); }
  const Writer& writer() const { return input_.writer(); }

  // Access input stream reader, but const-only (can't read from outside)
  const Reader& reader() const { return input_.reader(); }

private:
  // Variables initialized in constructor
  ByteStream input_;
  Wrap32 isn_;
  uint64_t initial_RTO_ms_;
  // my addition
  Timer timer_;
  uint64_t acked_no_ {};
  uint64_t next_seq_ {};
  uint16_t swnd_ { 1 };
  uint64_t consecutive_retransmissions_ {};
  // std::string tcp_buffer_ {};
  std::map<uint64_t, TCPSenderMessage> tcp_buffer_ {};
  bool FIN_sent {};
  void send_msg_with( uint16_t msg_length, uint64_t abs_sqeno, const TransmitFunction& transmit );
};
