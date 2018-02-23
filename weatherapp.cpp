#include "weatherapp.h"
#include "ui_weatherapp.h"

#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QMessageBox>
#include <QModelIndex>

WeatherApp::WeatherApp(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::WeatherApp)
{
    ui->setupUi(this);

    ui->button_add->setDisabled(true);//until there is some city name and country code given
    ui->button_connect->setDisabled(true);//wait until there is api code in valid format given and city selected
    //ui->button_history->setDisabled(true);//as previous one (first SQL database is needed to implement)

    ui->list_city->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->list_city->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->list_city->verticalHeader()->hide();

    setComboBox();

    //string_list_model_city = new QStringListModel;
   // ui->list_city->setModel(string_list_model_city);

    connection = new ApiConnection(this);

    setDatabase();
}

void WeatherApp::setComboBox()
{
    QFile country_list("C:/Users/Pawlllosss/Documents/programowanie/qt/WeatherUpdate/country_codes.csv");

    if(!country_list.open(QIODevice::ReadOnly))
    {
        qDebug()<<country_list.errorString();
        QMessageBox::warning(this, "File error", "Error occured during opening the file");
        exit(1);
    }

    string_list_country_codes = new QStringList;
    QTextStream input(&country_list);

    if(!input.atEnd())
        input.readLine();//skip first line

    while(!input.atEnd())
    {
        QString line = input.readLine();
        if(line.contains("\","))
            string_list_country_codes->append(line.split("\",")[1]);
        else
            string_list_country_codes->append(line.split(",")[1]);
    }

    //TO DO - sort this list

    string_list_model_country_codes = new QStringListModel(*string_list_country_codes);

    ui->comboBox_country->setModel(string_list_model_country_codes);


}
/*
void WeatherApp::Test()
{
    qDebug()<<"test";
}
*/

//set database
void WeatherApp::setDatabase()
{
    //to do - set db in file and check if already exist, if exists then load its content

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(":memory:");

    if(!db.open())
    {
        QMessageBox::warning(this, "Error with creating database", db.lastError().text());
        return;
    }

    QSqlQuery q;

    if(!q.exec(QLatin1String("create table cities(id_city integer primary key, name varchar, country varchar)")))
    {
        QMessageBox::warning(this, "Error table cities", q.lastError().text());
        return;
    }

    if(!q.exec(QLatin1String("create table weather(id_weather integer primary key, date_weather date, hour time, temperature double, pressure int, humidity int, description varchar, city_id int)")))
    {
        QMessageBox::warning(this, "Error table weather", q.lastError().text());
        return;
    }

    model = new QSqlRelationalTableModel(ui->list_city);
    model->setTable("weather");

    model->setRelation(model->fieldIndex("city_id"), QSqlRelation("cities", "id_city", "name, country"));

    //important!
    model->select();

    model->setTable("cities");

    ui->list_city->setModel(model);


    //weather model


    //table_sql_test - self explanatiory - for test - delete it later
    weather_model = new QSqlRelationalTableModel(ui->table_sql_test);
    weather_model->setTable("weather");
    ui->table_sql_test->setModel(weather_model);
}

//destructor

WeatherApp::~WeatherApp()
{
    delete ui;
}

//SLOTS

void WeatherApp::add_to_weather_db(QVariantList weather_info)
{
    qDebug()<<"Weather model start: ===================";

    static int id = 0;

    int rows = weather_model->rowCount();

    qDebug()<<rows;
    qDebug()<<weather_info;

    weather_model->insertRow(rows);

    //id_weather integer primary key, date_weather date, hour time, temperature int, pressure int, humidity int, description varchar, city_id int

    qDebug()<<weather_info[0].toDate();
    qDebug()<<weather_info[1].toTime();
    qDebug()<<weather_info[2].toDouble();
    qDebug()<< weather_info[3].toInt();

    weather_model->setData(weather_model->index(rows, 0), id);
    weather_model->setData(weather_model->index(rows, 1), weather_info[0].toDate());
    weather_model->setData(weather_model->index(rows, 2), weather_info[1].toTime());
    weather_model->setData(weather_model->index(rows, 3), weather_info[2].toDouble());
    weather_model->setData(weather_model->index(rows, 4), weather_info[3].toInt());
    weather_model->setData(weather_model->index(rows, 5), weather_info[4].toInt());
    weather_model->setData(weather_model->index(rows, 6), weather_info[5].toString());
    weather_model->setData(weather_model->index(rows, 7), weather_info[6].toInt());

    weather_model->submitAll();

    id++;

    qDebug()<<"Weather model end: ===================";
}

void WeatherApp::support_weather(QVariantList weather_info)
{
    qDebug()<<"Test";

    QString text_output;

    if( weather_info.size() != 7 )
        return;

/*
    for(int i = 0; i < weather_info.size() ; i++)
    {
        qDebug()<<weather_info[i];
        text_output.append(weather_info[i].toString()+"\n");
    }
*/
    text_output.append("Date: " + weather_info[0].toString() + "\n");
    text_output.append("Hour: " + weather_info[1].toString() + "\n");
    text_output.append("Temperature: " + QString::number((weather_info[2].toDouble() - 32)*(5.0/9)) + " C\n");
    text_output.append("Pressure: " + QString::number(weather_info[3].toDouble()) + " hPa\n");
    text_output.append("Humidity: " + QString::number(weather_info[4].toInt()) + "% \n");
    text_output.append("Weather description: " + weather_info[5].toString() + "\n");


    qDebug()<<text_output;
    ui->text_weather_info->setText(text_output);
}

