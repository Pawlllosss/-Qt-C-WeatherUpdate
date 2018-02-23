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
    connect(this, SIGNAL(weather_list_ready(QVariantList)), (WeatherApp*)(parent), SLOT(add_to_weather_db(QVariantList)));

    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(weather_ready(QNetworkReply*)));
}

void ApiConnection::ask_for_weather(const int city_id, const QString & city_name, const QString & country_code, const QString & api_code, const QString & language)
{
    //rember city_id value, for database purposes

    current_city_id = city_id;

    QNetworkRequest request(QUrl("http://api.openweathermap.org/data/2.5/weather?q="+city_name+","+country_code+"&APPID="+api_code+"&lang="+language));
    manager->get(request);

    qDebug()<<"http://api.openweathermap.org/data/2.5/weather?q="+city_name+","+country_code+"&APPID="+api_code+"&lang="+language;
    qDebug()<<"End of ask_for_weather";
}

void ApiConnection::weather_ready(QNetworkReply * reply)
{
    QString weather_info = (QString)reply->readAll();

    QJsonDocument weather_doc = QJsonDocument::fromJson(weather_info.toUtf8());
    QJsonObject weather_obj = weather_doc.object();

    qDebug()<<weather_obj;


    QJsonObject main_obj = weather_obj.value("main").toObject();//access to object main nested in json


    //date-hour-temperature-pressure-humidity-weather_description

   QJsonObject weather_description = weather_obj.value("weather").toArray().first().toObject();

   QVariantList weather_list;

    weather_list << QDate::currentDate() << QTime::currentTime() << main_obj.value("temp").toDouble() << main_obj.value("pressure").toDouble()
                 << main_obj.value("humidity").toInt() << weather_description.value("description").toString() << current_city_id;

    qDebug() <<weather_list;


    emit weather_list_ready(weather_list);
}
