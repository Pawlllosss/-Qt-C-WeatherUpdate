# Weather Update (C++/Qt)

A simple C++ project using Qt library, which downloads a weather through the free API provided by www.openweathermap.org. Then saves it to the SQL database.

## Getting Started

As the openweathermap's API uses ISO 3166 country codes to determine in which country is the city. The program uses country code list in csv file, which can be downloaded from here: https://datahub.io/core/country-list
If this file is named "country_codes.csv" and is located in the build directory, then you don't need to perform any other action. In other case you need to select the file location during first run.

To use API you will need to obtain free API key, which you can get here: http://openweathermap.org/appid

After closing this program the: API key, last selected country, and "country_codes.csv" files location will be saved into "save.json" file. If you want to clean these settings, you can just remove this file.

## Author

* **Paweł Oczadly** - *All the stuff :)*

## License

This project is licensed under the MIT License.
