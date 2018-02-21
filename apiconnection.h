#ifndef APICONNECTION_H
#define APICONNECTION_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QMap>


class ApiConnection : public QObject
{
    Q_OBJECT
public:
    explicit ApiConnection(QObject *parent = nullptr);
    void ask_for_weather(const QString & city_name, const QString & country_code, const QString & api_code, const QString & language="en");

signals:
    void weather_list_ready(QVariantList);

public slots:
    void weather_ready(QNetworkReply *);
private:
    QNetworkAccessManager * manager;
    //QNetworkReply * weather_reply;
};

#endif // APICONNECTION_H
