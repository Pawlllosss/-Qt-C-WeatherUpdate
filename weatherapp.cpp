#include "weatherapp.h"
#include "ui_weatherapp.h"

#include <QFile>
#include <QFileDialog>
#include <QTextStream>
#include <QDebug>
#include <QMessageBox>
#include <QModelIndex>


WeatherApp::WeatherApp(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::WeatherApp), country_codes_file_path(QDir::currentPath() + "/country_codes.csv"), last_used_country_code(""), current_id_city(0), current_id_weather(0)
{
    ui->setupUi(this);

    ui->button_add->setDisabled(true);//until there is some city name and country code given
    ui->button_connect->setDisabled(true);//wait until there is api code in valid format given and city selected

    ui->list_city->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->list_city->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->list_city->verticalHeader()->hide();

    //load last setting if save.json file exists
    QFile settings("save.json");

    if(settings.exists())
    {
        if(!settings.open(QIODevice::ReadOnly))
            QMessageBox::warning(this, "Previous settings", "Unable to load previous settings");

        QByteArray loaded_settings = settings.readAll();

        QJsonObject settings_object = QJsonDocument::fromJson(loaded_settings).object();

        //set country codes file path
        country_codes_file_path = settings_object["country_codes_file_name"].toString();

        //set last used api key
        ui->lineEdit_api->setText(settings_object["api"].toString());

        //set last used country code to combo box
        last_used_country_code = settings_object["selected_country"].toString();

        qDebug()<<settings_object["api"]<<settings_object["selected_country"];
    }

    //set countrynames to combobox
    setComboBox();

    connection = new ApiConnection(this);

    //set database tables
    setDatabase();
}

void WeatherApp::setComboBox()
{
    QFile country_list(country_codes_file_path);

    if(!country_list.exists())
    {
        country_codes_file_path = QFileDialog::getOpenFileName(this, "Country codes csv file", QDir::currentPath()
                                                               , "csv file (*.csv)");
        country_list.setFileName(country_codes_file_path);
    }

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

    std::sort( string_list_country_codes->begin(), string_list_country_codes->end() );

    string_list_model_country_codes = new QStringListModel(*string_list_country_codes);

    ui->comboBox_country->setModel(string_list_model_country_codes);

    //set a country code if it has been loaded from setting json file
    if( last_used_country_code != "" )
    {
        int index = ui->comboBox_country->findText(last_used_country_code);

        if(index != -1)
            ui->comboBox_country->setCurrentIndex(index);
    }
}

//set database
void WeatherApp::setDatabase()
{

    int loaded = 0;
    int loaded_weather = 0;
    //to do - set db in file and check if already exist, if exists then load its content

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    qDebug()<<QDir::currentPath();
    db.setDatabaseName(QDir::currentPath() + "/weather.db.sqlite");

    if(!db.open())
    {
        QMessageBox::warning(this, "Error while creating database", db.lastError().text());
        return;
    }

    QStringList existing_tables = db.tables();

    qDebug()<<existing_tables;


    QSqlQuery q;

    if(!(existing_tables.contains("cities")))
    {
        if(!q.exec(QLatin1String("create table cities(id_city integer primary key, name varchar, country varchar)")))
        {
            QMessageBox::warning(this, "Error table cities", q.lastError().text());
            return;
        }
    }
    else
    {
        if(!q.exec(QLatin1String("select * from cities")))
        {
            QMessageBox::warning(this, "Error during loading table cities", q.lastError().text());
            return;
        }

        loaded = 1;
        qDebug()<<"select";
    }

    if(!(existing_tables.contains("weather")))//I want to load db from file and if it already exists don't create new table
    {
        if(!q.exec(QLatin1String("create table weather(id_weather integer primary key, date_weather date, hour time, temperature double, pressure int, humidity int, description varchar, city_id int)")))
        {
            QMessageBox::warning(this, "Error table weather", q.lastError().text());
            return;
        }
    }
    else
    {
        if(!q.exec(QLatin1String("select * from weather")))
        {
            QMessageBox::warning(this, "Error during loading table weather", q.lastError().text());
            return;
        }

        loaded_weather = 1;
        qDebug()<<"select weather";
    }

    model = new QSqlRelationalTableModel(ui->list_city);


    model->setTable("cities");
    model->setSort(0, Qt::AscendingOrder);//it might be useful to sort cities by id_city;

    model->setHeaderData(1, Qt::Horizontal, "City name");
    model->setHeaderData(2, Qt::Horizontal, "Country" );

    ui->list_city->setModel(model);

    ui->list_city->setColumnHidden( 0 , true); //hide id field

    model->select();//important!

    if(loaded)
    {
        qDebug()<<"loaded";
        qDebug()<<model->record(model->rowCount() - 1).value("id_city").toInt();
        current_id_city = model->record(model->rowCount() - 1).value("id_city").toInt();
    }

    //weather model


    weather_model = new QSqlRelationalTableModel();
    weather_model->setTable("weather");

    weather_model->select();//it's very, very important!

    if(loaded_weather)
    {
        qDebug()<<"loaded_weather";
        current_id_weather = weather_model->record(weather_model->rowCount() - 1).value("id_weather").toInt();
    }
}

void WeatherApp::removeWeather(int city_id_rm)
{
    for (int i = 0; i < weather_model->rowCount(); i++) {
        int city_id = weather_model->record(i).value("city_id").toInt();

        if( city_id == city_id_rm)
            weather_model->removeRow(i);
    }

    weather_model->select();
}

//destructor

