#include "apiconnection.h"
#include "weatherapp.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDate>
#include <QTime>

class WeatherApp;

ApiConnection::ApiConnection(QObject *parent) : QObject(parent)
{
    manager = new QNetworkAccessManager;

    connect(this, SIGNAL(weather_list_ready(QVariantList)), (WeatherApp*)(parent), SLOT(support_weather(QVariantList)));
}

void ApiConnection::ask_for_weather(const QString & city_name, const QString & country_code, const QString & api_code, const QString & language)
{
    QNetworkRequest request(QUrl("http://api.openweathermap.org/data/2.5/weather?q="+city_name+","+country_code+"&APPID="+api_code+"&lang="+language));
    manager->get(request);

    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(weather_ready(QNetworkReply*)));

    qDebug()<<"http://api.openweathermap.org/data/2.5/weather?q="+city_name+","+country_code+"&APPID="+api_code+"&lang="+language;
    qDebug()<<"End of ask_for_weather";
}

void ApiConnection::weather_ready(QNetworkReply * reply)
{
    QString weather_info = (QString)reply->readAll();

    QJsonDocument weather_doc = QJsonDocument::fromJson(weather_info.toUtf8());
    QJsonObject weather_obj = weather_doc.object();


    QJsonObject main_obj = weather_obj.value("main").toObject();//access to object main nested in json

    //getting into array weather nested in json, and parsing the description field

    /*QJsonArray weather_array = weather_obj.value("weather").toArray();

    QJsonValue weather_description;

    foreach( const QJsonValue & value, weather_array)
    {
        QJsonObject obj = value.toObject();
        if(obj.contains("description"))
        {
            weather_description = obj.value("description").toString();
            break;
        }

    }*/

    //date-hour-temperature-pressure-humidity-weather_description

   QJsonObject weather_description = weather_obj.value("weather").toArray().first().toObject();

   QVariantList weather_list;

    weather_list << QDate::currentDate() << QTime::currentTime() << main_obj.value("temp").toDouble() << main_obj.value("pressure").toDouble()
                 << main_obj.value("humidity").toInt() << weather_description.value("description").toString();

    qDebug() <<weather_list;

    //((WeatherApp*)(parent()))->Test(); //just curious

    //emit signal or just call a function to set in weather app?
    emit weather_list_ready(weather_list);
}
