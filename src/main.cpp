
#include "Application.h"

#ifdef __APPLE__
#include <unistd.h>
#endif

int main()
{
    // Prevent running app twice while debugging via xcode 16.1
#ifdef __APPLE__
    sleep(1);
#endif
    
    Application app;
    app.run();
    
    return 0;
}