WeatherApp::~WeatherApp()
{
    //saving settings to json
    QFile saveFile(QString("save.json"));

    if(!saveFile.open(QIODevice::WriteOnly))
        QMessageBox::warning(this, "Error saving", "Unable to save settings");

    QJsonObject save_object;

    save_object["country_codes_file_name"] = country_codes_file_path;
    save_object["api"] = ui->lineEdit_api->text();
    save_object["selected_country"] = ui->comboBox_country->currentText();

    QJsonDocument save_document( save_object );
    saveFile.write( save_document.toJson() );

    //-----------------------------------

    delete ui;
}

//SLOTS

void WeatherApp::add_to_weather_db(QVariantList weather_info)
{
    qDebug()<<"Weather model start: ===================";

    current_id_weather++;

    int rows = weather_model->rowCount();

    qDebug()<<rows;
    qDebug()<<weather_info;

    weather_model->insertRow(rows);

    //id_weather integer primary key, date_weather date, hour time, temperature double, pressure int, humidity int, description varchar, city_id int

    qDebug()<<weather_info[0].toDate();
    qDebug()<<weather_info[1].toTime();
    qDebug()<<weather_info[2].toDouble();
    qDebug()<< weather_info[3].toInt();

    weather_model->setData(weather_model->index(rows, 0), current_id_weather);
    weather_model->setData(weather_model->index(rows, 1), weather_info[0].toDate());
    weather_model->setData(weather_model->index(rows, 2), weather_info[1].toTime());
    weather_model->setData(weather_model->index(rows, 3), weather_info[2].toDouble() - 273 );//kelvins to celcius
    weather_model->setData(weather_model->index(rows, 4), weather_info[3].toInt());
    weather_model->setData(weather_model->index(rows, 5), weather_info[4].toInt());
    weather_model->setData(weather_model->index(rows, 6), weather_info[5].toString());
    weather_model->setData(weather_model->index(rows, 7), weather_info[6].toInt());

    weather_model->submitAll();

    qDebug()<<"Weather model end: ===================";
}

void WeatherApp::support_weather(QVariantList weather_info)
{
    qDebug()<<"Test";

    QString text_output;

    if( weather_info.size() != 7 )
        return;

    text_output.append("Date: " + weather_info[0].toString() + "\n");
    text_output.append("Hour: " + weather_info[1].toString() + "\n");
    text_output.append("Temperature: " + QString::number((weather_info[2].toDouble() - 273)) + " C\n"); //converting kelvin's to celcius ( we are not physics - luckily )
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

    //TO DO:
    //CHANGE HEADERS

    //check duplicate
    QString city_name = ui->lineEdit_city->text();
    QString country_code = ui->comboBox_country->currentText();

    for (int i = 0; i < model->rowCount(); i++) {
        QString name_record = model->record(i).value(1).toString();
        QString country_record = model->record(i).value(2).toString();

        if(( city_name == name_record ) && ( country_code == country_record ))
        {
            QMessageBox::warning(this, "Duplicate city!", "This city already exist in database!");
            return;
        }
    }

    //add city

    current_id_city++;

    int rows = model->rowCount();

    qDebug()<<rows;

    model->insertRow(rows);


    model->setData(model->index(rows, 0), current_id_city);
    model->setData(model->index(rows, 1), city_name);
    model->setData(model->index(rows, 2), country_code);

    model->submitAll();

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
    QItemSelectionModel *selection = ui->list_city->selectionModel();


    if( selection->selectedRows().count() != 1)
    {
        qDebug()<<"NO VALID SELECTION";
        return;
    }

    //it have to remember id_city
    int select_id = selection->selectedRows(0).value(0).data().toInt();
    QString select_name = selection->selectedRows(1).value(0).data().toString();
    QString select_country = selection->selectedRows(2).value(0).data().toString();

    connection->ask_for_weather(select_id, select_name, select_country, ui->lineEdit_api->text());

}

void WeatherApp::on_button_history_clicked()
{
    QItemSelectionModel *selection = ui->list_city->selectionModel();

    if( selection->selectedRows().count() != 1)
    {
        qDebug()<<"NO VALID SELECTION";
        return;
    }

    int select_id = selection->selectedRows(0).value(0).data().toInt();
    QString city_name = selection->selectedRows(1).value(0).data().toString();
    QString country_code = selection->selectedRows(2).value(0).data().toString();

    weather_model->setFilter("city_id = "+QString::number(select_id));
    //sorting first by hour (col 3) and then by date (col 2) (?)
    weather_model->setSort(3, Qt::DescendingOrder);
    weather_model->setSort(2, Qt::DescendingOrder);


    weather_model->select();

    QString text_output;

    text_output.append(city_name + " = " + country_code +"\n=============\n");

    for(int i = 0 ; i < weather_model->rowCount() ; i++)
    {
        text_output.append("Date: " + weather_model->record(i).value("date_weather").toString()  + "\n");
        text_output.append("Hour: " + weather_model->record(i).value("hour").toString() + "\n");
        text_output.append("Temperature: " + QString::number(weather_model->record(i).value("temperature").toDouble()) + " C\n");
        text_output.append("Pressure: " + weather_model->record(i).value("pressure").toString() + " hPa\n");
        text_output.append("Humidity: " + weather_model->record(i).value("humidity").toString() + "% \n");
        text_output.append("Weather description: " + weather_model->record(i).value("description").toString()  + "\n\n");
    }

    qDebug()<<text_output;
    ui->text_weather_info->setText(text_output);
}

void WeatherApp::on_button_delete_clicked()
{
    QItemSelectionModel *selection = ui->list_city->selectionModel();
    QModelIndexList index = selection->selectedRows();

    if( index.count() != 1)
        return;

    removeWeather( selection->selectedRows(0).value(0).data().toInt() ); //it removes related records in the weather table


    model->removeRow( index.at(0).row());
    model->submitAll();
    model->select();//as in setdb, select() is important!!!!
}