//UI SLOTS

void WeatherApp::on_lineEdit_city_textChanged(const QString &arg1)
{
    if(arg1.isEmpty())
        ui->button_add->setDisabled(true);
    else
        ui->button_add->setDisabled(false);
}

void WeatherApp::on_button_add_clicked()
{

    //TO DO: PREVENT ADDING IDENTICAL CITIES
    //CHANGE HEADERS
    static int id = 0;

    //first click doesnt work!
    /*
    int rows = string_list_model_city->rowCount();
    string_list_model_city->insertRow(rows);
    QModelIndex index = string_list_model_city->index( rows - 1 );
    string_list_model_city->setData( index, QString(ui->lineEdit_city->text()+" - "+ui->comboBox_country->currentText()));
    */
    int rows = model->rowCount();

    qDebug()<<rows;

    model->insertRow(rows);

/*
    QSqlRecord rec = model->record();

    rec.setValue("name", ui->lineEdit_city->text() );
    rec.setValue("country", ui->comboBox_country->currentText() );

    qDebug()<<rec;

    qDebug()<<model->setRecord( rows , rec);
*/

    model->setData(model->index(rows, 0), id);
    model->setData(model->index(rows, 1), ui->lineEdit_city->text());
    model->setData(model->index(rows, 2), ui->comboBox_country->currentText());

    model->submitAll();

    id++;
 /*   model->insertRecord(model->rowCount() - 1, rec);
[
    model->submitAll();

    model->database().commit();*/

}

void WeatherApp::on_lineEdit_api_textChanged(const QString &arg1)
{
    if(arg1.isEmpty())
        ui->button_connect->setDisabled(true);
    else
        ui->button_connect->setDisabled(false);
}

void WeatherApp::on_button_connect_clicked()
{
   /* QModelIndex index = ui->list_city->currentIndex();

    if(!index.isValid())
            return;

    QStringList city_and_country = index.data().toString().split(" - ");

    connection->ask_for_weather(city_and_country[0].simplified(), city_and_country[1].simplified(), ui->lineEdit_api->text());
    */
    QItemSelectionModel *selection = ui->list_city->selectionModel();


    //it doesn's work (I  want to with whether item was selected
    if( !selection )
    {
        qDebug() << "Didn't select a row";
        return;
    }

    //it should remember id_city!!!!

    int select_id = selection->selectedRows(0).value(0).data().toInt();
    QString select_name = selection->selectedRows(1).value(0).data().toString();
    QString select_country = selection->selectedRows(2).value(0).data().toString();

    connection->ask_for_weather(select_id, select_name, select_country, ui->lineEdit_api->text());

}

void WeatherApp::on_button_history_clicked()
{
    QItemSelectionModel *selection = ui->list_city->selectionModel();

    //TO DO: implement valid selection

    int select_id = selection->selectedRows(0).value(0).data().toInt();
    QString city_name = selection->selectedRows(1).value(0).data().toString();
    QString country_code = selection->selectedRows(2).value(0).data().toString();

    weather_model->setFilter("city_id = "+QString::number(select_id));

    //sorting first by hour (col 3) and then by date (col 2) (?)

    weather_model->select();

    QString text_output;

    text_output.append(city_name + " = " + country_code +"\n=============\n");

    for(int i = 0 ; i < weather_model->rowCount() ; i++)
    {
        //id_weather integer primary key, date_weather date, hour time, temperature double, pressure int, humidity int, description varchar, city_id int
        /*text_output.append("Date: " + weather_model->record(i).value("date_weather").toDate() + "\n");
        text_output.append("Hour: " + weather_model->record(i).value("hour").toDateTime() + "\n");
        text_output.append("Temperature: " + weather_model->record(i).value("temperature").toDouble() " C\n");
        text_output.append("Pressure: " + weather_model->record(i).value("pressure").toInt() + " hPa\n");
        text_output.append("Humidity: " + weather_model->record(i).value("humidity").toInt() + "% \n");
        text_output.append("Weather description: " + weather_model->record(i).value("description").toString()  + "\n");*/
        text_output.append("Date: " + weather_model->record(i).value("date_weather").toString()  + "\n");
        text_output.append("Hour: " + weather_model->record(i).value("hour").toString() + "\n");
        text_output.append("Temperature: " + weather_model->record(i).value("temperature").toString() + " C\n");
        text_output.append("Pressure: " + weather_model->record(i).value("pressure").toString() + " hPa\n");
        text_output.append("Humidity: " + weather_model->record(i).value("humidity").toString() + "% \n");
        text_output.append("Weather description: " + weather_model->record(i).value("description").toString()  + "\n\n");
    }

    qDebug()<<text_output;
    ui->text_weather_info->setText(text_output);
}
