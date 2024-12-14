#include "router.hh"

#include <iostream>
#include <limits>

using namespace std;

void RouterMapped::insert( const RouterPre& router_pre,
                           size_t interface_num,
                           const std::optional<Address>& next_hop )
{
  auto insert_pair { children_.insert( make_pair( router_pre, RouterMapped( interface_num, next_hop ) ) ) };
  if ( insert_pair.second ) {
    return;
  }

  auto& eq_it { insert_pair.first };
  if ( eq_it->first.prefix_length_ < router_pre.prefix_length_ ) {
    return eq_it->second.insert( router_pre, interface_num, next_hop );
  } else if ( eq_it->first.prefix_length_ > router_pre.prefix_length_ ) {
    std::map<RouterPre, RouterMapped, std::less<>> new_children;
    new_children.emplace( move( *eq_it ) );

    auto cur_it { eq_it };
    auto erase_begin_it { cur_it };
    if ( cur_it != children_.begin() ) {
      --cur_it;
      while ( cur_it->first == router_pre ) {
        new_children.emplace_hint( new_children.begin(), move( *cur_it ) );
        erase_begin_it = cur_it;
        if ( cur_it == children_.begin() ) {
          break;
        }
        --cur_it;
      }
    }

    cur_it = eq_it;
    ++cur_it;
    auto erase_end_it { cur_it };
    while ( cur_it != children_.end() && cur_it->first.prefix_length_ > router_pre.prefix_length_ ) {
      new_children.emplace_hint( new_children.end(), move( *cur_it ) );
      ++cur_it;
      erase_end_it = cur_it;
    }

    children_.emplace_hint(
      children_.erase( erase_begin_it, erase_end_it ),
      make_pair( router_pre, RouterMapped( interface_num, next_hop, move( new_children ) ) ) );
  }
}

std::optional<RouterMapped> RouterMapped::find( const RouterPre& ip ) const
{
  auto find_it { children_.find( ip ) };
  if ( find_it == children_.end() ) {
    return *this;
  } else {
    return find_it->second.find( ip );
  }
}

// route_prefix: The "up-to-32-bit" IPv4 address prefix to match the datagram's destination address against
// prefix_length: For this route to be applicable, how many high-order (most-significant) bits of
//    the route_prefix will need to match the corresponding bits of the datagram's destination address?
// next_hop: The IP address of the next hop. Will be empty if the network is directly attached to the router (in
//    which case, the next hop address should be the datagram's final destination).
// interface_num: The index of the interface to send the datagram out on.
void Router::add_route( const uint32_t route_prefix,
                        const uint8_t prefix_length,
                        const optional<Address> next_hop,
                        const size_t interface_num )
{
  cerr << "DEBUG: adding route " << Address::from_ipv4_numeric( route_prefix ).ip() << "/"
       << static_cast<int>( prefix_length ) << " => " << ( next_hop.has_value() ? next_hop->ip() : "(direct)" )
       << " on interface " << interface_num << "\n";

  // Your code here.
  insert( RouterPre( route_prefix, prefix_length ), interface_num, next_hop );
}

// Go through all the interfaces, and route every incoming datagram to its proper outgoing interface.
void Router::route()
{
  // Your code here.
  for ( auto& interface : _interfaces ) {
    auto& datagrams { interface->datagrams_received() };
    while ( !datagrams.empty() ) {
      auto& gram { datagrams.front() };
      if ( gram.header.ttl == 0 ) {
        datagrams.pop();
        continue;
      }
      auto dest { find( RouterPre( gram.header.dst, 32 ) ) };

      if ( !dest || --gram.header.ttl == 0 || dest.value().interface_num() == -1UL ) {
        datagrams.pop();
        continue;
      }
      gram.header.compute_checksum();
      if ( dest.value().next_hop() ) {
        _interfaces[dest.value().interface_num()]->send_datagram( gram, dest.value().next_hop().value() );
      } else {
        _interfaces[dest.value().interface_num()]->send_datagram( gram,
                                                                  Address::from_ipv4_numeric( gram.header.dst ) );
      }
      datagrams.pop();
    }
  }
}
