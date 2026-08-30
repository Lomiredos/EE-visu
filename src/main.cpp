#include "visu/App.hpp"

int main()
{
    App &app = App::getInstance();

    if (!app.init())
        return 1;

    app.openProject("C:/Dev/eliott-engine-projects/empty-sphere");

    app.run();

    return 0;
}
