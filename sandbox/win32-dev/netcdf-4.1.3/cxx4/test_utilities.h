#include <vector>
using namespace std;

template <class T> void initializeVector(std::vector<T>& dataVector) {
  for(int  i=0; i<dataVector.size(); i++) {
    dataVector[i]=i;
  }
}
