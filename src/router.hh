#pragma once

#include <map>
#include <memory>
#include <optional>
#include <set>

#include "exception.hh"
#include "network_interface.hh"

struct RouterPre
{
  explicit RouterPre( uint32_t route_prefix, uint8_t prefix_length )
    : route_prefix_( route_prefix ), prefix_length_( prefix_length )
  {}

  //! \attention 该比较衍生出的equiv不符合等价关系
  bool operator<( const RouterPre& other ) const
  {
    uint8_t prefix_length { std::min( prefix_length_, other.prefix_length_ ) };
    if ( prefix_length == 0 ) {
      return false;
    }
    return ( route_prefix_ & ( static_cast<uint32_t>( -1 ) << ( 32 - prefix_length ) ) )
           < ( other.route_prefix_ & ( static_cast<uint32_t>( -1 ) << ( 32 - prefix_length ) ) );
  }
  bool operator==( const RouterPre& other ) const
  {
    uint8_t prefix_length { std::min( prefix_length_, other.prefix_length_ ) };
    if ( prefix_length == 0 ) {
      return true;
    }
    return ( route_prefix_ & ( static_cast<uint32_t>( -1 ) << ( 32 - prefix_length ) ) )
           == ( other.route_prefix_ & ( static_cast<uint32_t>( -1 ) << ( 32 - prefix_length ) ) );
  }

  uint32_t route_prefix_ {};
  uint8_t prefix_length_ {};
};

class RouterMapped
{
public:
  explicit RouterMapped( size_t interface_num = -1,
                         const std::optional<Address>& next_hop = {},
                         const std::map<RouterPre, RouterMapped, std::less<>>&& children = {} )
    : interface_num_( interface_num ), next_hop_( next_hop ), children_( std::move( children ) )
  {}
  void insert( const RouterPre& router_pre, size_t interface_num, const std::optional<Address>& next_hop );
  std::optional<RouterMapped> find( const RouterPre& ip ) const;
  std::optional<Address>& next_hop() { return next_hop_; }
  size_t interface_num() { return interface_num_; }

protected:
  size_t interface_num_;
  std::optional<Address> next_hop_;
  std::map<RouterPre, RouterMapped, std::less<>> children_;
};

//! \brief A router that has multiple network interfaces and
// performs longest-prefix-match routing between them.
class Router : private RouterMapped
{
public:
  // Add an interface to the router
  //! \param[in] interface an already-constructed network interface
  //! \returns The index of the interface after it has been added to the router
  size_t add_interface( std::shared_ptr<NetworkInterface> interface )
  {
    _interfaces.push_back( notnull( "add_interface", std::move( interface ) ) );
    return _interfaces.size() - 1;
  }

  // Access an interface by index
  std::shared_ptr<NetworkInterface> interface( const size_t N ) { return _interfaces.at( N ); }

  // Add a route (a forwarding rule)
  void add_route( uint32_t route_prefix,
                  uint8_t prefix_length,
                  std::optional<Address> next_hop,
                  size_t interface_num );

  // Route packets between the interfaces
  void route();

private:
  // The router's collection of network interfaces
  std::vector<std::shared_ptr<NetworkInterface>> _interfaces {};
};
