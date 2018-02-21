#ifndef WEATHERAPP_H
#define WEATHERAPP_H

#include <QDialog>
#include <QStringList>
#include <QStringListModel>
#include "apiconnection.h"

namespace Ui {
class WeatherApp;
}

class WeatherApp : public QDialog
{
    Q_OBJECT

public:
    explicit WeatherApp(QWidget *parent = 0);
    void Test();

    ~WeatherApp();

private slots:
    void on_lineEdit_city_textChanged(const QString &arg1);

    void on_button_add_clicked();

    void on_lineEdit_api_textChanged(const QString &arg1);

    void on_button_connect_clicked();

private:
    void setComboBox();

    Ui::WeatherApp *ui;

    QStringList * string_list_country_codes;
    QStringListModel * string_list_model_country_codes;
    QStringListModel * string_list_model_city;

    ApiConnection * connection;
};

#endif // WEATHERAPP_H
