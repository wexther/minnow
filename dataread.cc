#include <cstdint>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

int main()
{
  std::ifstream file( "data.txt" );

  std::string line;
  std::getline( file, line );
  uint64_t seq_count, time_count, seq_remind;
  uint64_t seq, time, timestamp1, timestamp2;

  while ( std::getline( file, line ) ) {
    if ( sscanf( line.c_str(),
                 "[%ld.%ld] 64 bytes from 41.186.255.86: icmp_seq=%ld ttl=227 time=%ld ms",
                 &timestamp1,
                 &timestamp2,
                 &seq,
                 &time )
         != 4 ) {
      std::cout << "wrong with seq" << seq << std::endl;
      break;
    }
    ++seq_count;
    ++seq_remind;
    if (seq_remind!=seq){
      std::cout << "missing from " << seq_remind << " to " << seq << std::endl;
      seq_remind = seq;
    }
    time_count += time;
  }
  std::cout << "total num: " << seq_count << "/" << seq << " time_avg: " << time_count / seq_count << std::endl;
  file.close();
  return 0;
}