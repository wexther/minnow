#include <iostream>

#include "arp_message.hh"
#include "exception.hh"
#include "network_interface.hh"

using namespace std;

//! \param[in] ethernet_address Ethernet (what ARP calls "hardware") address of the interface
//! \param[in] ip_address IP (what ARP calls "protocol") address of the interface
NetworkInterface::NetworkInterface( string_view name,
                                    shared_ptr<OutputPort> port,
                                    const EthernetAddress& ethernet_address,
                                    const Address& ip_address )
  : name_( name )
  , port_( notnull( "OutputPort", move( port ) ) )
  , ethernet_address_( ethernet_address )
  , ip_address_( ip_address )
{
  cerr << "DEBUG: Network interface has Ethernet address " << to_string( ethernet_address ) << " and IP address "
       << ip_address.ip() << "\n";
}

//! \param[in] dgram the IPv4 datagram to be sent
//! \param[in] next_hop the IP address of the interface to send it to (typically a router or default gateway, but
//! may also be another host if directly connected to the same network as the destination) Note: the Address type
//! can be converted to a uint32_t (raw 32-bit IP address) by using the Address::ipv4_numeric() method.
void NetworkInterface::send_datagram( const InternetDatagram& dgram, const Address& next_hop )
{
  // Your code here.
  // (void)dgram;
  // (void)next_hop;

  uint32_t ip = next_hop.ipv4_numeric();
  auto map_it = ip_map_.find( ip );
  if ( map_it == ip_map_.end() ) {
    auto request_it = request_timeout_map_.find( ip );
    if ( request_it == request_timeout_map_.end() || request_it->second < cur_time_stamp_ ) {
      ARPMessage request;
      request.opcode = ARPMessage::OPCODE_REQUEST;
      request.sender_ethernet_address = ethernet_address_;
      request.sender_ip_address = ip_address_.ipv4_numeric();
      request.target_ethernet_address = EthernetAddress { 0, 0, 0, 0, 0, 0 };
      request.target_ip_address = ip;

      transmit( EthernetFrame { EthernetHeader { ETHERNET_BROADCAST, ethernet_address_, EthernetHeader::TYPE_ARP },
                                serialize( request ) } );

      request_timeout_map_[ip] = cur_time_stamp_ + 5000;
    }
    pending_datagram_.emplace( ip, dgram );
  } else {
    transmit( EthernetFrame { EthernetHeader { map_it->second, ethernet_address_, EthernetHeader::TYPE_IPv4 },
                              serialize( dgram ) } );
  }
}

//! \param[in] frame the incoming Ethernet frame
void NetworkInterface::recv_frame( const EthernetFrame& frame )
{
  // Your code here.
  // (void)frame;

  if ( frame.header.dst != ethernet_address_ && frame.header.dst != ETHERNET_BROADCAST ) {
    return;
  }
  if ( frame.header.type == EthernetHeader::TYPE_IPv4 ) {
    InternetDatagram dgram;
    if ( parse( dgram, frame.payload ) ) {
      datagrams_received_.push( dgram );
    }
  } else { // frame.header.typr == EthernetHeader::TYPE_ARP
    ARPMessage arp;
    if ( parse( arp, frame.payload ) ) {
      auto map_it = ip_map_.find( arp.sender_ip_address );
      if ( map_it == ip_map_.end() ) {
        map_it = ip_map_.emplace( arp.sender_ip_address, arp.sender_ethernet_address ).first;
      } else {
        for ( auto list_it = time_expire_list_.begin(); list_it != time_expire_list_.end(); ++list_it ) {
          if ( list_it->second == map_it ) {
            time_expire_list_.erase( list_it );
            break;
          }
        }
      }
      time_expire_list_.emplace_back( cur_time_stamp_ + 30000, map_it );

      auto pending_it_range = pending_datagram_.equal_range( arp.sender_ip_address );
      for ( auto pending_it = pending_it_range.first; pending_it != pending_it_range.second; ++pending_it ) {
        transmit( EthernetFrame {
          EthernetHeader { arp.sender_ethernet_address, ethernet_address_, EthernetHeader::TYPE_IPv4 },
          serialize( pending_it->second ) } );
      }
      pending_datagram_.erase( pending_it_range.first, pending_it_range.second );

      if ( arp.opcode == ARPMessage::OPCODE_REQUEST ) {
        if ( arp.target_ip_address == ip_address_.ipv4_numeric() ) {
          ARPMessage reply;
          reply.opcode = ARPMessage::OPCODE_REPLY;
          reply.sender_ethernet_address = ethernet_address_;
          reply.sender_ip_address = ip_address_.ipv4_numeric();
          reply.target_ethernet_address = arp.sender_ethernet_address;
          reply.target_ip_address = arp.sender_ip_address;

          transmit( EthernetFrame {
            EthernetHeader { arp.sender_ethernet_address, ethernet_address_, EthernetHeader::TYPE_ARP },
            serialize( reply ) } );
        }
      }
    }
  }
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void NetworkInterface::tick( const size_t ms_since_last_tick )
{
  // Your code here.
  // (void)ms_since_last_tick;

  cur_time_stamp_ += ms_since_last_tick;
  while ( !time_expire_list_.empty() && time_expire_list_.front().first < cur_time_stamp_ ) {
    ip_map_.erase( time_expire_list_.front().second );
    time_expire_list_.pop_front();
  }
}
