#include "%{APPNAMELC}.h"
#include <iostream>

using namespace std;

int main(int argc, char** argv)
{
  %{APPNAMEID}* instance = new %{APPNAMEID};
  cout << "Created an instance of %{APPNAMEID}";
  delete instance;
  cout << "Deleted the instance";
}
