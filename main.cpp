#include "weatherapp.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    WeatherApp w;
    w.show();

    return a.exec();
}
