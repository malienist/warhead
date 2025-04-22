//use this file to build an executable for testing the injection on. This bin should keep running for a minute which should give you enough time to launch the warhead. 
#include <windows.h>
int main() {
    Sleep(60000);  // Sleep 1 minute so you can inject into it
    return 0;
}
