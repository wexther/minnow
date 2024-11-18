#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

using namespace std;

int main()
{
  ifstream file( "data.txt" );
  ofstream c;
  c.open( "rttdis.csv", ios::out | ios::trunc );
  int rttdis[800] = { 0 };

  string line;
  getline( file, line );
  uint64_t seq_count = 0, time_count = 0, seq_remind = 0;
  uint64_t seq, time, timestamp1, timestamp2;
  uint64_t min_rtt = UINT64_MAX, max_rtt = 0;

  while ( getline( file, line ) ) {
    if ( sscanf( line.c_str(),
                 "[%ld.%ld] 64 bytes from 41.186.255.86: icmp_seq=%ld ttl=227 time=%ld ms",
                 &timestamp1,
                 &timestamp2,
                 &seq,
                 &time )
         != 4 ) {
      cout << "wrong with seq" << seq << endl;
      break;
    }
    ++seq_count;
    ++seq_remind;
    if ( seq_remind != seq ) {
      cout << "missing from " << seq_remind << " to " << seq << endl;
      seq_remind = seq;
    }
    ++rttdis[time];
    time_count += time;
    if ( time > max_rtt ) {
      max_rtt = time;
    }
    if ( time < min_rtt ) {
      min_rtt = time;
    }
  }
  cout << "total num: " << seq_count << "/" << seq << " time_avg: " << time_count / seq_count
       << " time_max: " << max_rtt << " time_min: " << min_rtt << endl;

  for ( int i = 414; i <= 779; i++ ) {
    c << i << "," << rttdis[i] << endl;
  }
  
  c.close();
  file.close();
  return 0;
}