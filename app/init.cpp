#include <app.h>

AppClass App;

String AppClass::serverURL{"http://10.2.113.100:3000"};
bool AppClass::sound{true};

void init()
{
        App.init();
        App.start();
}
