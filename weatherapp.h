#ifndef WEATHERAPP_H
#define WEATHERAPP_H

#include <QDialog>
#include <QStringList>
#include <QStringListModel>
#include <QVariantList>
#include <QtSql>

#include "apiconnection.h"

namespace Ui {
class WeatherApp;
}

class WeatherApp : public QDialog
{
    Q_OBJECT

public:
    explicit WeatherApp(QWidget *parent = 0);
    //void Test();

    ~WeatherApp();

private slots:
    void on_lineEdit_city_textChanged(const QString &arg1);

    void on_button_add_clicked();

    void on_lineEdit_api_textChanged(const QString &arg1);

    void on_button_connect_clicked();

    //END OF UI RELATED

    void add_to_weather_db(QVariantList weather_info);

    void support_weather(QVariantList weather_info);

    void on_button_history_clicked();

    void on_button_delete_clicked();

private:
    void setComboBox();

    void setDatabase();

    Ui::WeatherApp *ui;

    QStringList * string_list_country_codes;
    QStringListModel * string_list_model_country_codes;
    QStringListModel * string_list_model_city;

    QSqlRelationalTableModel * model;
    QSqlRelationalTableModel * weather_model;

    ApiConnection * connection;

    int current_id_city;
    int current_id_weather;
};

#endif // WEATHERAPP_H
