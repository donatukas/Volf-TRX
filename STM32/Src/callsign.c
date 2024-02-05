#include "callsign.h"
#include "fonts.h"
#include "fpga.h"
#include "functions.h"
#include "lcd.h"
#include "lcd_driver.h"
#include "locator.h"
#include "main.h"
#include "trx_manager.h"

// Global cty.dat
static const CALLSIGN_INFO_LINE CALLSIGN_INFO_GLOBAL_DB[CALLSIGN_DB_GLOBAL_COUNT] = {
    {.continent = "EU", .country = "Sov Mil Order of Malta", .cq_zone = 15, .itu_zone = 28, .lat = 41.90, .lon = 12.43, .prefixes = "1A"},
    {.continent = "AS", .country = "Spratly Islands", .cq_zone = 26, .itu_zone = 50, .lat = 9.88, .lon = 114.23, .prefixes = "9M0,BM9S,BN9S,BO9S,BP9S,BQ9S,BU9S,BV9S,BW9S,BX9S"},
    {.continent = "EU", .country = "Monaco", .cq_zone = 14, .itu_zone = 27, .lat = 43.73, .lon = 7.4, .prefixes = "3A"},
    {.continent = "AF", .country = "Agalega & St. Brandon", .cq_zone = 39, .itu_zone = 53, .lat = -10.45, .lon = 56.67, .prefixes = "3B6,3B7"},
    {.continent = "AF", .country = "Mauritius", .cq_zone = 39, .itu_zone = 53, .lat = -20.35, .lon = 57.5, .prefixes = "3B8"},
    {.continent = "AF", .country = "Rodriguez Island", .cq_zone = 39, .itu_zone = 53, .lat = -19.70, .lon = 63.42, .prefixes = "3B9"},
    {.continent = "AF", .country = "Equatorial Guinea", .cq_zone = 36, .itu_zone = 47, .lat = 1.70, .lon = 10.33, .prefixes = "3C"},
    {.continent = "AF", .country = "Annobon Island", .cq_zone = 36, .itu_zone = 52, .lat = -1.43, .lon = 5.62, .prefixes = "3C0"},
    {.continent = "OC", .country = "Fiji", .cq_zone = 32, .itu_zone = 56, .lat = -17.78, .lon = 177.92, .prefixes = "3D2"},
    {.continent = "OC", .country = "Conway Reef", .cq_zone = 32, .itu_zone = 56, .lat = -22.00, .lon = 175, .prefixes = "3D2CR"},
    {.continent = "OC", .country = "Rotuma Island", .cq_zone = 32, .itu_zone = 56, .lat = -12.48, .lon = 177.08, .prefixes = "3D2RI"},
    {.continent = "AF", .country = "Swaziland", .cq_zone = 38, .itu_zone = 57, .lat = -26.65, .lon = 31.48, .prefixes = "3DA"},
    {.continent = "AF", .country = "Tunisia", .cq_zone = 33, .itu_zone = 37, .lat = 35.40, .lon = 9.32, .prefixes = "3V,TS"},
    {.continent = "AS", .country = "Vietnam", .cq_zone = 26, .itu_zone = 49, .lat = 15.80, .lon = 107.9, .prefixes = "3W,XV"},
    {.continent = "AF", .country = "Guinea", .cq_zone = 35, .itu_zone = 46, .lat = 11.00, .lon = -10.68, .prefixes = "3X"},
    {.continent = "AF", .country = "Bouvet", .cq_zone = 38, .itu_zone = 67, .lat = -54.42, .lon = 3.38, .prefixes = "3Y0E"},
    {.continent = "SA", .country = "Peter 1 Island", .cq_zone = 12, .itu_zone = 72, .lat = -68.77, .lon = -90.58, .prefixes = "3Y0X"},
    {.continent = "AS", .country = "Azerbaijan", .cq_zone = 21, .itu_zone = 29, .lat = 40.45, .lon = 47.37, .prefixes = "4J,4K"},
    {.continent = "AS", .country = "Georgia", .cq_zone = 21, .itu_zone = 29, .lat = 42.00, .lon = 45, .prefixes = "4L"},
    {.continent = "EU", .country = "Montenegro", .cq_zone = 15, .itu_zone = 28, .lat = 42.50, .lon = 19.28, .prefixes = "4O"},
    {.continent = "AS", .country = "Sri Lanka", .cq_zone = 22, .itu_zone = 41, .lat = 7.60, .lon = 80.7, .prefixes = "4P,4Q,4R,4S"},
    {.continent = "EU", .country = "ITU HQ", .cq_zone = 14, .itu_zone = 28, .lat = 46.17, .lon = 6.05, .prefixes = "4U1ITU,4U1WRC"},
    {.continent = "NA", .country = "United Nations HQ", .cq_zone = 5, .itu_zone = 8, .lat = 40.75, .lon = -73.97, .prefixes = "4U1UN"},
    {.continent = "EU", .country = "Vienna Intl Ctr", .cq_zone = 15, .itu_zone = 28, .lat = 48.20, .lon = 16.3, .prefixes = "4U0R,4U1A,4U1VIC,4U2U,4U500M,4Y1A,C7A"},
    {.continent = "OC", .country = "Timor - Leste", .cq_zone = 28, .itu_zone = 54, .lat = -8.80, .lon = 126.05, .prefixes = "4W"},
    {.continent = "AS", .country = "Israel", .cq_zone = 20, .itu_zone = 39, .lat = 31.32, .lon = 34.82, .prefixes = "4X,4Z"},
    {.continent = "AF", .country = "Libya", .cq_zone = 34, .itu_zone = 38, .lat = 27.20, .lon = 16.6, .prefixes = "5A"},
    {.continent = "AS", .country = "Cyprus", .cq_zone = 20, .itu_zone = 39, .lat = 35.00, .lon = 33, .prefixes = "5B,C4,H2,P3"},
    {.continent = "AF", .country = "Tanzania", .cq_zone = 37, .itu_zone = 53, .lat = -5.75, .lon = 33.92, .prefixes = "5H,5I"},
    {.continent = "AF", .country = "Nigeria", .cq_zone = 35, .itu_zone = 46, .lat = 9.87, .lon = 7.55, .prefixes = "5N,5O"},
    {.continent = "AF", .country = "Madagascar", .cq_zone = 39, .itu_zone = 53, .lat = -19.00, .lon = 46.58, .prefixes = "5R,5S,6X"},
    {.continent = "AF", .country = "Mauritania", .cq_zone = 35, .itu_zone = 46, .lat = 20.60, .lon = -10.5, .prefixes = "5T"},
    {.continent = "AF", .country = "Niger", .cq_zone = 35, .itu_zone = 46, .lat = 17.63, .lon = 9.43, .prefixes = "5U"},
    {.continent = "AF", .country = "Togo", .cq_zone = 35, .itu_zone = 46, .lat = 8.40, .lon = 1.28, .prefixes = "5V"},
    {.continent = "OC", .country = "Samoa", .cq_zone = 32, .itu_zone = 62, .lat = -13.93, .lon = -171.7, .prefixes = "5W"},
    {.continent = "AF", .country = "Uganda", .cq_zone = 37, .itu_zone = 48, .lat = 1.92, .lon = 32.6, .prefixes = "5X"},
    {.continent = "AF", .country = "Kenya", .cq_zone = 37, .itu_zone = 48, .lat = 0.32, .lon = 38.15, .prefixes = "5Y,5Z"},
    {.continent = "AF", .country = "Senegal", .cq_zone = 35, .itu_zone = 46, .lat = 15.20, .lon = -14.63, .prefixes = "6V,6W"},
    {.continent = "NA", .country = "Jamaica", .cq_zone = 8, .itu_zone = 11, .lat = 18.20, .lon = -77.47, .prefixes = "6Y"},
    {.continent = "AS", .country = "Yemen", .cq_zone = 21, .itu_zone = 39, .lat = 15.65, .lon = 48.12, .prefixes = "7O"},
    {.continent = "AF", .country = "Lesotho", .cq_zone = 38, .itu_zone = 57, .lat = -29.22, .lon = 27.88, .prefixes = "7P"},
    {.continent = "AF", .country = "Malawi", .cq_zone = 37, .itu_zone = 53, .lat = -14.00, .lon = 34, .prefixes = "7Q"},
    {.continent = "AF", .country = "Algeria", .cq_zone = 33, .itu_zone = 37, .lat = 28.00, .lon = 2, .prefixes = "7R,7T,7U,7V,7W,7X,7Y"},
    {.continent = "NA", .country = "Barbados", .cq_zone = 8, .itu_zone = 11, .lat = 13.18, .lon = -59.53, .prefixes = "8P"},
    {.continent = "AS", .country = "Maldives", .cq_zone = 22, .itu_zone = 41, .lat = 4.15, .lon = 73.45, .prefixes = "8Q"},
    {.continent = "SA", .country = "Guyana", .cq_zone = 9, .itu_zone = 12, .lat = 6.02, .lon = -59.45, .prefixes = "8R"},
    {.continent = "EU", .country = "Croatia", .cq_zone = 15, .itu_zone = 28, .lat = 45.18, .lon = 15.3, .prefixes = "9A"},
    {.continent = "AF", .country = "Ghana", .cq_zone = 35, .itu_zone = 46, .lat = 7.70, .lon = -1.57, .prefixes = "9G"},
    {.continent = "EU", .country = "Malta", .cq_zone = 15, .itu_zone = 28, .lat = 35.88, .lon = 14.42, .prefixes = "9H"},
    {.continent = "AF", .country = "Zambia", .cq_zone = 36, .itu_zone = 53, .lat = -14.22, .lon = 26.73, .prefixes = "9I,9J"},
    {.continent = "AS", .country = "Kuwait", .cq_zone = 21, .itu_zone = 39, .lat = 29.38, .lon = 47.38, .prefixes = "9K,NLD"},
    {.continent = "AF", .country = "Sierra Leone", .cq_zone = 35, .itu_zone = 46, .lat = 8.50, .lon = -13.25, .prefixes = "9L"},
    {.continent = "AS", .country = "West Malaysia", .cq_zone = 28, .itu_zone = 54, .lat = 3.95, .lon = 102.23, .prefixes = "9M,9M62M,9W"},
    {.continent = "OC", .country = "East Malaysia", .cq_zone = 28, .itu_zone = 54, .lat = 2.68, .lon = 113.32, .prefixes = "9M6,9M8,9W6,9W8,9M4CKT,9M4CRB,9M4CRP"},
    {.continent = "AS", .country = "Nepal", .cq_zone = 22, .itu_zone = 42, .lat = 27.70, .lon = 85.33, .prefixes = "9N"},
    {.continent = "AF", .country = "Dem. Rep. of the Congo", .cq_zone = 36, .itu_zone = 52, .lat = -3.12, .lon = 23.03, .prefixes = "9O,9P,9Q,9R,9S,9T"},
    {.continent = "AF", .country = "Burundi", .cq_zone = 36, .itu_zone = 52, .lat = -3.17, .lon = 29.78, .prefixes = "9U"},
    {.continent = "AS", .country = "Singapore", .cq_zone = 28, .itu_zone = 54, .lat = 1.37, .lon = 103.78, .prefixes = "9V,S6"},
    {.continent = "AF", .country = "Rwanda", .cq_zone = 36, .itu_zone = 52, .lat = -1.75, .lon = 29.82, .prefixes = "9X"},
    {.continent = "SA", .country = "Trinidad & Tobago", .cq_zone = 9, .itu_zone = 11, .lat = 10.38, .lon = -61.28, .prefixes = "9Y,9Z"},
    {.continent = "AF", .country = "Botswana", .cq_zone = 38, .itu_zone = 57, .lat = -22.00, .lon = 24, .prefixes = "8O,A2"},
    {.continent = "OC", .country = "Tonga", .cq_zone = 32, .itu_zone = 62, .lat = -21.22, .lon = -175.13, .prefixes = "A3"},
    {.continent = "AS", .country = "Oman", .cq_zone = 21, .itu_zone = 39, .lat = 23.60, .lon = 58.55, .prefixes = "A4"},
    {.continent = "AS", .country = "Bhutan", .cq_zone = 22, .itu_zone = 41, .lat = 27.40, .lon = 90.18, .prefixes = "A5"},
    {.continent = "AS", .country = "United Arab Emirates", .cq_zone = 21, .itu_zone = 39, .lat = 24.00, .lon = 54, .prefixes = "A6"},
    {.continent = "AS", .country = "Qatar", .cq_zone = 21, .itu_zone = 39, .lat = 25.25, .lon = 51.13, .prefixes = "A7"},
    {.continent = "AS", .country = "Bahrain", .cq_zone = 21, .itu_zone = 39, .lat = 26.03, .lon = 50.53, .prefixes = "A9"},
    {.continent = "AS", .country = "Pakistan", .cq_zone = 21, .itu_zone = 41, .lat = 30.00, .lon = 70, .prefixes = "6P,6Q,6R,6S,AP,AQ,AR,AS"},
    {.continent = "AS", .country = "Scarborough Reef", .cq_zone = 27, .itu_zone = 50, .lat = 15.08, .lon = 117.72, .prefixes = "BS7H"},
    {.continent = "AS", .country = "Taiwan", .cq_zone = 24, .itu_zone = 44, .lat = 23.72, .lon = 120.88, .prefixes = "BM,BN,BO,BP,BQ,BU,BV,BW,BX"},
    {.continent = "AS", .country = "Pratas Island", .cq_zone = 24, .itu_zone = 44, .lat = 20.70, .lon = 116.7, .prefixes = "BM9P,BN9P,BO9P,BP9P,BQ9P,BU9P,BV9P,BW9P,BX9P"},
    {.continent = "AS",
     .country = "China",
     .cq_zone = 24,
     .itu_zone = 44,
     .lat = 36.00,
     .lon = 102,
     .prefixes = "3H,3H0,3H9,3I,3I0,3I9,3J,3J0,3J9,3K,3K0,3K9,3L,3L0,3L9,3M,3M0,3M9,3N,3N0,3N9,3O,3O0,3O9,3P,3P0,3P9,3Q,3Q0,3Q9,3R,3R0,3R9,3S,3S0,3S9,"
                 "3T,3T0,3T9,3U,3U0,3U9,B0,B2,B3,B4,B5,B6,B7,B8,B9,BA,BA0,BA9,BD,BD0,BD9,BG,BG0,BG9,BH,BH0,BH9,BI,BI0,BI9,BJ,BJ0,BJ9,BL,BL0,BL9,BT,BT0,"
                 "BT9,BY,BY0,BY9,BZ,BZ0,BZ9,XS,XS0,XS9,B1,B2A,B2B,B2C,B2D,B2E,B2F,B2G,B2H,B2I,B2J,B2K,B2L,B2M,B2N,B2O,B2P,B3G,B3H,B3I,B3J,B3K,B3L,B6Q,"
                 "B6R,B6S,B6T,B6U,B6V,B6W,B6X,B7A,B7B,B7C,B7D,B7E,B7F,B7G,B7H,B7Q,B7R,B7S,B7T,B7U,B7V,B7W,B7X,B8A,B8B,B8C,B8D,B8E,B8F,B8G,B8H,B8I,B8J,"
                 "B8K,B8L,B8M,B8N,B8O,B8P,B8Q,B8R,B8S,B8T,B8U,B8V,B8W,B8X,B9A,B9B,B9C,B9D,B9E,B9F,B9S,B9T,B9U,B9V,B9W,B9X,BA2A,BA2B,BA2C,BA2D,BA2E,BA2F,"
                 "BA2G,BA2H,BA2I,BA2J,BA2K,BA2L,BA2M,BA2N,BA2O,BA2P,BA3G,BA3H,BA3I,BA3J,BA3K,BA3L,BA6Q,BA6R,BA6S,BA6T,BA6U,BA6V,BA6W,BA6X,BA7A,BA7B,"
                 "BA7C,BA7D,BA7E,BA7F,BA7G,BA7H,BA7Q,BA7R,BA7S,BA7T,BA7U,BA7V,BA7W,BA7X,BA8A,BA8B,BA8C,BA8D,BA8E,BA8F,BA8G,BA8H,BA8I,BA8J,BA8K,BA8L,"
                 "BA8M,BA8N,BA8O,BA8P,BA8Q,BA8R,BA8S,BA8T,BA8U,BA8V,BA8W,BA8X,BA9A,BA9B,BA9C,BA9D,BA9E,BA9F,BA9S,BA9T,BA9U,BA9V,BA9W,BA9X,BD2A,BD2B,"
                 "BD2C,BD2D,BD2E,BD2F,BD2G,BD2H,BD2I,BD2J,BD2K,BD2L,BD2M,BD2N,BD2O,BD2P,BD3G,BD3H,BD3I,BD3J,BD3K,BD3L,BD6Q,BD6R,BD6S,BD6T,BD6U,BD6V,"
                 "BD6W,BD6X,BD7A,BD7B,BD7C,BD7D,BD7E,BD7F,BD7G,BD7H,BD7Q,BD7R,BD7S,BD7T,BD7U,BD7V,BD7W,BD7X,BD8A,BD8B,BD8C,BD8D,BD8E,BD8F,BD8G,BD8H,"
                 "BD8I,BD8J,BD8K,BD8L,BD8M,BD8N,BD8O,BD8P,BD8Q,BD8R,BD8S,BD8T,BD8U,BD8V,BD8W,BD8X,BD9A,BD9B,BD9C,BD9D,BD9E,BD9F,BD9S,BD9T,BD9U,BD9V,"
                 "BD9W,BD9X,BG2A,BG2B,BG2C,BG2D,BG2E,BG2F,BG2G,BG2H,BG2I,BG2J,BG2K,BG2L,BG2M,BG2N,BG2O,BG2P,BG3G,BG3H,BG3I,BG3J,BG3K,BG3L,BG6Q,BG6R,"
                 "BG6S,BG6T,BG6U,BG6V,BG6W,BG6X,BG7A,BG7B,BG7C,BG7D,BG7E,BG7F,BG7G,BG7H,BG7Q,BG7R,BG7S,BG7T,BG7U,BG7V,BG7W,BG7X,BG8A,BG8B,BG8C,BG8D,"
                 "BG8E,BG8F,BG8G,BG8H,BG8I,BG8J,BG8K,BG8L,BG8M,BG8N,BG8O,BG8P,BG8Q,BG8R,BG8S,BG8T,BG8U,BG8V,BG8W,BG8X,BG9A,BG9B,BG9C,BG9D,BG9E,BG9F,"
                 "BG9S,BG9T,BG9U,BG9V,BG9W,BG9X,BH2A,BH2B,BH2C,BH2D,BH2E,BH2F,BH2G,BH2H,BH2I,BH2J,BH2K,BH2L,BH2M,BH2N,BH2O,BH2P,BH3G,BH3H,BH3I,BH3J,"
                 "BH3K,BH3L,BH6Q,BH6R,BH6S,BH6T,BH6U,BH6V,BH6W,BH6X,BH7A,BH7B,BH7C,BH7D,BH7E,BH7F,BH7G,BH7H,BH7Q,BH7R,BH7S,BH7T,BH7U,BH7V,BH7W,BH7X,"
                 "BH8A,BH8B,BH8C,BH8D,BH8E,BH8F,BH8G,BH8H,BH8I,BH8J,BH8K,BH8L,BH8M,BH8N,BH8O,BH8P,BH8Q,BH8R,BH8S,BH8T,BH8U,BH8V,BH8W,BH8X,BH9A,BH9B,"
                 "BH9C,BH9D,BH9E,BH9F,BH9S,BH9T,BH9U,BH9V,BH9W,BH9X,BI2A,BI2B,BI2C,BI2D,BI2E,BI2F,BI2G,BI2H,BI2I,BI2J,BI2K,BI2L,BI2M,BI2N,BI2O,BI2P,"
                 "BI3G,BI3H,BI3I,BI3J,BI3K,BI3L,BI6Q,BI6R,BI6S,BI6T,BI6U,BI6V,BI6W,BI6X,BI7A,BI7B,BI7C,BI7D,BI7E,BI7F,BI7G,BI7H,BI7Q,BI7R,BI7S,BI7T,"
                 "BI7U,BI7V,BI7W,BI7X,BI8A,BI8B,BI8C,BI8D,BI8E,BI8F,BI8G,BI8H,BI8I,BI8J,BI8K,BI8L,BI8M,BI8N,BI8O,BI8P,BI8Q,BI8R,BI8S,BI8T,BI8U,BI8V,"
                 "BI8W,BI8X,BI9A,BI9B,BI9C,BI9D,BI9E,BI9F,BI9S,BI9T,BI9U,BI9V,BI9W,BI9X,BJ2A,BJ2B,BJ2C,BJ2D,BJ2E,BJ2F,BJ2G,BJ2H,BJ2I,BJ2J,BJ2K,BJ2L,"
                 "BJ2M,BJ2N,BJ2O,BJ2P,BJ3G,BJ3H,BJ3I,BJ3J,BJ3K,BJ3L,BJ6Q,BJ6R,BJ6S,BJ6T,BJ6U,BJ6V,BJ6W,BJ6X,BJ7A,BJ7B,BJ7C,BJ7D,BJ7E,BJ7F,BJ7G,BJ7H,"
                 "BJ7Q,BJ7R,BJ7S,BJ7T,BJ7U,BJ7V,BJ7W,BJ7X,BJ8A,BJ8B,BJ8C,BJ8D,BJ8E,BJ8F,BJ8G,BJ8H,BJ8I,BJ8J,BJ8K,BJ8L,BJ8M,BJ8N,BJ8O,BJ8P,BJ8Q,BJ8R,"
                 "BJ8S,BJ8T,BJ8U,BJ8V,BJ8W,BJ8X,BJ9A,BJ9B,BJ9C,BJ9D,BJ9E,BJ9F,BJ9S,BJ9T,BJ9U,BJ9V,BJ9W,BJ9X,BL2A,BL2B,BL2C,BL2D,BL2E,BL2F,BL2G,BL2H,"
                 "BL2I,BL2J,BL2K,BL2L,BL2M,BL2N,BL2O,BL2P,BL3G,BL3H,BL3I,BL3J,BL3K,BL3L,BL6Q,BL6R,BL6S,BL6T,BL6U,BL6V,BL6W,BL6X,BL7A,BL7B,BL7C,BL7D,"
                 "BL7E,BL7F,BL7G,BL7H,BL7Q,BL7R,BL7S,BL7T,BL7U,BL7V,BL7W,BL7X,BL8A,BL8B,BL8C,BL8D,BL8E,BL8F,BL8G,BL8H,BL8I,BL8J,BL8K,BL8L,BL8M,BL8N,"
                 "BL8O,BL8P,BL8Q,BL8R,BL8S,BL8T,BL8U,BL8V,BL8W,BL8X,BL9A,BL9B,BL9C,BL9D,BL9E,BL9F,BL9S,BL9T,BL9U,BL9V,BL9W,BL9X,BT2A,BT2B,BT2C,BT2D,"
                 "BT2E,BT2F,BT2G,BT2H,BT2I,BT2J,BT2K,BT2L,BT2M,BT2N,BT2O,BT2P,BT3G,BT3H,BT3I,BT3J,BT3K,BT3L,BT6Q,BT6R,BT6S,BT6T,BT6U,BT6V,BT6W,BT6X,"
                 "BT7A,BT7B,BT7C,BT7D,BT7E,BT7F,BT7G,BT7H,BT7Q,BT7R,BT7S,BT7T,BT7U,BT7V,BT7W,BT7X,BT8A,BT8B,BT8C,BT8D,BT8E,BT8F,BT8G,BT8H,BT8I,BT8J,"
                 "BT8K,BT8L,BT8M,BT8N,BT8O,BT8P,BT8Q,BT8R,BT8S,BT8T,BT8U,BT8V,BT8W,BT8X,BT9A,BT9B,BT9C,BT9D,BT9E,BT9F,BT9S,BT9T,BT9U,BT9V,BT9W,BT9X,"
                 "BY2A,BY2B,BY2C,BY2D,BY2E,BY2F,BY2G,BY2H,BY2I,BY2J,BY2K,BY2L,BY2M,BY2N,BY2O,BY2P,BY3G,BY3H,BY3I,BY3J,BY3K,BY3L,BY6Q,BY6R,BY6S,BY6T,"
                 "BY6U,BY6V,BY6W,BY6X,BY7A,BY7B,BY7C,BY7D,BY7E,BY7F,BY7G,BY7H,BY7Q,BY7R,BY7S,BY7T,BY7U,BY7V,BY7W,BY7X,BY8A,BY8B,BY8C,BY8D,BY8E,BY8F,"
                 "BY8G,BY8H,BY8I,BY8J,BY8K,BY8L,BY8M,BY8N,BY8O,BY8P,BY8Q,BY8R,BY8S,BY8T,BY8U,BY8V,BY8W,BY8X,BY9A,BY9B,BY9C,BY9D,BY9E,BY9F,BY9S,BY9T,"
                 "BY9U,BY9V,BY9W,BY9X,BZ2A,BZ2B,BZ2C,BZ2D,BZ2E,BZ2F,BZ2G,BZ2H,BZ2I,BZ2J,BZ2K,BZ2L,BZ2M,BZ2N,BZ2O,BZ2P,BZ3G,BZ3H,BZ3I,BZ3J,BZ3K,BZ3L,"
                 "BZ6Q,BZ6R,BZ6S,BZ6T,BZ6U,BZ6V,BZ6W,BZ6X,BZ7A,BZ7B,BZ7C,BZ7D,BZ7E,BZ7F,BZ7G,BZ7H,BZ7Q,BZ7R,BZ7S,BZ7T,BZ7U,BZ7V,BZ7W,BZ7X,BZ8A,BZ8B,"
                 "BZ8C,BZ8D,BZ8E,BZ8F,BZ8G,BZ8H,BZ8I,BZ8J,BZ8K,BZ8L,BZ8M,BZ8N,BZ8O,BZ8P,BZ8Q,BZ8R,BZ8S,BZ8T,BZ8U,BZ8V,BZ8W,BZ8X,BZ9A,BZ9B,BZ9C,BZ9D,"
                 "BZ9E,BZ9F,BZ9S,BZ9T,BZ9U,BZ9V,BZ9W,BZ9X"},
    {.continent = "OC", .country = "Nauru", .cq_zone = 31, .itu_zone = 65, .lat = -0.52, .lon = 166.92, .prefixes = "C2"},
    {.continent = "EU", .country = "Andorra", .cq_zone = 14, .itu_zone = 27, .lat = 42.58, .lon = 1.62, .prefixes = "C3"},
    {.continent = "AF", .country = "The Gambia", .cq_zone = 35, .itu_zone = 46, .lat = 13.40, .lon = -16.38, .prefixes = "C5"},
    {.continent = "NA", .country = "Bahamas", .cq_zone = 8, .itu_zone = 11, .lat = 24.25, .lon = -76, .prefixes = "C6"},
    {.continent = "AF", .country = "Mozambique", .cq_zone = 37, .itu_zone = 53, .lat = -18.25, .lon = 35, .prefixes = "C8,C9"},
    {.continent = "SA",
     .country = "Chile",
     .cq_zone = 12,
     .itu_zone = 14,
     .lat = -30.00,
     .lon = -71,
     .prefixes = "3G,CA,CB,CC,CD,CE,XQ,XR,3G7,3G8,CA7,CA8,CB7,CB8,CC7,CC8,CD7,CD8,CE7,CE8,XQ7,XQ8,XR7,XR8,CE6PGO,XQ6CFX,XQ6OA,XR6F"},
    {.continent = "SA", .country = "San Felix & San Ambrosio", .cq_zone = 12, .itu_zone = 14, .lat = -26.28, .lon = -80.07, .prefixes = "3G0X,CA0X,CB0X,CC0X,CD0X,CE0X,XQ0X,XR0X"},
    {.continent = "SA", .country = "Easter Island", .cq_zone = 12, .itu_zone = 63, .lat = -27.10, .lon = -109.37, .prefixes = "3G0,CA0,CB0,CC0,CD0,CE0,XQ0,XR0"},
    {.continent = "SA", .country = "Juan Fernandez Islands", .cq_zone = 12, .itu_zone = 14, .lat = -33.60, .lon = -78.85, .prefixes = "3G0Z,CA0Z,CB0Z,CC0Z,CD0Z,CE0I,CE0Z,XQ0Z,XR0Z"},
    {.continent = "SA",
     .country = "Antarctica",
     .cq_zone = 13,
     .itu_zone = 74,
     .lat = -90.00,
     .lon = -0,
     .prefixes = "3Y,AX0,AY1Z,AY2Z,AY3Z,AY4Z,AY5Z,AY6Z,AY7Z,AY8Z,AY9Z,FT0Y,FT1Y,FT2Y,FT3Y,FT4Y,FT5Y,FT6Y,FT7Y,FT8Y,LU1Z,LU2Z,LU3Z,LU4Z,LU5Z,LU6Z,LU7Z,"
                 "LU8Z,LU9Z,RI1AN,VI0,VK0,ZL5,ZM5,ZS7,8J1RL,DP0GVN,DP1POL,EM1U,EM1UA,KC4AAA,KC4AAC,KC4USB,KC4USV,RI1ANC,RI1ANZ,VP8AL,VP8CTR"},
    {.continent = "NA", .country = "Cuba", .cq_zone = 8, .itu_zone = 11, .lat = 21.50, .lon = -80, .prefixes = "CL,CM,CO,T4"},
    {.continent = "AF", .country = "Morocco", .cq_zone = 33, .itu_zone = 37, .lat = 32.00, .lon = -5, .prefixes = "5C,5D,5E,5F,5G,CN"},
    {.continent = "SA", .country = "Bolivia", .cq_zone = 10, .itu_zone = 12, .lat = -17.00, .lon = -65, .prefixes = "CP,CP2,CP3,CP4,CP5,CP6,CP7"},
    {.continent = "EU", .country = "Portugal", .cq_zone = 14, .itu_zone = 37, .lat = 39.50, .lon = -8, .prefixes = "CQ,CR,CS,CT"},
    {.continent = "AF", .country = "Madeira Islands", .cq_zone = 33, .itu_zone = 36, .lat = 32.75, .lon = -16.95, .prefixes = "CQ2,CQ3,CQ9,CR3,CR9,CS3,CS9,CT3,CT9"},
    {.continent = "EU", .country = "Azores", .cq_zone = 14, .itu_zone = 36, .lat = 38.70, .lon = -27.23, .prefixes = "CQ1,CQ8,CR1,CR2,CR8,CS4,CS8,CT8,CU"},
    {.continent = "SA", .country = "Uruguay", .cq_zone = 13, .itu_zone = 14, .lat = -33.00, .lon = -56, .prefixes = "CV,CW,CX"},
    {.continent = "NA", .country = "Sable Island", .cq_zone = 5, .itu_zone = 9, .lat = 43.93, .lon = -59.9, .prefixes = "CY0"},
    {.continent = "NA", .country = "St. Paul Island", .cq_zone = 5, .itu_zone = 9, .lat = 47.00, .lon = -60, .prefixes = "CY9"},
    {.continent = "AF", .country = "Angola", .cq_zone = 36, .itu_zone = 52, .lat = -12.50, .lon = 18.5, .prefixes = "D2,D3"},
    {.continent = "AF", .country = "Cape Verde", .cq_zone = 35, .itu_zone = 46, .lat = 16.00, .lon = -24, .prefixes = "D4"},
    {.continent = "AF", .country = "Comoros", .cq_zone = 39, .itu_zone = 53, .lat = -11.63, .lon = 43.3, .prefixes = "D6"},
    {.continent = "EU",
     .country = "Fed. Rep. of Germany",
     .cq_zone = 14,
     .itu_zone = 28,
     .lat = 51.00,
     .lon = 10,
     .prefixes = "DA,DB,DC,DD,DE,DF,DG,DH,DI,DJ,DK,DL,DM,DN,DO,DP,DQ,DR,Y2,Y3,Y4,Y5,Y6,Y7,Y8,Y9"},
    {.continent = "OC", .country = "Philippines", .cq_zone = 27, .itu_zone = 50, .lat = 13.00, .lon = 122, .prefixes = "4D,4E,4F,4G,4H,4I,DU,DV,DW,DX,DY,DZ"},
    {.continent = "AF", .country = "Eritrea", .cq_zone = 37, .itu_zone = 48, .lat = 15.00, .lon = 39, .prefixes = "E3"},
    {.continent = "AS", .country = "Palestine", .cq_zone = 20, .itu_zone = 39, .lat = 31.28, .lon = 34.27, .prefixes = "E4"},
    {.continent = "OC", .country = "North Cook Islands", .cq_zone = 32, .itu_zone = 62, .lat = -10.02, .lon = -161.08, .prefixes = "E51PT,E51WL"},
    {.continent = "OC", .country = "South Cook Islands", .cq_zone = 32, .itu_zone = 63, .lat = -21.90, .lon = -157.93, .prefixes = "E5"},
    {.continent = "OC", .country = "Niue", .cq_zone = 32, .itu_zone = 62, .lat = -19.03, .lon = -169.85, .prefixes = "E6"},
    {.continent = "EU", .country = "Bosnia-Herzegovina", .cq_zone = 15, .itu_zone = 28, .lat = 44.32, .lon = 17.57, .prefixes = "E7"},
    {.continent = "EU", .country = "Spain", .cq_zone = 14, .itu_zone = 37, .lat = 40.37, .lon = -4.88, .prefixes = "AM,AN,AO,EA,EB,EC,ED,EE,EF,EG,EH,EA9HU"},
    {.continent = "EU", .country = "Balearic Islands", .cq_zone = 14, .itu_zone = 37, .lat = 39.60, .lon = 2.95, .prefixes = "AM6,AN6,AO6,EA6,EB6,EC6,ED6,EE6,EF6,EG6,EH6"},
    {.continent = "AF", .country = "Canary Islands", .cq_zone = 33, .itu_zone = 36, .lat = 28.32, .lon = -15.85, .prefixes = "AM8,AN8,AO8,EA8,EB8,EC8,ED8,EE8,EF8,EG8,EH8"},
    {.continent = "AF", .country = "Ceuta & Melilla", .cq_zone = 33, .itu_zone = 37, .lat = 35.90, .lon = -5.27, .prefixes = "AM9,AN9,AO9,EA9,EB9,EC9,ED9,EE9,EF9,EG9,EH9"},
    {.continent = "EU", .country = "Ireland", .cq_zone = 14, .itu_zone = 27, .lat = 53.13, .lon = -8.02, .prefixes = "EI,EJ"},
    {.continent = "AS", .country = "Armenia", .cq_zone = 21, .itu_zone = 29, .lat = 40.40, .lon = 44.9, .prefixes = "EK"},
    {.continent = "AF", .country = "Liberia", .cq_zone = 35, .itu_zone = 46, .lat = 6.50, .lon = -9.5, .prefixes = "5L,5M,6Z,A8,D5,EL"},
    {.continent = "AS", .country = "Iran", .cq_zone = 21, .itu_zone = 40, .lat = 32.00, .lon = 53, .prefixes = "9B,9C,9D,EP,EQ"},
    {.continent = "EU", .country = "Moldova", .cq_zone = 16, .itu_zone = 29, .lat = 47.00, .lon = 29, .prefixes = "ER"},
    {.continent = "EU", .country = "Estonia", .cq_zone = 15, .itu_zone = 29, .lat = 59.00, .lon = 25, .prefixes = "ES"},
    {.continent = "AF", .country = "Ethiopia", .cq_zone = 37, .itu_zone = 48, .lat = 9.00, .lon = 39, .prefixes = "9E,9F,ET"},
    {.continent = "EU", .country = "Belarus", .cq_zone = 16, .itu_zone = 29, .lat = 54.00, .lon = 28, .prefixes = "EU,EV,EW"},
    {.continent = "AS", .country = "Kyrgyzstan", .cq_zone = 17, .itu_zone = 30, .lat = 41.70, .lon = 74.13, .prefixes = "EX,EX0P,EX0Q,EX2P,EX2Q,EX6P,EX6Q,EX7P,EX7Q,EX8P,EX8Q"},
    {.continent = "AS", .country = "Tajikistan", .cq_zone = 17, .itu_zone = 30, .lat = 38.82, .lon = 71.22, .prefixes = "EY"},
    {.continent = "AS", .country = "Turkmenistan", .cq_zone = 17, .itu_zone = 30, .lat = 38.00, .lon = 58, .prefixes = "EZ"},
    {.continent = "EU", .country = "France", .cq_zone = 14, .itu_zone = 27, .lat = 46.00, .lon = 2, .prefixes = "F,HW,HX,HY,TH,TM,TP,TQ,TV"},
    {.continent = "NA", .country = "Guadeloupe", .cq_zone = 8, .itu_zone = 11, .lat = 16.13, .lon = -61.67, .prefixes = "FG,TO1T"},
    {.continent = "AF", .country = "Mayotte", .cq_zone = 39, .itu_zone = 53, .lat = -12.88, .lon = 45.15, .prefixes = "FH"},
    {.continent = "NA", .country = "St. Barthelemy", .cq_zone = 8, .itu_zone = 11, .lat = 17.90, .lon = -62.83, .prefixes = "FJ"},
    {.continent = "OC", .country = "New Caledonia", .cq_zone = 32, .itu_zone = 56, .lat = -21.50, .lon = 165.5, .prefixes = "FK"},
    {.continent = "OC", .country = "Chesterfield Islands", .cq_zone = 30, .itu_zone = 56, .lat = -19.87, .lon = 158.32, .prefixes = "TX3X"},
    {.continent = "NA", .country = "Martinique", .cq_zone = 8, .itu_zone = 11, .lat = 14.70, .lon = -61.03, .prefixes = "FM,TO5A,TO5T,TO7A"},
    {.continent = "OC", .country = "French Polynesia", .cq_zone = 32, .itu_zone = 63, .lat = -17.65, .lon = -149.4, .prefixes = "FO"},
    {.continent = "OC", .country = "Austral Islands", .cq_zone = 32, .itu_zone = 63, .lat = -23.37, .lon = -149.48, .prefixes = "TX2A"},
    {.continent = "NA", .country = "Clipperton Island", .cq_zone = 7, .itu_zone = 10, .lat = 10.28, .lon = -109.22, .prefixes = "TX5P"},
    {.continent = "OC", .country = "Marquesas Islands", .cq_zone = 31, .itu_zone = 63, .lat = -8.92, .lon = -140.07, .prefixes = "TX5EG"},
    {.continent = "NA", .country = "St. Pierre & Miquelon", .cq_zone = 5, .itu_zone = 9, .lat = 46.77, .lon = -56.2, .prefixes = "FP,TO5M"},
    {.continent = "AF", .country = "Reunion Island", .cq_zone = 39, .itu_zone = 53, .lat = -21.12, .lon = 55.48, .prefixes = "FR,TO0MPB"},
    {.continent = "NA", .country = "St. Martin", .cq_zone = 8, .itu_zone = 11, .lat = 18.08, .lon = -63.03, .prefixes = "FS"},
    {.continent = "AF", .country = "Glorioso Islands", .cq_zone = 39, .itu_zone = 53, .lat = -11.55, .lon = 47.28, .prefixes = "FT0G,FT1G,FT2G,FT3G,FT4G,FT5G,FT6G,FT7G,FT8G,FT9G"},
    {.continent = "AF",
     .country = "Juan de Nova, Europa",
     .cq_zone = 39,
     .itu_zone = 53,
     .lat = -17.05,
     .lon = 42.72,
     .prefixes = "FT0E,FT0J,FT1E,FT1J,FT2E,FT2J,FT3E,FT3J,FT4E,FT4J,FT6E,FT6J,FT7E,FT7J,FT8E,FT8J,FT9E,FT9J"},
    {.continent = "AF", .country = "Tromelin Island", .cq_zone = 39, .itu_zone = 53, .lat = -15.88, .lon = 54.5, .prefixes = "FT0T,FT1T,FT2T,FT3T,FT4T,FT5T,FT6T,FT7T,FT8T,FT9T"},
    {.continent = "AF", .country = "Crozet Island", .cq_zone = 39, .itu_zone = 68, .lat = -46.42, .lon = 51.75, .prefixes = "FT0W,FT4W,FT5W,FT8W"},
    {.continent = "AF", .country = "Kerguelen Islands", .cq_zone = 39, .itu_zone = 68, .lat = -49.00, .lon = 69.27, .prefixes = "FT0X,FT2X,FT4X,FT5X,FT8X"},
    {.continent = "AF", .country = "Amsterdam & St. Paul Is.", .cq_zone = 39, .itu_zone = 68, .lat = -37.85, .lon = 77.53, .prefixes = "FT0Z,FT1Z,FT2Z,FT3Z,FT4Z,FT5Z,FT6Z,FT7Z,FT8Z"},
    {.continent = "OC", .country = "Wallis & Futuna Islands", .cq_zone = 32, .itu_zone = 62, .lat = -13.30, .lon = -176.2, .prefixes = "FW,TW"},
    {.continent = "SA", .country = "French Guiana", .cq_zone = 9, .itu_zone = 12, .lat = 4.00, .lon = -53, .prefixes = "FY,TO1A"},
    {.continent = "EU", .country = "England", .cq_zone = 14, .itu_zone = 27, .lat = 52.77, .lon = -1.47, .prefixes = "2E,G,M"},
    {.continent = "EU", .country = "Isle of Man", .cq_zone = 14, .itu_zone = 27, .lat = 54.20, .lon = -4.53, .prefixes = "2D,GD,GT,MD,MT"},
    {.continent = "EU", .country = "Northern Ireland", .cq_zone = 14, .itu_zone = 27, .lat = 54.73, .lon = -6.68, .prefixes = "2I,GI,GN,MI,MN,GB0OGC,GB3NGI,GB4UAS,GB5BL"},
    {.continent = "EU", .country = "Jersey", .cq_zone = 14, .itu_zone = 27, .lat = 49.22, .lon = -2.18, .prefixes = "2J,GH,GJ,MH,MJ,GB19CJ"},
    {.continent = "EU",
     .country = "Shetland Islands",
     .cq_zone = 14,
     .itu_zone = 27,
     .lat = 60.50,
     .lon = -1.5,
     .prefixes = "2M0BDR,2M0BDT,2M0CPN,2M0GFC,2M0SEG,2M0SPX,2M0ZET,GB2ELH,GB3LER,GB4LER,GM0AVR,GM0CXQ,GM0EKM,GM0GFL,GM0ILB,GM0JDB,GM1FGN,GM1KKI,"
                 "GM1ZNR,GM3WHT,GM3ZET,GM4IPK,GM4JPI,GM4LBE,GM4LER,GM4PXG,GM4SLV,GM4SSA,GM4WXQ,GM4ZHL,GM6RQW,GM6YQA,GM7AFE,GM7GWW,GM8LNH,GM8MMA,"
                 "GM8YEC,GS3ZET,MM0LSM,MM0NQY,MM0VIK,MM0XAU,MM0ZAL,MM0ZCG,MM0ZRC,MM1FJM,MM3VQO,MM5PSL,MM5YLO,MM6BDU,MM6BZQ,MM6IKB,MM6IMB,MM6MFA,"
                 "MM6PTE,MM6SJK,MM6YLO,MM6ZBG,MM6ZDW,MM8A,MS0ZCG,MS0ZET"},
    {.continent = "EU",
     .country = "Scotland",
     .cq_zone = 14,
     .itu_zone = 27,
     .lat = 56.82,
     .lon = -4.18,
     .prefixes = "2A,2M,GM,GS,MA,MM,MS,GB0CLH,GB0DAM,GB0FFS,GB0NHL,GB0SSB,GB0TNL,GB19CS,GB2AL,GB2ELH,GB2JCM,GB2LBN,GB2OL,GB2OWM,GB2TNL,GB3ANG,"
                 "GB3LER,GB4LER,GB50GT,GB5RO"},
    {.continent = "EU", .country = "Guernsey", .cq_zone = 14, .itu_zone = 27, .lat = 49.45, .lon = -2.58, .prefixes = "2U,GP,GU,MP,MU,GB19CG"},
    {.continent = "EU",
     .country = "Wales",
     .cq_zone = 14,
     .itu_zone = 27,
     .lat = 52.28,
     .lon = -3.73,
     .prefixes = "2W,GC,GW,MC,MW,GB0CTK,GB0MSB,GB19CW,GB19SG,GB1BPL,GB2IMD,GB2ORM,GB2SCC,GB2TD,GB2VK"},
    {.continent = "OC", .country = "Solomon Islands", .cq_zone = 28, .itu_zone = 51, .lat = -9.00, .lon = 160, .prefixes = "H4"},
    {.continent = "OC", .country = "Temotu Province", .cq_zone = 32, .itu_zone = 51, .lat = -10.72, .lon = 165.8, .prefixes = "H40"},
    {.continent = "EU", .country = "Hungary", .cq_zone = 15, .itu_zone = 28, .lat = 47.12, .lon = 19.28, .prefixes = "HA,HG"},
    {.continent = "EU", .country = "Switzerland", .cq_zone = 14, .itu_zone = 28, .lat = 46.87, .lon = 8.12, .prefixes = "HB,HE"},
    {.continent = "EU", .country = "Liechtenstein", .cq_zone = 14, .itu_zone = 28, .lat = 47.13, .lon = 9.57, .prefixes = "HB0,HE0"},
    {.continent = "SA", .country = "Ecuador", .cq_zone = 10, .itu_zone = 12, .lat = -1.40, .lon = -78.4, .prefixes = "HC,HD"},
    {.continent = "SA", .country = "Galapagos Islands", .cq_zone = 10, .itu_zone = 12, .lat = -0.78, .lon = -91.03, .prefixes = "HC8,HD8"},
    {.continent = "NA", .country = "Haiti", .cq_zone = 8, .itu_zone = 11, .lat = 19.02, .lon = -72.18, .prefixes = "4V,HH"},
    {.continent = "NA", .country = "Dominican Republic", .cq_zone = 8, .itu_zone = 11, .lat = 19.13, .lon = -70.68, .prefixes = "HI"},
    {.continent = "SA", .country = "Colombia", .cq_zone = 9, .itu_zone = 12, .lat = 5.00, .lon = -74, .prefixes = "5J,5K,HJ,HK"},
    {.continent = "NA", .country = "San Andres & Providencia", .cq_zone = 7, .itu_zone = 11, .lat = 12.55, .lon = -81.72, .prefixes = "5J0,5K0,HJ0,HK0"},
    {.continent = "SA", .country = "Malpelo Island", .cq_zone = 9, .itu_zone = 12, .lat = 3.98, .lon = -81.58, .prefixes = "HJ0M,HK0M,HK0TU"},
    {.continent = "AS", .country = "Republic of Korea", .cq_zone = 25, .itu_zone = 44, .lat = 36.23, .lon = 127.9, .prefixes = "6K,6L,6M,6N,D7,D8,D9,DS,DT,HL,KL9K"},
    {.continent = "NA", .country = "Panama", .cq_zone = 7, .itu_zone = 11, .lat = 9.00, .lon = -80, .prefixes = "3E,3F,H3,H8,H9,HO,HP"},
    {.continent = "NA", .country = "Honduras", .cq_zone = 7, .itu_zone = 11, .lat = 15.00, .lon = -87, .prefixes = "HQ,HR"},
    {.continent = "AS", .country = "Thailand", .cq_zone = 26, .itu_zone = 49, .lat = 12.60, .lon = 99.7, .prefixes = "E2,HS"},
    {.continent = "EU", .country = "Vatican City", .cq_zone = 15, .itu_zone = 28, .lat = 41.90, .lon = 12.47, .prefixes = "HV"},
    {.continent = "AS", .country = "Saudi Arabia", .cq_zone = 21, .itu_zone = 39, .lat = 24.20, .lon = 43.83, .prefixes = "7Z,8Z,HZ"},
    {.continent = "EU", .country = "Italy", .cq_zone = 15, .itu_zone = 28, .lat = 42.82, .lon = 12.58, .prefixes = "I,4U0WFP,4U1GSC,4U5F,4U73B"},
    {.continent = "AF", .country = "African Italy", .cq_zone = 33, .itu_zone = 37, .lat = 35.67, .lon = 12.67, .prefixes = "IG9,IH9"},
    {.continent = "EU",
     .country = "Sardinia",
     .cq_zone = 15,
     .itu_zone = 28,
     .lat = 40.15,
     .lon = 9.27,
     .prefixes = "IM0,IS,IW0U,IW0V,IW0W,IW0X,IW0Y,IW0Z,II0ICH,II0IDP,II0M,II0SRE,IQ0AG,IQ0AH,IQ0AI,IQ0AK,IQ0AL,IQ0AM,IQ0EH,IQ0HO,IQ0ID,IQ0NU,"
                 "IQ0NV,IQ0OG,IQ0OH,IQ0QP,IQ0SS,IY0GA"},
    {.continent = "EU", .country = "Sicily", .cq_zone = 15, .itu_zone = 28, .lat = 37.50, .lon = 14, .prefixes = "IB9,ID9,IE9,IF9,II9,IJ9,IO9,IQ9,IR9,IT9,IU9,IW9,IY9"},
    {.continent = "AF", .country = "Djibouti", .cq_zone = 37, .itu_zone = 48, .lat = 11.75, .lon = 42.35, .prefixes = "J2"},
    {.continent = "NA", .country = "Grenada", .cq_zone = 8, .itu_zone = 11, .lat = 12.13, .lon = -61.68, .prefixes = "J3"},
    {.continent = "AF", .country = "Guinea-Bissau", .cq_zone = 35, .itu_zone = 46, .lat = 12.02, .lon = -14.8, .prefixes = "J5"},
    {.continent = "NA", .country = "St. Lucia", .cq_zone = 8, .itu_zone = 11, .lat = 13.87, .lon = -61, .prefixes = "J6"},
    {.continent = "NA", .country = "Dominica", .cq_zone = 8, .itu_zone = 11, .lat = 15.43, .lon = -61.35, .prefixes = "J7"},
    {.continent = "NA", .country = "St. Vincent", .cq_zone = 8, .itu_zone = 11, .lat = 13.23, .lon = -61.2, .prefixes = "J8"},
    {.continent = "AS",
     .country = "Japan",
     .cq_zone = 25,
     .itu_zone = 45,
     .lat = 36.40,
     .lon = 138.38,
     .prefixes = "7J,7K,7L,7M,7N,8J,8K,8L,8M,8N,JA,JE,JF,JG,JH,JI,JJ,JK,JL,JM,JN,JO,JP,JQ,JR,JS"},
    {.continent = "AS", .country = "Ogasawara", .cq_zone = 27, .itu_zone = 45, .lat = 27.05, .lon = 142.2, .prefixes = "JD1"},
    {.continent = "AS", .country = "Mongolia", .cq_zone = 23, .itu_zone = 32, .lat = 46.77, .lon = 102.17, .prefixes = "JT,JU,JV,JT2,JT3,JU2,JU3,JV2,JV3"},
    {.continent = "EU", .country = "Svalbard", .cq_zone = 40, .itu_zone = 18, .lat = 78.00, .lon = 16, .prefixes = "JW"},
    {.continent = "EU", .country = "Jan Mayen", .cq_zone = 40, .itu_zone = 18, .lat = 71.05, .lon = -8.28, .prefixes = "JX"},
    {.continent = "AS", .country = "Jordan", .cq_zone = 20, .itu_zone = 39, .lat = 31.18, .lon = 36.42, .prefixes = "JY"},
    {.continent = "NA",
     .country = "United States",
     .cq_zone = 5,
     .itu_zone = 8,
     .lat = 37.53,
     .lon = -91.67,
     .prefixes = "AA,AB,AC,AD,AE,AF,AG,AI,AJ,AK,K,N,W,4U1WB,AA0O,AA2IL,AA4DD,AA4R,AA7TV,AA8R,AA9HQ,AB4B,AB4EJ,AB4GG,AB4IQ,AB4KJ,AB5OR,AB8RL,AB9DU,AC4CA,"
                 "AC4G,AC4GW,AC6WI,AC6ZM,AC7AF,AC7P,AC8Y,AD1C,AD4EB,AD4TA,AD8J,AE7AP,AG1RL,AG4W,AG5Z,AH0U,AH2O,AH6AX,AI4DB,AI9K,AJ4A,AJ4F,AJ7G,AL0A,"
                 "AL4B,AL7GI,AL7RF,G8ERJ,K0ACP,K0AU,K0EJ,K0IP,K0LUZ,K0NW,K0PJ,K0SN,K0TQ,K0ZR,K1DW,K1GU,K1KD,K1LT,K1PQS,K2DSW,K2GMY,K2KR,K2PM,K2PO,K2RD,"
                 "K2UR,K2VV,K3DMG,K3EST,K3IE,K3JWI,K3PA,K3WT,K3WYC,K3YP,K4AB,K4AFE,K4AMC,K4AVX,K4BX,K4CWW,K4DG,K4DXV,K4FN,K4FT,K4HAL,K4IE,K4IU,K4LPQ,"
                 "K4MMW,K4OAQ,K4OWR,K4RO,K4TCG,K4TZ,K4WG,K4WI,K4WW,K4XU,K4YJ,K4ZGB,K5AUP,K5EK,K5KG,K5OA,K5VIP,K5WK,K5ZD,K5ZM,K6BFL,K6XT,K7ABV,K7BV,K7DR,"
                 "K7IA,K7JOE,K7OM,K7QA,K7SCX,K7SV,K7TD,K7VU,K7ZYV,K8AC,K8ARY,K8CN,K8FU,K8GU,K8IA,K8JQ,K8LBQ,K8LF,K8LS,K8NYG,K8PO,K8SYH,K9DR,K9DU,K9JF,"
                 "K9JM,K9OM,K9OR,K9OZ,K9RS,K9WA,K9WZB,K9YC,KA1KIX,KA3MTT,KA4BVG,KA4GAV,KA4OTB,KA8HDE,KA8Q,KA9FOX,KA9VVQ,KB7Q,KB8KMH,KB8V,KB9S,KC2LM,"
                 "KC4HW,KC4NX,KC4SAW,KC4TEO,KC4WQ,KC6X,KC7QY,KC9K,KD0EE,KD2KW,KD4ADC,KD4EE,KD5DD,KD5M,KD9BSO,KD9LA,KE0L,KE0YI,KE1B,KE2VB,KE3K,KE4KY,"
                 "KE8FT,KF4AV,KF5BA,KF5MU,KG4CUY,KG5HVO,KG6MC,KH2D,KH2GM,KH6CT,KH6M,KH6VM,KH6XS,KI6DY,KI6QDH,KJ4AOM,KJ4IWZ,KJ4KKD,KJ4M,KK9A,KK9N,KL0MP,"
                 "KL2RA,KL4CZ,KL7IKV,KL7JR,KL7QW,KL7SK,KM4FO,KM4JA,KM7W,KM9R,KN1CBR,KN4DUA,KN4FRG,KN8U,KO0Z,KO4OL,KO9V,KP2XX,KP4KD,KP4M,KQ9J,KR4F,KS0CW,"
                 "KS4L,KS4X,KS7T,KS9W,KT3M,KT4O,KT4RR,KU1CW,KU8E,KV8S,KW4J,KW7D,KW7Q,KX4FR,KX4WB,KX4X,KY0Q,KY4F,KY4KD,KZ1W,KZ9V,N0SMX,N1JM,N1RU,N1XK,"
                 "N2AU,N2BJ,N2IC,N2NS,N3BB,N3BUO,N3CI,N3RC,N3ZZ,N4ARO,N4AU,N4BAA,N4BCB,N4BCD,N4DW,N4HAI,N4HID,N4KC,N4KG,N4KH,N4NA,N4NO,N4OGW,N4QS,N4SL,"
                 "N4TZ,N4UC,N4UW,N4VI,N4VV,N4WE,N4ZY,N4ZZ,N5CR,N5CW,N5DX,N5KO,N5RP,N5SMQ,N5TB,N5TOO,N5YT,N5ZO,N6AR,N6DW,N7DF,N7DR,N7FLT,N7IP,N7IV,N7MZW,"
                 "N7NG,N7RCS,N7US,N7WY,N7ZZ,N8AID,N8GU,N8II,N8NA,N8OO,N8PR,N8RA,N8WXQ,N9CIQ,N9GB,N9JF,N9NA,N9NB,N9NC,N9NM,N9RV,N9SB,N9VPV,NA2CC,NA4K,"
                 "NA5NN,NA9RB,ND2T,ND3N,ND4Y,NE8P,NE9U,NF4J,NG7A,NH6T,NI7R,NJ8J,NK8Q,NL7CQ,NL7D,NL7QC,NL7XM,NN1N,NN4SA,NN5O,NN7A,NN7CW,NO9E,NP2GG,NP3K,"
                 "NQ6N,NR5W,NR7DX,NS4X,NW7R,NW8U,NX1P,NY6DX,W0BR,W0CN,W0JX,W0MHK,W0PV,W0QQG,W0RIC,W0SK,W0ZP,W1GKT,W1NN,W1PDI,W1PR,W1RH,W1SRD,W2ACY,W2FV,"
                 "W2GS,W2VJN,W3DQN,W3HDH,W3HKK,W3TB,W4BCG,W4DAN,W4ER,W4GKM,W4GZX,W4JSI,W4KW,W4LC,W4LID,W4NBS,W4NI,W4NZ,W4PF,W4RJJ,W4RYW,W4TLK,W4TTM,"
                 "W4UAL,W4UT,W5HIO,W5JR,W5MX,W5NZ,W5UE,W5VE,W5VS,W5XB,W6GMT,W6KGP,W6LFB,W6NWS,W6RLL,W6UB,W6XR,W6YTG,W7DO,W7EE,W7HJ,W7IGC,W7II,W7IY,"
                 "W7KAM,W7KF,W7RY,W7SE,W7UT,W7XU,W8AT,W8BAR,W8FJ,W8FN,W8HAP,W8HGH,W8KA,W8LYJ,W8OV,W8TK,W8WEJ,W9CF,W9DC,W9ET,W9IL,W9JA,W9JEF,W9KKN,W9MAF,"
                 "W9PL,W9RM,W9RNY,WA0WWW,WA1FCN,WA1UJU,WA2VYA,WA3C,WA4JA,WA5POK,WA8KAN,WA8OJR,WA8ZBT,WB3JFS,WB4YDL,WB4YDY,WB7S,WB8BPU,WB8IMY,WB8YYY,"
                 "WB9QAF,WC7S,WD5F,WE6EZ,WF3H,WF7T,WF9A,WH0AI,WH6AQ,WH6LE,WH7R,WI4T,WJ9B,WL7OU,WM5DX,WN1G,WN7S,WP2B,WP3ME,WQ3U,WQ5L,WQ9T,WR5J,WS6K,WS6X,"
                 "WS9M,WT2P,WT8WV,WU0B,WU9B,WV4P,WV8SW,WW4R,WW5M,WX4W,WX5S,WY6K,WY7FD,WZ1Y,WZ4F,WZ7I,WZ8T"},
    {.continent = "NA", .country = "Guantanamo Bay", .cq_zone = 8, .itu_zone = 11, .lat = 20.00, .lon = -75, .prefixes = "KG4,KG4AY,KG4WH"},
    {.continent = "OC", .country = "Mariana Islands", .cq_zone = 27, .itu_zone = 64, .lat = 15.18, .lon = 145.72, .prefixes = "AH0,KH0,NH0,WH0,K8KH,K8RN,NH2B"},
    {.continent = "OC", .country = "Baker & Howland Islands", .cq_zone = 31, .itu_zone = 61, .lat = 0.00, .lon = -176, .prefixes = "AH1,KH1,NH1,WH1"},
    {.continent = "OC",
     .country = "Guam",
     .cq_zone = 27,
     .itu_zone = 64,
     .lat = 13.37,
     .lon = 144.7,
     .prefixes = "AH2,KH2,NH2,WH2,AH0FM,KB5OXR,KF7BMU,KG6DX,KG6JDX,KH0UM,KJ6AYQ,KK6GVF,N0RY,NH0Q"},
    {.continent = "OC", .country = "Johnston Island", .cq_zone = 31, .itu_zone = 61, .lat = 16.72, .lon = -169.53, .prefixes = "AH3,KH3,NH3,WH3"},
    {.continent = "OC", .country = "Midway Island", .cq_zone = 31, .itu_zone = 61, .lat = 28.20, .lon = -177.37, .prefixes = "AH4,KH4,NH4,WH4"},
    {.continent = "OC", .country = "Palmyra & Jarvis Islands", .cq_zone = 31, .itu_zone = 61, .lat = 5.87, .lon = -162.07, .prefixes = "AH5,KH5,NH5,WH5"},
    {.continent = "OC",
     .country = "Hawaii",
     .cq_zone = 31,
     .itu_zone = 61,
     .lat = 21.12,
     .lon = -157.48,
     .prefixes = "AH6,AH7,KH6,KH7,NH6,NH7,WH6,WH7,AA7LE,AC7N,AC9PT,AK2J,K2GT,K3NW,K4XS,K6BU,K6IJ,K6MIO,K9FD,KA7BSK,KB1UHL,KB6EGA,KB6INB,KB7WDC,"
                 "KC0YIH,KD0OXU,KD4NFW,KD6NVX,KE0JSB,KE4DYE,KE5VQB,KE6MKW,KE7LWN,KE7QML,KF6RLP,KH0WJ,KH3AE,KH8Z,KI4YOG,KI7AUZ,KK6RM,KL0TK,KM6IK,"
                 "KQ6M,KR1LLR,N1CBF,N1TEE,N1VOP,N3BQY,N3FUR,N3GWR,N7BMD,W6KEV,W7NX,WA6IIQ,WA8JQP,WB0TZQ,WB4JTT,WB6PIO,WB8NCD,WD0LFN,WY6F"},
    {.continent = "OC", .country = "Kure Island", .cq_zone = 31, .itu_zone = 61, .lat = 29.00, .lon = -178, .prefixes = "AH7K,KH7K,NH7K,WH7K"},
    {.continent = "OC", .country = "American Samoa", .cq_zone = 32, .itu_zone = 62, .lat = -14.32, .lon = -170.78, .prefixes = "AH8,KH8,NH8,WH8"},
    {.continent = "OC", .country = "Wake Island", .cq_zone = 31, .itu_zone = 65, .lat = 19.28, .lon = 166.63, .prefixes = "AH9,KH9,NH9,WH9"},
    {.continent = "NA",
     .country = "Alaska",
     .cq_zone = 1,
     .itu_zone = 1,
     .lat = 61.40,
     .lon = -148.87,
     .prefixes = "AL,KL,NL,WL,AJ4MY,AK4CM,K1KAO,K4HOE,K4PSG,K7SGA,KA1NCN,KB3CYB,KB4DX,KC0GLN,KC0YSW,KC5YOX,KC5ZAA,KD0JJB,KD5WYP,KD7BBX,KF3L,"
                 "KF6IAO,KF6RMG,KF7GKY,KG5MIO,KG7JVJ,KG7OUF,KI6BGR,KJ4YOY,KJ6DCH,KN4HGD,KW1W,N1NDA,N1TX,N6QEK,N6ZZX,N7DBN,N7XEA,N8SUG,NM0H,NU9Q,"
                 "NW7F,W1JM,W1LYD,W3MKG,WH6DPL,WH6GBB,WH6GCO"},
    {.continent = "NA", .country = "Navassa Island", .cq_zone = 8, .itu_zone = 11, .lat = 18.40, .lon = -75, .prefixes = "KP1,NP1,WP1"},
    {.continent = "NA", .country = "US Virgin Islands", .cq_zone = 8, .itu_zone = 11, .lat = 17.73, .lon = -64.8, .prefixes = "KP2,NP2,WP2,K8RF,KV4FZ,W4LIS,W9Q"},
    {.continent = "NA",
     .country = "Puerto Rico",
     .cq_zone = 8,
     .itu_zone = 11,
     .lat = 18.18,
     .lon = -66.55,
     .prefixes = "KP3,KP4,NP3,NP4,WP3,WP4,K4LCR,KB0JRR,KC2TE,KC2VCR,KC8BFN,KD9GIZ,KG4GYO,KK4DCX,KP2Z,N2IBR,N4CIE,N6NVD"},
    {.continent = "NA", .country = "Desecheo Island", .cq_zone = 8, .itu_zone = 11, .lat = 18.08, .lon = -67.88, .prefixes = "KP5,NP5,WP5"},
    {.continent = "EU", .country = "Norway", .cq_zone = 14, .itu_zone = 18, .lat = 61.00, .lon = 9, .prefixes = "LA,LB,LC,LD,LE,LF,LG,LH,LI,LJ,LK,LL,LM,LN"},
    {.continent = "SA",
     .country = "Argentina",
     .cq_zone = 13,
     .itu_zone = 14,
     .lat = -34.80,
     .lon = -65.92,
     .prefixes = "AY,AZ,L1,L2,L3,L4,L5,L6,L7,L8,L9,LO,LP,LQ,LR,LS,LT,LU,LV,LW,AY0V,AY0W,AY0X,AY0Y,AY1V,AY1W,AY1X,AY1Y,AY2V,AY2W,AY2X,AY2Y,AY3V,AY3W,"
                 "AY3X,AY3Y,AY4V,AY4W,AY4X,AY4Y,AY5V,AY5W,AY5X,AY5Y,AY6V,AY6W,AY6X,AY6Y,AY7V,AY7W,AY7X,AY7Y,AY8V,AY8W,AY8X,AY8Y,AY9V,AY9W,AY9X,AY9Y,"
                 "AZ0V,AZ0W,AZ0X,AZ0Y,AZ1V,AZ1W,AZ1X,AZ1Y,AZ2V,AZ2W,AZ2X,AZ2Y,AZ3V,AZ3W,AZ3X,AZ3Y,AZ4V,AZ4W,AZ4X,AZ4Y,AZ5V,AZ5W,AZ5X,AZ5Y,AZ6V,AZ6W,"
                 "AZ6X,AZ6Y,AZ7V,AZ7W,AZ7X,AZ7Y,AZ8V,AZ8W,AZ8X,AZ8Y,AZ9V,AZ9W,AZ9X,AZ9Y,L20V,L20W,L20X,L20Y,L21V,L21W,L21X,L21Y,L22V,L22W,L22X,L22Y,"
                 "L23V,L23W,L23X,L23Y,L24V,L24W,L24X,L24Y,L25V,L25W,L25X,L25Y,L26V,L26W,L26X,L26Y,L27V,L27W,L27X,L27Y,L28V,L28W,L28X,L28Y,L29V,L29W,"
                 "L29X,L29Y,L30V,L30W,L30X,L30Y,L31V,L31W,L31X,L31Y,L32V,L32W,L32X,L32Y,L33V,L33W,L33X,L33Y,L34V,L34W,L34X,L34Y,L35V,L35W,L35X,L35Y,"
                 "L36V,L36W,L36X,L36Y,L37V,L37W,L37X,L37Y,L38V,L38W,L38X,L38Y,L39V,L39W,L39X,L39Y,L40V,L40W,L40X,L40Y,L41V,L41W,L41X,L41Y,L42V,L42W,"
                 "L42X,L42Y,L43V,L43W,L43X,L43Y,L44V,L44W,L44X,L44Y,L45V,L45W,L45X,L45Y,L46V,L46W,L46X,L46Y,L47V,L47W,L47X,L47Y,L48V,L48W,L48X,L48Y,"
                 "L49V,L49W,L49X,L49Y,L50V,L50W,L50X,L50Y,L51V,L51W,L51X,L51Y,L52V,L52W,L52X,L52Y,L53V,L53W,L53X,L53Y,L54V,L54W,L54X,L54Y,L55V,L55W,"
                 "L55X,L55Y,L56V,L56W,L56X,L56Y,L57V,L57W,L57X,L57Y,L58V,L58W,L58X,L58Y,L59V,L59W,L59X,L59Y,L60V,L60W,L60X,L60Y,L61V,L61W,L61X,L61Y,"
                 "L62V,L62W,L62X,L62Y,L63V,L63W,L63X,L63Y,L64V,L64W,L64X,L64Y,L65V,L65W,L65X,L65Y,L66V,L66W,L66X,L66Y,L67V,L67W,L67X,L67Y,L68V,L68W,"
                 "L68X,L68Y,L69V,L69W,L69X,L69Y,L70V,L70W,L70X,L70Y,L71V,L71W,L71X,L71Y,L72V,L72W,L72X,L72Y,L73V,L73W,L73X,L73Y,L74V,L74W,L74X,L74Y,"
                 "L75V,L75W,L75X,L75Y,L76V,L76W,L76X,L76Y,L77V,L77W,L77X,L77Y,L78V,L78W,L78X,L78Y,L79V,L79W,L79X,L79Y,L80V,L80W,L80X,L80Y,L81V,L81W,"
                 "L81X,L81Y,L82V,L82W,L82X,L82Y,L83V,L83W,L83X,L83Y,L84V,L84W,L84X,L84Y,L85V,L85W,L85X,L85Y,L86V,L86W,L86X,L86Y,L87V,L87W,L87X,L87Y,"
                 "L88V,L88W,L88X,L88Y,L89V,L89W,L89X,L89Y,L90V,L90W,L90X,L90Y,L91V,L91W,L91X,L91Y,L92V,L92W,L92X,L92Y,L93V,L93W,L93X,L93Y,L94V,L94W,"
                 "L94X,L94Y,L95V,L95W,L95X,L95Y,L96V,L96W,L96X,L96Y,L97V,L97W,L97X,L97Y,L98V,L98W,L98X,L98Y,L99V,L99W,L99X,L99Y,LO0V,LO0W,LO0X,LO0Y,"
                 "LO1V,LO1W,LO1X,LO1Y,LO2V,LO2W,LO2X,LO2Y,LO3V,LO3W,LO3X,LO3Y,LO4V,LO4W,LO4X,LO4Y,LO5V,LO5W,LO5X,LO5Y,LO6V,LO6W,LO6X,LO6Y,LO7V,LO7W,"
                 "LO7X,LO7Y,LO8V,LO8W,LO8X,LO8Y,LO9V,LO9W,LO9X,LO9Y,LP0V,LP0W,LP0X,LP0Y,LP1V,LP1W,LP1X,LP1Y,LP2V,LP2W,LP2X,LP2Y,LP3V,LP3W,LP3X,LP3Y,"
                 "LP4V,LP4W,LP4X,LP4Y,LP5V,LP5W,LP5X,LP5Y,LP6V,LP6W,LP6X,LP6Y,LP7V,LP7W,LP7X,LP7Y,LP8V,LP8W,LP8X,LP8Y,LP9V,LP9W,LP9X,LP9Y,LQ0V,LQ0W,"
                 "LQ0X,LQ0Y,LQ1V,LQ1W,LQ1X,LQ1Y,LQ2V,LQ2W,LQ2X,LQ2Y,LQ3V,LQ3W,LQ3X,LQ3Y,LQ4V,LQ4W,LQ4X,LQ4Y,LQ5V,LQ5W,LQ5X,LQ5Y,LQ6V,LQ6W,LQ6X,LQ6Y,"
                 "LQ7V,LQ7W,LQ7X,LQ7Y,LQ8V,LQ8W,LQ8X,LQ8Y,LQ9V,LQ9W,LQ9X,LQ9Y,LR0V,LR0W,LR0X,LR0Y,LR1V,LR1W,LR1X,LR1Y,LR2V,LR2W,LR2X,LR2Y,LR3V,LR3W,"
                 "LR3X,LR3Y,LR4V,LR4W,LR4X,LR4Y,LR5V,LR5W,LR5X,LR5Y,LR6V,LR6W,LR6X,LR6Y,LR7V,LR7W,LR7X,LR7Y,LR8V,LR8W,LR8X,LR8Y,LR9V,LR9W,LR9X,LR9Y,"
                 "LS0V,LS0W,LS0X,LS0Y,LS1V,LS1W,LS1X,LS1Y,LS2V,LS2W,LS2X,LS2Y,LS3V,LS3W,LS3X,LS3Y,LS4V,LS4W,LS4X,LS4Y,LS5V,LS5W,LS5X,LS5Y,LS6V,LS6W,"
                 "LS6X,LS6Y,LS7V,LS7W,LS7X,LS7Y,LS8V,LS8W,LS8X,LS8Y,LS9V,LS9W,LS9X,LS9Y,LT0V,LT0W,LT0X,LT0Y,LT1V,LT1W,LT1X,LT1Y,LT2V,LT2W,LT2X,LT2Y,"
                 "LT3V,LT3W,LT3X,LT3Y,LT4V,LT4W,LT4X,LT4Y,LT5V,LT5W,LT5X,LT5Y,LT6V,LT6W,LT6X,LT6Y,LT7V,LT7W,LT7X,LT7Y,LT8V,LT8W,LT8X,LT8Y,LT9V,LT9W,"
                 "LT9X,LT9Y,LU0V,LU0W,LU0X,LU0Y,LU1V,LU1W,LU1X,LU1Y,LU2V,LU2W,LU2X,LU2Y,LU3V,LU3W,LU3X,LU3Y,LU4V,LU4W,LU4X,LU4Y,LU5V,LU5W,LU5X,LU5Y,"
                 "LU6V,LU6W,LU6X,LU6Y,LU7V,LU7W,LU7X,LU7Y,LU8V,LU8W,LU8X,LU8Y,LU9V,LU9W,LU9X,LU9Y,LV0V,LV0W,LV0X,LV0Y,LV1V,LV1W,LV1X,LV1Y,LV2V,LV2W,"
                 "LV2X,LV2Y,LV3V,LV3W,LV3X,LV3Y,LV4V,LV4W,LV4X,LV4Y,LV5V,LV5W,LV5X,LV5Y,LV6V,LV6W,LV6X,LV6Y,LV7V,LV7W,LV7X,LV7Y,LV8V,LV8W,LV8X,LV8Y,"
                 "LV9V,LV9W,LV9X,LV9Y,LW0V,LW0W,LW0X,LW0Y,LW1V,LW1W,LW1X,LW1Y,LW2V,LW2W,LW2X,LW2Y,LW3V,LW3W,LW3X,LW3Y,LW4V,LW4W,LW4X,LW4Y,LW5V,LW5W,"
                 "LW5X,LW5Y,LW6V,LW6W,LW6X,LW6Y,LW7V,LW7W,LW7X,LW7Y,LW8V,LW8W,LW8X,LW8Y,LW9V,LW9W,LW9X,LW9Y"},
    {.continent = "EU", .country = "Luxembourg", .cq_zone = 14, .itu_zone = 27, .lat = 50.00, .lon = 6, .prefixes = "LX"},
    {.continent = "EU", .country = "Lithuania", .cq_zone = 15, .itu_zone = 29, .lat = 55.45, .lon = 23.63, .prefixes = "LY"},
    {.continent = "EU", .country = "Bulgaria", .cq_zone = 20, .itu_zone = 28, .lat = 42.83, .lon = 25.08, .prefixes = "LZ"},
    {.continent = "SA", .country = "Peru", .cq_zone = 10, .itu_zone = 12, .lat = -10.00, .lon = -76, .prefixes = "4T,OA,OB,OC"},
    {.continent = "AS", .country = "Lebanon", .cq_zone = 20, .itu_zone = 39, .lat = 33.83, .lon = 35.83, .prefixes = "OD"},
    {.continent = "EU", .country = "Austria", .cq_zone = 15, .itu_zone = 28, .lat = 47.33, .lon = 13.33, .prefixes = "OE,4U0R,4U1A,4U1VIC,4U2U,4U500M,4Y1A,C7A"},
    {.continent = "EU", .country = "Finland", .cq_zone = 15, .itu_zone = 18, .lat = 63.78, .lon = 27.08, .prefixes = "OF,OG,OH,OI,OJ"},
    {.continent = "EU", .country = "Aland Islands", .cq_zone = 15, .itu_zone = 18, .lat = 60.13, .lon = 20.37, .prefixes = "OF0,OG0,OH0,OI0"},
    {.continent = "EU", .country = "Market Reef", .cq_zone = 15, .itu_zone = 18, .lat = 60.00, .lon = 19, .prefixes = "OJ0"},
    {.continent = "EU", .country = "Czech Republic", .cq_zone = 15, .itu_zone = 28, .lat = 50.00, .lon = 16, .prefixes = "OK,OL"},
    {.continent = "EU", .country = "Slovak Republic", .cq_zone = 15, .itu_zone = 28, .lat = 49.00, .lon = 20, .prefixes = "OM"},
    {.continent = "EU", .country = "Belgium", .cq_zone = 14, .itu_zone = 27, .lat = 50.70, .lon = 4.85, .prefixes = "ON,OO,OP,OQ,OR,OS,OT"},
    {.continent = "NA", .country = "Greenland", .cq_zone = 40, .itu_zone = 5, .lat = 74.00, .lon = -42.78, .prefixes = "OX,XP"},
    {.continent = "EU", .country = "Faroe Islands", .cq_zone = 14, .itu_zone = 18, .lat = 62.07, .lon = -6.93, .prefixes = "OW,OY"},
    {.continent = "EU", .country = "Denmark", .cq_zone = 14, .itu_zone = 18, .lat = 56.00, .lon = 10, .prefixes = "5P,5Q,OU,OV,OZ"},
    {.continent = "OC", .country = "Papua New Guinea", .cq_zone = 28, .itu_zone = 51, .lat = -9.50, .lon = 147.12, .prefixes = "P2"},
    {.continent = "SA", .country = "Aruba", .cq_zone = 9, .itu_zone = 11, .lat = 12.53, .lon = -69.98, .prefixes = "P4"},
    {.continent = "AS", .country = "DPR of Korea", .cq_zone = 25, .itu_zone = 44, .lat = 39.78, .lon = 126.3, .prefixes = "P5,P6,P7,P8,P9"},
    {.continent = "EU", .country = "Netherlands", .cq_zone = 14, .itu_zone = 27, .lat = 52.28, .lon = 5.47, .prefixes = "PA,PB,PC,PD,PE,PF,PG,PH,PI"},
    {.continent = "SA", .country = "Curacao", .cq_zone = 9, .itu_zone = 11, .lat = 12.17, .lon = -69, .prefixes = "PJ2"},
    {.continent = "SA", .country = "Bonaire", .cq_zone = 9, .itu_zone = 11, .lat = 12.20, .lon = -68.25, .prefixes = "PJ4"},
    {.continent = "NA", .country = "Saba & St. Eustatius", .cq_zone = 8, .itu_zone = 11, .lat = 17.57, .lon = -63.1, .prefixes = "PJ5,PJ6"},
    {.continent = "NA", .country = "Sint Maarten", .cq_zone = 8, .itu_zone = 11, .lat = 18.07, .lon = -63.07, .prefixes = "PJ0,PJ7,PJ8"},
    {.continent = "SA",
     .country = "Brazil",
     .cq_zone = 11,
     .itu_zone = 15,
     .lat = -10.00,
     .lon = -53,
     .prefixes = "PP,PQ,PR,PS,PT,PU,PV,PW,PX,PY,ZV,ZW,ZX,ZY,ZZ,PP6,PP7,PP8,PQ2,PQ8,PR7,PR8,PS7,PS8,PT2,PT7,PT8,PV8,PW8,PY6,PY7,PY8,PY9"},
    {.continent = "SA",
     .country = "Fernando de Noronha",
     .cq_zone = 11,
     .itu_zone = 13,
     .lat = -3.85,
     .lon = -32.43,
     .prefixes = "PP0F,PP0ZF,PQ0F,PQ0ZF,PR0F,PR0ZF,PS0F,PS0ZF,PT0F,PT0ZF,PU0F,PU0ZF,PV0F,PV0ZF,PW0F,PW0ZF,PX0F,PX0ZF,PY0F,PY0Z,ZV0F,ZV0ZF,ZW0F,"
                 "ZW0ZF,ZX0F,ZX0ZF,ZY0F,ZY0Z,ZZ0F,ZZ0ZF,PP0R,PP0ZR,PQ0R,PQ0ZR,PR0R,PR0ZR,PS0R,PS0ZR,PT0R,PT0ZR,PU0R,PU0ZR,PV0R,PV0ZR,PW0R,PW0ZR,"
                 "PX0R,PX0ZR,PY0R,ZV0R,ZV0ZR,ZW0R,ZW0ZR,ZX0R,ZX0ZR,ZY0R,ZZ0R,ZZ0ZR"},
    {.continent = "SA",
     .country = "St. Peter & St. Paul",
     .cq_zone = 11,
     .itu_zone = 13,
     .lat = 0.00,
     .lon = -29,
     .prefixes = "PP0S,PP0ZS,PQ0S,PQ0ZS,PR0S,PR0ZS,PS0S,PS0ZS,PT0S,PT0ZS,PU0S,PU0ZS,PV0S,PV0ZS,PW0S,PW0ZS,PX0S,PX0ZS,PY0S,PY0ZS,ZV0S,ZV0ZS,ZW0S,"
                 "ZW0ZS,ZX0S,ZX0ZS,ZY0S,ZY0ZS,ZZ0S,ZZ0ZS"},
    {.continent = "SA",
     .country = "Trindade & Martim Vaz",
     .cq_zone = 11,
     .itu_zone = 15,
     .lat = -20.50,
     .lon = -29.32,
     .prefixes = "PP0T,PP0ZT,PQ0T,PQ0ZT,PR0T,PR0ZT,PS0T,PS0ZT,PT0T,PT0ZT,PU0T,PU0ZT,PV0T,PV0ZT,PW0T,PW0ZT,PX0T,PX0ZT,PY0T,PY0ZT,ZV0T,ZV0ZT,ZW0T,"
                 "ZW0ZT,ZX0T,ZX0ZT,ZY0T,ZY0ZT,ZZ0T,ZZ0ZT"},
    {.continent = "SA", .country = "Suriname", .cq_zone = 9, .itu_zone = 12, .lat = 4.00, .lon = -56, .prefixes = "PZ"},
    {.continent = "EU", .country = "Franz Josef Land", .cq_zone = 40, .itu_zone = 75, .lat = 80.68, .lon = 49.92, .prefixes = "RI1F"},
    {.continent = "AF", .country = "Western Sahara", .cq_zone = 33, .itu_zone = 46, .lat = 24.82, .lon = -13.85, .prefixes = "S0"},
    {.continent = "AS", .country = "Bangladesh", .cq_zone = 22, .itu_zone = 41, .lat = 24.12, .lon = 89.65, .prefixes = "S2,S3"},
    {.continent = "EU", .country = "Slovenia", .cq_zone = 15, .itu_zone = 28, .lat = 46.00, .lon = 14, .prefixes = "S5"},
    {.continent = "AF", .country = "Seychelles", .cq_zone = 39, .itu_zone = 53, .lat = -4.67, .lon = 55.47, .prefixes = "S7"},
    {.continent = "AF", .country = "Sao Tome & Principe", .cq_zone = 36, .itu_zone = 47, .lat = 0.22, .lon = 6.57, .prefixes = "S9"},
    {.continent = "EU", .country = "Sweden", .cq_zone = 14, .itu_zone = 18, .lat = 61.20, .lon = 14.57, .prefixes = "7S,8S,SA,SB,SC,SD,SE,SF,SG,SH,SI,SJ,SK,SL,SM"},
    {.continent = "EU", .country = "Poland", .cq_zone = 15, .itu_zone = 28, .lat = 52.28, .lon = 18.67, .prefixes = "3Z,HF,SN,SO,SP,SQ,SR"},
    {.continent = "AF", .country = "Sudan", .cq_zone = 34, .itu_zone = 48, .lat = 14.47, .lon = 28.62, .prefixes = "6T,6U,ST"},
    {.continent = "AF", .country = "Egypt", .cq_zone = 34, .itu_zone = 38, .lat = 26.28, .lon = 28.6, .prefixes = "6A,6B,SS,SU"},
    {.continent = "EU", .country = "Greece", .cq_zone = 20, .itu_zone = 28, .lat = 39.78, .lon = 21.78, .prefixes = "J4,SV,SW,SX,SY,SZ"},
    {.continent = "EU", .country = "Dodecanese", .cq_zone = 20, .itu_zone = 28, .lat = 36.17, .lon = 27.93, .prefixes = "J45,SV5,SW5,SX5,SY5,SZ5"},
    {.continent = "EU", .country = "Crete", .cq_zone = 20, .itu_zone = 28, .lat = 35.23, .lon = 24.78, .prefixes = "J49,SV9,SW9,SX9,SY9,SZ9"},
    {.continent = "OC", .country = "Tuvalu", .cq_zone = 31, .itu_zone = 65, .lat = -8.50, .lon = 179.2, .prefixes = "T2"},
    {.continent = "OC", .country = "Western Kiribati", .cq_zone = 31, .itu_zone = 65, .lat = 1.42, .lon = 173, .prefixes = "T30"},
    {.continent = "OC", .country = "Central Kiribati", .cq_zone = 31, .itu_zone = 62, .lat = -2.83, .lon = -171.72, .prefixes = "T31"},
    {.continent = "OC", .country = "Eastern Kiribati", .cq_zone = 31, .itu_zone = 61, .lat = 1.80, .lon = -157.35, .prefixes = "T32"},
    {.continent = "OC", .country = "Banaba Island", .cq_zone = 31, .itu_zone = 65, .lat = -0.88, .lon = 169.53, .prefixes = "T33"},
    {.continent = "AF", .country = "Somalia", .cq_zone = 37, .itu_zone = 48, .lat = 2.03, .lon = 45.35, .prefixes = "6O,T5"},
    {.continent = "EU", .country = "San Marino", .cq_zone = 15, .itu_zone = 28, .lat = 43.95, .lon = 12.45, .prefixes = "T7"},
    {.continent = "OC", .country = "Palau", .cq_zone = 27, .itu_zone = 64, .lat = 7.45, .lon = 134.53, .prefixes = "T8"},
    {.continent = "AS", .country = "Asiatic Turkey", .cq_zone = 20, .itu_zone = 39, .lat = 39.18, .lon = 35.65, .prefixes = "TA,TB,TC,YM,TA1D/3,TA1D/4"},
    {.continent = "EU", .country = "European Turkey", .cq_zone = 20, .itu_zone = 39, .lat = 41.02, .lon = 28.97, .prefixes = "TA1,TB1,TC1,YM1"},
    {.continent = "EU", .country = "Iceland", .cq_zone = 40, .itu_zone = 17, .lat = 64.80, .lon = -18.73, .prefixes = "TF"},
    {.continent = "NA", .country = "Guatemala", .cq_zone = 7, .itu_zone = 11, .lat = 15.50, .lon = -90.3, .prefixes = "TD,TG"},
    {.continent = "NA", .country = "Costa Rica", .cq_zone = 7, .itu_zone = 11, .lat = 10.00, .lon = -84, .prefixes = "TE,TI"},
    {.continent = "NA", .country = "Cocos Island", .cq_zone = 7, .itu_zone = 11, .lat = 5.52, .lon = -87.05, .prefixes = "TE9,TI9"},
    {.continent = "AF", .country = "Cameroon", .cq_zone = 36, .itu_zone = 47, .lat = 5.38, .lon = 11.87, .prefixes = "TJ"},
    {.continent = "EU", .country = "Corsica", .cq_zone = 15, .itu_zone = 28, .lat = 42.00, .lon = 9, .prefixes = "TK"},
    {.continent = "AF", .country = "Central African Republic", .cq_zone = 36, .itu_zone = 47, .lat = 6.75, .lon = 20.33, .prefixes = "TL"},
    {.continent = "AF", .country = "Republic of the Congo", .cq_zone = 36, .itu_zone = 52, .lat = -1.02, .lon = 15.37, .prefixes = "TN"},
    {.continent = "AF", .country = "Gabon", .cq_zone = 36, .itu_zone = 52, .lat = -0.37, .lon = 11.73, .prefixes = "TR"},
    {.continent = "AF", .country = "Chad", .cq_zone = 36, .itu_zone = 47, .lat = 15.80, .lon = 18.17, .prefixes = "TT"},
    {.continent = "AF", .country = "Cote d'Ivoire", .cq_zone = 35, .itu_zone = 46, .lat = 7.58, .lon = -5.8, .prefixes = "TU"},
    {.continent = "AF", .country = "Benin", .cq_zone = 35, .itu_zone = 46, .lat = 9.87, .lon = 2.25, .prefixes = "TY"},
    {.continent = "AF", .country = "Mali", .cq_zone = 35, .itu_zone = 46, .lat = 18.00, .lon = -2.58, .prefixes = "TZ"},
    {.continent = "EU",
     .country = "European Russia",
     .cq_zone = 16,
     .itu_zone = 29,
     .lat = 53.65,
     .lon = 41.37,
     .prefixes = "R,U,R1I,R1N,R1O,R1P,R1Z,R4H,R4I,R4W,R8F,R8G,R8X,R9F,R9G,R9X,RA1I,RA1N,RA1O,RA1P,RA1Z,RA4H,RA4I,RA4W,RA8F,RA8G,RA8X,RA9F,RA9G,RA9X,"
                 "RC1I,RC1N,RC1O,RC1P,RC1Z,RC4H,RC4I,RC4W,RC8F,RC8G,RC8X,RC9F,RC9G,RC9X,RD1I,RD1N,RD1O,RD1P,RD1Z,RD4H,RD4I,RD4W,RD8F,RD8G,RD8X,RD9F,"
                 "RD9G,RD9X,RE1I,RE1N,RE1O,RE1P,RE1Z,RE4H,RE4I,RE4W,RE8F,RE8G,RE8X,RE9F,RE9G,RE9X,RF1I,RF1N,RF1O,RF1P,RF1Z,RF4H,RF4I,RF4W,RF8F,RF8G,"
                 "RF8X,RF9F,RF9G,RF9X,RG1I,RG1N,RG1O,RG1P,RG1Z,RG4H,RG4I,RG4W,RG8F,RG8G,RG8X,RG9F,RG9G,RG9X,RI8X,RI9X,RJ1I,RJ1N,RJ1O,RJ1P,RJ1Z,RJ4H,"
                 "RJ4I,RJ4W,RJ8F,RJ8G,RJ8X,RJ9F,RJ9G,RJ9X,RK1I,RK1N,RK1O,RK1P,RK1Z,RK4H,RK4I,RK4W,RK8F,RK8G,RK8X,RK9F,RK9G,RK9X,RL1I,RL1N,RL1O,RL1P,"
                 "RL1Z,RL4H,RL4I,RL4W,RL8F,RL8G,RL8X,RL9F,RL9G,RL9X,RM1I,RM1N,RM1O,RM1P,RM1Z,RM4H,RM4I,RM4W,RM8F,RM8G,RM8X,RM9F,RM9G,RM9X,RN1I,RN1N,"
                 "RN1O,RN1P,RN1Z,RN4H,RN4I,RN4W,RN8F,RN8G,RN8X,RN9F,RN9G,RN9X,RO1I,RO1N,RO1O,RO1P,RO1Z,RO4H,RO4I,RO4W,RO8F,RO8G,RO8X,RO9F,RO9G,RO9X,"
                 "RQ1I,RQ1N,RQ1O,RQ1P,RQ1Z,RQ4H,RQ4I,RQ4W,RQ8F,RQ8G,RQ8X,RQ9F,RQ9G,RQ9X,RT1I,RT1N,RT1O,RT1P,RT1Z,RT4H,RT4I,RT4W,RT8F,RT8G,RT8X,RT9F,"
                 "RT9G,RT9X,RU1I,RU1N,RU1O,RU1P,RU1Z,RU4H,RU4I,RU4W,RU8F,RU8G,RU8X,RU9F,RU9G,RU9X,RV1I,RV1N,RV1O,RV1P,RV1Z,RV4H,RV4I,RV4W,RV8F,RV8G,"
                 "RV8X,RV9F,RV9G,RV9X,RW1I,RW1N,RW1O,RW1P,RW1Z,RW4H,RW4I,RW4W,RW8F,RW8G,RW8X,RW9F,RW9G,RW9X,RX1I,RX1N,RX1O,RX1P,RX1Z,RX4H,RX4I,RX4W,"
                 "RX8F,RX8G,RX8X,RX9F,RX9G,RX9X,RY1I,RY1N,RY1O,RY1P,RY1Z,RY4H,RY4I,RY4W,RY8F,RY8G,RY8X,RY9F,RY9G,RY9X,RZ1I,RZ1N,RZ1O,RZ1P,RZ1Z,RZ4H,"
                 "RZ4I,RZ4W,RZ8F,RZ8G,RZ8X,RZ9F,RZ9G,RZ9X,U1I,U1N,U1O,U1P,U1Z,U4H,U4I,U4W,U8F,U8G,U8X,U9F,U9G,U9X,UA1I,UA1N,UA1O,UA1P,UA1Z,UA4H,UA4I,"
                 "UA4W,UA8F,UA8G,UA8X,UA9F,UA9G,UA9X,UB1I,UB1N,UB1O,UB1P,UB1Z,UB4H,UB4I,UB4W,UB8F,UB8G,UB8X,UB9F,UB9G,UB9X,UC1I,UC1N,UC1O,UC1P,UC1Z,"
                 "UC4H,UC4I,UC4W,UC8F,UC8G,UC8X,UC9F,UC9G,UC9X,UD1I,UD1N,UD1O,UD1P,UD1Z,UD4H,UD4I,UD4W,UD8F,UD8G,UD8X,UD9F,UD9G,UD9X,UE1I,UE1N,UE1O,"
                 "UE1P,UE1Z,UE4H,UE4I,UE4W,UE8F,UE8G,UE8X,UE9F,UE9G,UE9X,UF1I,UF1N,UF1O,UF1P,UF1Z,UF4H,UF4I,UF4W,UF8F,UF8G,UF8X,UF9F,UF9G,UF9X,UG1I,"
                 "UG1N,UG1O,UG1P,UG1Z,UG4H,UG4I,UG4W,UG8F,UG8G,UG8X,UG9F,UG9G,UG9X,UH1I,UH1N,UH1O,UH1P,UH1Z,UH4H,UH4I,UH4W,UH8F,UH8G,UH8X,UH9F,UH9G,"
                 "UH9X,UI1I,UI1N,UI1O,UI1P,UI1Z,UI4H,UI4I,UI4W,UI8F,UI8G,UI8X,UI9F,UI9G,UI9X,R4HAT,R4HC,R4HCE,R4HCZ,R4HD,R4HDC,R4HDR,R4HL,R4IC,R4ID,"
                 "R4II,R4IK,R4IM,R4IN,R4IO,R4IT,R7AB/P,R8FF/3,R8FF/P,R8MB/1,R9GM/P,R9PA/4,RA3X/"
                 "1,RA4HL,RA4NCC,RC4HT,RC4I,RJ4I,RJ4P,RK30DR,RK4HM,RK75OP,RM4I,RM4R,RN4HFJ,RN4HIF,RT9K/6,RT9S/P,RT9T/1,RT9T/"
                 "3,RU4HD,RU4HP,RU4I,RW4HM,RW4HTK,RW4HW,RW4HZ,UA4H,UA4HBM,UA4HGL,UA4HIP,UA4HRZ,UA4HY,UA4NF,UA4PN,UC4I,UE00S,UE90PR,UI4I"},
    {.continent = "EU",
     .country = "Kaliningrad",
     .cq_zone = 15,
     .itu_zone = 29,
     .lat = 54.72,
     .lon = 20.52,
     .prefixes = "R2F,R2K,RA2,RC2F,RC2K,RD2F,RD2K,RE2F,RE2K,RF2F,RF2K,RG2F,RG2K,RJ2F,RJ2K,RK2F,RK2K,RL2F,RL2K,RM2F,RM2K,RN2F,RN2K,RO2F,RO2K,RQ2F,RQ2K,"
                 "RT2F,RT2K,RU2F,RU2K,RV2F,RV2K,RW2F,RW2K,RX2F,RX2K,RY2F,RY2K,RZ2F,RZ2K,U2F,U2K,UA2,UB2,UC2,UD2,UE2,UF2,UG2,UH2,UI2,R2MWO"},
    {.continent = "AS",
     .country = "Asiatic Russia",
     .cq_zone = 17,
     .itu_zone = 30,
     .lat = 55.88,
     .lon = 84.08,
     .prefixes = "R0,R8,R9,RA0,RA8,RA9,RC0,RC8,RC9,RD0,RD8,RD9,RE0,RE8,RE9,RF0,RF8,RF9,RG0,RG8,RG9,RI0,RI8,RI9,RJ0,RJ8,RJ9,RK0,RK8,RK9,RL0,RL8,RL9,RM0,"
                 "RM8,RM9,RN0,RN8,RN9,RO0,RO8,RO9,RQ0,RQ8,RQ9,RT0,RT8,RT9,RU0,RU8,RU9,RV0,RV8,RV9,RW0,RW8,RW9,RX0,RX8,RX9,RY0,RY8,RY9,RZ0,RZ8,RZ9,U0,U8,"
                 "U9,UA0,UA8,UA9,UB0,UB8,UB9,UC0,UC8,UC9,UD0,UD8,UD9,UE0,UE8,UE9,UF0,UF8,UF9,UG0,UG8,UG9,UH0,UH8,UH9,UI0,UI8,UI9,R0T,R8H,R8I,R8O,R8P,"
                 "R8S,R8T,R8U,R8V,R8W,R8Y,R8Z,R9I,R9M,R9P,R9S,R9T,R9V,R9W,RA0T,RA8H,RA8I,RA8O,RA8P,RA8S,RA8T,RA8U,RA8V,RA8W,RA8Y,RA8Z,RA9I,RA9M,RA9P,"
                 "RA9S,RA9T,RA9V,RA9W,RC0T,RC8H,RC8I,RC8O,RC8P,RC8S,RC8T,RC8U,RC8V,RC8W,RC8Y,RC8Z,RC9I,RC9M,RC9P,RC9S,RC9T,RC9V,RC9W,RD0T,RD8H,RD8I,"
                 "RD8O,RD8P,RD8S,RD8T,RD8U,RD8V,RD8W,RD8Y,RD8Z,RD9I,RD9M,RD9P,RD9S,RD9T,RD9V,RD9W,RE0T,RE8H,RE8I,RE8O,RE8P,RE8S,RE8T,RE8U,RE8V,RE8W,"
                 "RE8Y,RE8Z,RE9I,RE9M,RE9P,RE9S,RE9T,RE9V,RE9W,RF0T,RF8H,RF8I,RF8O,RF8P,RF8S,RF8T,RF8U,RF8V,RF8W,RF8Y,RF8Z,RF9I,RF9M,RF9P,RF9S,RF9T,"
                 "RF9V,RF9W,RG0T,RG8H,RG8I,RG8O,RG8P,RG8S,RG8T,RG8U,RG8V,RG8W,RG8Y,RG8Z,RG9I,RG9M,RG9P,RG9S,RG9T,RG9V,RG9W,RJ0T,RJ8H,RJ8I,RJ8O,RJ8P,"
                 "RJ8S,RJ8T,RJ8U,RJ8V,RJ8W,RJ8Y,RJ8Z,RJ9I,RJ9M,RJ9P,RJ9S,RJ9T,RJ9V,RJ9W,RK0T,RK8H,RK8I,RK8O,RK8P,RK8S,RK8T,RK8U,RK8V,RK8W,RK8Y,RK8Z,"
                 "RK9I,RK9M,RK9P,RK9S,RK9T,RK9V,RK9W,RL0T,RL8H,RL8I,RL8O,RL8P,RL8S,RL8T,RL8U,RL8V,RL8W,RL8Y,RL8Z,RL9I,RL9M,RL9P,RL9S,RL9T,RL9V,RL9W,"
                 "RM0T,RM8H,RM8I,RM8O,RM8P,RM8S,RM8T,RM8U,RM8V,RM8W,RM8Y,RM8Z,RM9I,RM9M,RM9P,RM9S,RM9T,RM9V,RM9W,RN0T,RN8H,RN8I,RN8O,RN8P,RN8S,RN8T,"
                 "RN8U,RN8V,RN8W,RN8Y,RN8Z,RN9I,RN9M,RN9P,RN9S,RN9T,RN9V,RN9W,RO0T,RO8H,RO8I,RO8O,RO8P,RO8S,RO8T,RO8U,RO8V,RO8W,RO8Y,RO8Z,RO9I,RO9M,"
                 "RO9P,RO9S,RO9T,RO9V,RO9W,RQ0T,RQ8H,RQ8I,RQ8O,RQ8P,RQ8S,RQ8T,RQ8U,RQ8V,RQ8W,RQ8Y,RQ8Z,RQ9I,RQ9M,RQ9P,RQ9S,RQ9T,RQ9V,RQ9W,RT0T,RT8H,"
                 "RT8I,RT8O,RT8P,RT8S,RT8T,RT8U,RT8V,RT8W,RT8Y,RT8Z,RT9I,RT9M,RT9P,RT9S,RT9T,RT9V,RT9W,RU0T,RU8H,RU8I,RU8O,RU8P,RU8S,RU8T,RU8U,RU8V,"
                 "RU8W,RU8Y,RU8Z,RU9I,RU9M,RU9P,RU9S,RU9T,RU9V,RU9W,RV0T,RV8H,RV8I,RV8O,RV8P,RV8S,RV8T,RV8U,RV8V,RV8W,RV8Y,RV8Z,RV9I,RV9M,RV9P,RV9S,"
                 "RV9T,RV9V,RV9W,RW0T,RW8H,RW8I,RW8O,RW8P,RW8S,RW8T,RW8U,RW8V,RW8W,RW8Y,RW8Z,RW9I,RW9M,RW9P,RW9S,RW9T,RW9V,RW9W,RX0T,RX8H,RX8I,RX8O,"
                 "RX8P,RX8S,RX8T,RX8U,RX8V,RX8W,RX8Y,RX8Z,RX9I,RX9M,RX9P,RX9S,RX9T,RX9V,RX9W,RY0T,RY8H,RY8I,RY8O,RY8P,RY8S,RY8T,RY8U,RY8V,RY8W,RY8Y,"
                 "RY8Z,RY9I,RY9M,RY9P,RY9S,RY9T,RY9V,RY9W,RZ0T,RZ8H,RZ8I,RZ8O,RZ8P,RZ8S,RZ8T,RZ8U,RZ8V,RZ8W,RZ8Y,RZ8Z,RZ9I,RZ9M,RZ9P,RZ9S,RZ9T,RZ9V,"
                 "RZ9W,U0T,U8H,U8I,U8O,U8P,U8S,U8T,U8U,U8V,U8W,U8Y,U8Z,U9I,U9M,U9P,U9S,U9T,U9V,U9W,UA0T,UA8H,UA8I,UA8O,UA8P,UA8S,UA8T,UA8U,UA8V,UA8W,"
                 "UA8Y,UA8Z,UA9I,UA9M,UA9P,UA9S,UA9T,UA9V,UA9W,UB0T,UB8H,UB8I,UB8O,UB8P,UB8S,UB8T,UB8U,UB8V,UB8W,UB8Y,UB8Z,UB9I,UB9M,UB9P,UB9S,UB9T,"
                 "UB9V,UB9W,UC0T,UC8H,UC8I,UC8O,UC8P,UC8S,UC8T,UC8U,UC8V,UC8W,UC8Y,UC8Z,UC9I,UC9M,UC9P,UC9S,UC9T,UC9V,UC9W,UD0T,UD8H,UD8I,UD8O,UD8P,"
                 "UD8S,UD8T,UD8U,UD8V,UD8W,UD8Y,UD8Z,UD9I,UD9M,UD9P,UD9S,UD9T,UD9V,UD9W,UE0T,UE8H,UE8I,UE8O,UE8P,UE8S,UE8T,UE8U,UE8V,UE8W,UE8Y,UE8Z,"
                 "UE9I,UE9M,UE9P,UE9S,UE9T,UE9V,UE9W,UF0T,UF8H,UF8I,UF8O,UF8P,UF8S,UF8T,UF8U,UF8V,UF8W,UF8Y,UF8Z,UF9I,UF9M,UF9P,UF9S,UF9T,UF9V,UF9W,"
                 "UG0T,UG8H,UG8I,UG8O,UG8P,UG8S,UG8T,UG8U,UG8V,UG8W,UG8Y,UG8Z,UG9I,UG9M,UG9P,UG9S,UG9T,UG9V,UG9W,UH0T,UH8H,UH8I,UH8O,UH8P,UH8S,UH8T,"
                 "UH8U,UH8V,UH8W,UH8Y,UH8Z,UH9I,UH9M,UH9P,UH9S,UH9T,UH9V,UH9W,UI0T,UI8H,UI8I,UI8O,UI8P,UI8S,UI8T,UI8U,UI8V,UI8W,UI8Y,UI8Z,UI9I,UI9M,"
                 "UI9P,UI9S,UI9T,UI9V,UI9W,R2ET/9,R30SIB,R63RRC,R9HQ,RN9S,RO9S,RT9S,RT9W,RX3Q/8,RZ5D/9,UA3A/P,UB5O/4"},
    {.continent = "AS", .country = "Uzbekistan", .cq_zone = 17, .itu_zone = 30, .lat = 41.40, .lon = 63.97, .prefixes = "UJ,UK,UL,UM"},
    {.continent = "AS",
     .country = "Kazakhstan",
     .cq_zone = 17,
     .itu_zone = 30,
     .lat = 48.17,
     .lon = 65.18,
     .prefixes = "UN,UO,UP,UQ,UN0F,UN0G,UN0J,UN0Q,UN2F,UN2G,UN2J,UN2Q,UN3F,UN3G,UN3J,UN3Q,UN4F,UN4G,UN4J,UN4Q,UN5F,UN5G,UN5J,UN5Q,UN6F,UN6G,"
                 "UN6J,UN6Q,UN7F,UN7G,UN7J,UN7Q,UN8F,UN8G,UN8J,UN8Q,UN9F,UN9G,UN9J,UN9Q,UO0F,UO0G,UO0J,UO0Q,UO1F,UO1G,UO1J,UO1Q,UO2F,UO2G,UO2J,"
                 "UO2Q,UO3F,UO3G,UO3J,UO3Q,UO4F,UO4G,UO4J,UO4Q,UO5F,UO5G,UO5J,UO5Q,UO6F,UO6G,UO6J,UO6Q,UO7F,UO7G,UO7J,UO7Q,UO8F,UO8G,UO8J,UO8Q,"
                 "UO9F,UO9G,UO9J,UO9Q,UP0F,UP0G,UP0J,UP0Q,UP1F,UP1G,UP1J,UP1Q,UP2F,UP2G,UP2J,UP2Q,UP3F,UP3G,UP3J,UP3Q,UP4F,UP4G,UP4J,UP4Q,UP5F,"
                 "UP5G,UP5J,UP5Q,UP6F,UP6G,UP6J,UP6Q,UP7F,UP7G,UP7J,UP7Q,UP8F,UP8G,UP8J,UP8Q,UP9F,UP9G,UP9J,UP9Q,UQ0F,UQ0G,UQ0J,UQ0Q,UQ1F,UQ1G,"
                 "UQ1J,UQ1Q,UQ2F,UQ2G,UQ2J,UQ2Q,UQ3F,UQ3G,UQ3J,UQ3Q,UQ4F,UQ4G,UQ4J,UQ4Q,UQ5F,UQ5G,UQ5J,UQ5Q,UQ6F,UQ6G,UQ6J,UQ6Q,UQ7F,UQ7G,UQ7J,"
                 "UQ7Q,UQ8F,UQ8G,UQ8J,UQ8Q,UQ9F,UQ9G,UQ9J,UQ9Q"},
    {.continent = "EU", .country = "Ukraine", .cq_zone = 16, .itu_zone = 29, .lat = 50.00, .lon = 30, .prefixes = "EM,EN,EO,U5,UR,US,UT,UU,UV,UW,UX,UY,UZ"},
    {.continent = "NA", .country = "Antigua & Barbuda", .cq_zone = 8, .itu_zone = 11, .lat = 17.07, .lon = -61.8, .prefixes = "V2"},
    {.continent = "NA", .country = "Belize", .cq_zone = 7, .itu_zone = 11, .lat = 16.97, .lon = -88.67, .prefixes = "V3"},
    {.continent = "NA", .country = "St. Kitts & Nevis", .cq_zone = 8, .itu_zone = 11, .lat = 17.37, .lon = -62.78, .prefixes = "V4"},
    {.continent = "AF", .country = "Namibia", .cq_zone = 38, .itu_zone = 57, .lat = -22.00, .lon = 17, .prefixes = "V5"},
    {.continent = "OC", .country = "Micronesia", .cq_zone = 27, .itu_zone = 65, .lat = 6.88, .lon = 158.2, .prefixes = "V6"},
    {.continent = "OC", .country = "Marshall Islands", .cq_zone = 31, .itu_zone = 65, .lat = 9.08, .lon = 167.33, .prefixes = "V7"},
    {.continent = "OC", .country = "Brunei Darussalam", .cq_zone = 28, .itu_zone = 54, .lat = 4.50, .lon = 114.6, .prefixes = "V8"},
    {.continent = "NA",
     .country = "Canada",
     .cq_zone = 5,
     .itu_zone = 9,
     .lat = 44.35,
     .lon = -78.75,
     .prefixes = "CF,CG,CJ,CK,VA,VB,VC,VE,VG,VX,VY9,XL,XM,CF2,CG2,CH1,CH2,CI0,CI1,CI2,CJ2,CK2,CY1,CY2,CZ0,CZ1,CZ2,VA2,VB2,VC2,VD1,VD2,VE2,VF0,"
                 "VF1,VF2,VG2,VO1,VO2,VX2,VY0,VY1,VY2,XJ1,XJ2,XK0,XK1,XK2,XL2,XM2,XN1,XN2,XO0,XO1,XO2,VA2VVV,VE2CSI,VE2EKA,VE2FK,VE2IDX,VE2IM,"
                 "VE2KK,VE2NN,VE2TWO,VE8AT,VY0AA,VY0PW"},
    {.continent = "OC", .country = "Australia", .cq_zone = 30, .itu_zone = 59, .lat = -23.70, .lon = 132.33, .prefixes = "AX,VH,VI,VJ,VK,VL,VM,VN,VZ,AX4,VH4,VI4,VJ4,VK4,VL4,VM4,VN4,VZ4"},
    {.continent = "AF", .country = "Heard Island", .cq_zone = 39, .itu_zone = 68, .lat = -53.08, .lon = 73.5, .prefixes = "VK0EK"},
    {.continent = "OC", .country = "Macquarie Island", .cq_zone = 30, .itu_zone = 60, .lat = -54.60, .lon = 158.88, .prefixes = "VK0AI"},
    {.continent = "OC",
     .country = "Cocos (Keeling) Islands",
     .cq_zone = 29,
     .itu_zone = 54,
     .lat = -12.15,
     .lon = 96.82,
     .prefixes = "AX9C,AX9Y,VH9C,VH9Y,VI9C,VI9Y,VJ9C,VJ9Y,VK9C,VK9FC,VK9KC,VK9Y,VK9ZY,VL9C,VL9Y,VM9C,VM9Y,VN9C,VN9Y,VZ9C,VZ9Y"},
    {.continent = "OC",
     .country = "Lord Howe Island",
     .cq_zone = 30,
     .itu_zone = 60,
     .lat = -31.55,
     .lon = 159.08,
     .prefixes = "AX9L,VH9L,VI9L,VJ9L,VK9FL,VK9L,VK9ZL,VL9L,VM9L,VN9L,VZ9L,VK9APX"},
    {.continent = "OC", .country = "Mellish Reef", .cq_zone = 30, .itu_zone = 56, .lat = -17.40, .lon = 155.85, .prefixes = "AX9M,VH9M,VI9M,VJ9M,VK9M,VL9M,VM9M,VN9M,VZ9M"},
    {.continent = "OC", .country = "Norfolk Island", .cq_zone = 32, .itu_zone = 60, .lat = -29.03, .lon = 167.93, .prefixes = "AX9,VH9,VI9,VJ9,VK9,VL9,VM9,VN9,VZ9"},
    {.continent = "OC",
     .country = "Willis Island",
     .cq_zone = 30,
     .itu_zone = 55,
     .lat = -16.22,
     .lon = 150.02,
     .prefixes = "AX9W,AX9Z,VH9W,VH9Z,VI9W,VI9Z,VJ9W,VJ9Z,VK9FW,VK9W,VK9Z,VL9W,VL9Z,VM9W,VM9Z,VN9W,VN9Z,VZ9W,VZ9Z"},
    {.continent = "OC", .country = "Christmas Island", .cq_zone = 29, .itu_zone = 54, .lat = -10.48, .lon = 105.63, .prefixes = "AX9X,VH9X,VI9X,VJ9X,VK9FX,VK9KX,VK9X,VL9X,VM9X,VN9X,VZ9X"},
    {.continent = "NA", .country = "Anguilla", .cq_zone = 8, .itu_zone = 11, .lat = 18.23, .lon = -63, .prefixes = "VP2E"},
    {.continent = "NA", .country = "Montserrat", .cq_zone = 8, .itu_zone = 11, .lat = 16.75, .lon = -62.18, .prefixes = "VP2M"},
    {.continent = "NA", .country = "British Virgin Islands", .cq_zone = 8, .itu_zone = 11, .lat = 18.33, .lon = -64.75, .prefixes = "VP2V"},
    {.continent = "NA", .country = "Turks & Caicos Islands", .cq_zone = 8, .itu_zone = 11, .lat = 21.77, .lon = -71.75, .prefixes = "VP5,VQ5"},
    {.continent = "OC", .country = "Pitcairn Island", .cq_zone = 32, .itu_zone = 63, .lat = -25.07, .lon = -130.1, .prefixes = "VP6"},
    {.continent = "OC", .country = "Ducie Island", .cq_zone = 32, .itu_zone = 63, .lat = -24.70, .lon = -124.8, .prefixes = "VP6D"},
    {.continent = "SA", .country = "Falkland Islands", .cq_zone = 13, .itu_zone = 16, .lat = -51.63, .lon = -58.72, .prefixes = "VP8"},
    {.continent = "SA", .country = "South Georgia Island", .cq_zone = 13, .itu_zone = 73, .lat = -54.48, .lon = -37.08, .prefixes = "VP8CA"},
    {.continent = "SA", .country = "South Shetland Islands", .cq_zone = 13, .itu_zone = 73, .lat = -62.08, .lon = -58.67, .prefixes = "CE9,XR9,HF0POL,RI1ANF"},
    {.continent = "SA", .country = "South Orkney Islands", .cq_zone = 13, .itu_zone = 73, .lat = -60.60, .lon = -45.55, .prefixes = "LU4ZD"},
    {.continent = "SA", .country = "South Sandwich Islands", .cq_zone = 13, .itu_zone = 73, .lat = -58.43, .lon = -26.33, .prefixes = "VP8DXU"},
    {.continent = "NA", .country = "Bermuda", .cq_zone = 5, .itu_zone = 11, .lat = 32.32, .lon = -64.73, .prefixes = "VP9"},
    {.continent = "AF", .country = "Chagos Islands", .cq_zone = 39, .itu_zone = 41, .lat = -7.32, .lon = 72.42, .prefixes = "VQ9"},
    {.continent = "AS", .country = "Hong Kong", .cq_zone = 24, .itu_zone = 44, .lat = 22.28, .lon = 114.18, .prefixes = "VR"},
    {.continent = "AS", .country = "India", .cq_zone = 22, .itu_zone = 41, .lat = 22.50, .lon = 77.58, .prefixes = "8T,8U,8V,8W,8X,8Y,AT,AU,AV,AW,VT,VU,VV,VW"},
    {.continent = "AS", .country = "Andaman & Nicobar Is.", .cq_zone = 26, .itu_zone = 49, .lat = 12.37, .lon = 92.78, .prefixes = "VU4"},
    {.continent = "AS", .country = "Lakshadweep Islands", .cq_zone = 22, .itu_zone = 41, .lat = 11.23, .lon = 72.78, .prefixes = "VU7"},
    {.continent = "NA", .country = "Mexico", .cq_zone = 6, .itu_zone = 10, .lat = 21.32, .lon = -100.23, .prefixes = "4A,4B,4C,6D,6E,6F,6G,6H,6I,6J,XA,XB,XC,XD,XE,XF,XG,XH,XI"},
    {.continent = "NA",
     .country = "Revillagigedo",
     .cq_zone = 6,
     .itu_zone = 10,
     .lat = 18.77,
     .lon = -110.97,
     .prefixes = "4A4,4B4,4C4,6D4,6E4,6F4,6G4,6H4,6I4,6J4,XA4,XB4,XC4,XD4,XE4,XF4,XG4,XH4,XI4"},
    {.continent = "AF", .country = "Burkina Faso", .cq_zone = 35, .itu_zone = 46, .lat = 12.00, .lon = -2, .prefixes = "XT"},
    {.continent = "AS", .country = "Cambodia", .cq_zone = 26, .itu_zone = 49, .lat = 12.93, .lon = 105.13, .prefixes = "XU"},
    {.continent = "AS", .country = "Laos", .cq_zone = 26, .itu_zone = 49, .lat = 18.20, .lon = 104.55, .prefixes = "XW"},
    {.continent = "AS", .country = "Macao", .cq_zone = 24, .itu_zone = 44, .lat = 22.10, .lon = 113.5, .prefixes = "XX9"},
    {.continent = "AS", .country = "Myanmar", .cq_zone = 26, .itu_zone = 49, .lat = 20.00, .lon = 96.37, .prefixes = "XY,XZ"},
    {.continent = "AS", .country = "Afghanistan", .cq_zone = 21, .itu_zone = 40, .lat = 34.70, .lon = 65.8, .prefixes = "T6,YA"},
    {.continent = "OC",
     .country = "Indonesia",
     .cq_zone = 28,
     .itu_zone = 51,
     .lat = -7.30,
     .lon = 109.88,
     .prefixes = "7A,7B,7C,7D,7E,7F,7G,7H,7I,8A,8B,8C,8D,8E,8F,8G,8H,8I,PK,PL,PM,PN,PO,YB,YC,YD,YE,YF,YG,YH,YB0,YB1,YB2,YB3,YB4,YB5,YB6,YB7,YB8,"
                 "YC0,YC1,YC2,YC3,YC4,YC5,YC6,YC7,YC8,YD0,YD1,YD2,YD3,YD4,YD5,YD6,YD7,YD8,YE0,YE1,YE2,YE3,YE4,YE5,YE6,YE7,YE8,YF0,YF1,YF2,YF3,"
                 "YF4,YF5,YF6,YF7,YF8,YG0,YG1,YG2,YG3,YG4,YG5,YG6,YG7,YG8,YH0,YH1,YH2,YH3,YH4,YH5,YH6,YH7,YH8"},
    {.continent = "AS", .country = "Iraq", .cq_zone = 21, .itu_zone = 39, .lat = 33.92, .lon = 42.78, .prefixes = "HN,YI"},
    {.continent = "OC", .country = "Vanuatu", .cq_zone = 32, .itu_zone = 56, .lat = -17.67, .lon = 168.38, .prefixes = "YJ"},
    {.continent = "AS", .country = "Syria", .cq_zone = 20, .itu_zone = 39, .lat = 35.38, .lon = 38.2, .prefixes = "6C,YK"},
    {.continent = "EU", .country = "Latvia", .cq_zone = 15, .itu_zone = 29, .lat = 57.03, .lon = 24.65, .prefixes = "YL"},
    {.continent = "NA", .country = "Nicaragua", .cq_zone = 7, .itu_zone = 11, .lat = 12.88, .lon = -85.05, .prefixes = "H6,H7,HT,YN"},
    {.continent = "EU", .country = "Romania", .cq_zone = 20, .itu_zone = 28, .lat = 45.78, .lon = 24.7, .prefixes = "YO,YP,YQ,YR"},
    {.continent = "NA", .country = "El Salvador", .cq_zone = 7, .itu_zone = 11, .lat = 14.00, .lon = -89, .prefixes = "HU,YS"},
    {.continent = "EU", .country = "Serbia", .cq_zone = 15, .itu_zone = 28, .lat = 44.00, .lon = 21, .prefixes = "YT,YU"},
    {.continent = "SA", .country = "Venezuela", .cq_zone = 9, .itu_zone = 12, .lat = 8.00, .lon = -66, .prefixes = "4M,YV,YW,YX,YY"},
    {.continent = "NA", .country = "Aves Island", .cq_zone = 8, .itu_zone = 11, .lat = 15.67, .lon = -63.6, .prefixes = "4M0,YV0,YW0,YX0,YY0"},
    {.continent = "AF", .country = "Zimbabwe", .cq_zone = 38, .itu_zone = 53, .lat = -18.00, .lon = 31, .prefixes = "Z2"},
    {.continent = "EU", .country = "North Macedonia", .cq_zone = 15, .itu_zone = 28, .lat = 41.60, .lon = 21.65, .prefixes = "Z3"},
    {.continent = "EU", .country = "Republic of Kosovo", .cq_zone = 15, .itu_zone = 28, .lat = 42.67, .lon = 21.17, .prefixes = "Z6"},
    {.continent = "AF", .country = "Republic of South Sudan", .cq_zone = 34, .itu_zone = 48, .lat = 4.85, .lon = 31.6, .prefixes = "Z8"},
    {.continent = "EU", .country = "Albania", .cq_zone = 15, .itu_zone = 28, .lat = 41.00, .lon = 20, .prefixes = "ZA"},
    {.continent = "EU", .country = "Gibraltar", .cq_zone = 14, .itu_zone = 37, .lat = 36.15, .lon = -5.37, .prefixes = "ZB,ZG"},
    {.continent = "AS", .country = "UK Base Areas on Cyprus", .cq_zone = 20, .itu_zone = 39, .lat = 35.32, .lon = 33.57, .prefixes = "ZC4"},
    {.continent = "AF", .country = "St. Helena", .cq_zone = 36, .itu_zone = 66, .lat = -15.97, .lon = -5.72, .prefixes = "ZD7"},
    {.continent = "AF", .country = "Ascension Island", .cq_zone = 36, .itu_zone = 66, .lat = -7.93, .lon = -14.37, .prefixes = "ZD8"},
    {.continent = "AF", .country = "Tristan da Cunha & Gough", .cq_zone = 38, .itu_zone = 66, .lat = -37.13, .lon = -12.3, .prefixes = "ZD9"},
    {.continent = "NA", .country = "Cayman Islands", .cq_zone = 8, .itu_zone = 11, .lat = 19.32, .lon = -81.22, .prefixes = "ZF"},
    {.continent = "OC", .country = "Tokelau Islands", .cq_zone = 31, .itu_zone = 62, .lat = -9.40, .lon = -171.2, .prefixes = "ZK3"},
    {.continent = "OC", .country = "New Zealand", .cq_zone = 32, .itu_zone = 60, .lat = -41.83, .lon = 173.27, .prefixes = "ZK,ZL,ZL50,ZM"},
    {.continent = "OC", .country = "Chatham Islands", .cq_zone = 32, .itu_zone = 60, .lat = -43.85, .lon = -176.48, .prefixes = "ZL7,ZM7"},
    {.continent = "OC", .country = "Kermadec Islands", .cq_zone = 32, .itu_zone = 60, .lat = -29.25, .lon = -177.92, .prefixes = "ZL8,ZM8"},
    {.continent = "OC", .country = "N.Z. Subantarctic Is.", .cq_zone = 32, .itu_zone = 60, .lat = -51.62, .lon = 167.62, .prefixes = "ZL9"},
    {.continent = "SA", .country = "Paraguay", .cq_zone = 11, .itu_zone = 14, .lat = -25.27, .lon = -57.67, .prefixes = "ZP"},
    {.continent = "AF", .country = "South Africa", .cq_zone = 38, .itu_zone = 57, .lat = -29.07, .lon = 22.63, .prefixes = "H5,S4,S8,V9,ZR,ZS,ZT,ZU"},
    {.continent = "AF", .country = "Pr. Edward & Marion Is.", .cq_zone = 38, .itu_zone = 57, .lat = -46.88, .lon = 37.72, .prefixes = "ZR8,ZS8,ZT8,ZU8"},
};

// Russian cty_rus.dat
static const CALLSIGN_INFO_LINE CALLSIGN_INFO_RUSSIAN_DB[CALLSIGN_DB_RUS_COUNT] = {
    {.continent = "AN", .country = "Russia - Mirny Station", .cq_zone = 29, .itu_zone = 69, .lat = -66.55, .lon = 93, .prefixes = "RI1AN,RI1ANT"},
    {.continent = "AN", .country = "Russia - Progress Station", .cq_zone = 39, .itu_zone = 69, .lat = -69.37, .lon = 76.38, .prefixes = "RI1ANZ"},
    {.continent = "AN", .country = "Russia - Vostok Station", .cq_zone = 29, .itu_zone = 70, .lat = -78.47, .lon = 106.8, .prefixes = "RI1ANC"},
    {.continent = "EU", .country = "Russia - Franz Josef Land", .cq_zone = 40, .itu_zone = 75, .lat = 80.68, .lon = 49.92, .prefixes = "RI1F"},
    {.continent = "EU",
     .country = "Russia - Saint Petersburg",
     .cq_zone = 16,
     .itu_zone = 29,
     .lat = 59.95,
     .lon = 30.3,
     .prefixes = "R1A,R1B,R1F,R1H,R1J,R1L,R1M,RA1A,RA1B,RA1F,RA1H,RA1J,RA1L,RA1M,RC1A,RC1B,RC1F,RC1H,RC1J,RC1L,RC1M,RD1A,RD1B,RD1F,RD1H,RD1J,"
                 "RD1L,RD1M,RE1A,RE1B,RE1F,RE1H,RE1J,RE1L,RE1M,RF1A,RF1B,RF1F,RF1H,RF1J,RF1L,RF1M,RG1A,RG1B,RG1F,RG1H,RG1J,RG1L,RG1M,RJ1A,RJ1B,"
                 "RJ1F,RJ1H,RJ1J,RJ1L,RJ1M,RK1A,RK1B,RK1F,RK1H,RK1J,RK1L,RK1M,RL1A,RL1B,RL1F,RL1H,RL1J,RL1L,RL1M,RM1A,RM1B,RM1F,RM1H,RM1J,RM1L,"
                 "RM1M,RN1A,RN1B,RN1F,RN1H,RN1J,RN1L,RN1M,RO1A,RO1B,RO1F,RO1H,RO1J,RO1L,RO1M,RQ1A,RQ1B,RQ1F,RQ1H,RQ1J,RQ1L,RQ1M,RT1A,RT1B,RT1F,"
                 "RT1H,RT1J,RT1L,RT1M,RU1A,RU1B,RU1F,RU1H,RU1J,RU1L,RU1M,RV1A,RV1B,RV1F,RV1H,RV1J,RV1L,RV1M,RW1A,RW1B,RW1F,RW1H,RW1J,RW1L,RW1M,"
                 "RX1A,RX1B,RX1F,RX1H,RX1J,RX1L,RX1M,RY1A,RY1B,RY1F,RY1H,RY1J,RY1L,RY1M,RZ1A,RZ1B,RZ1F,RZ1H,RZ1J,RZ1L,RZ1M,U1A,U1B,U1F,U1H,U1J,"
                 "U1L,U1M,UA1A,UA1B,UA1F,UA1H,UA1J,UA1L,UA1M,UB1A,UB1B,UB1F,UB1H,UB1J,UB1L,UB1M,UC1A,UC1B,UC1F,UC1H,UC1J,UC1L,UC1M,UD1A,UD1B,"
                 "UD1F,UD1H,UD1J,UD1L,UD1M,UE1A,UE1B,UE1F,UE1H,UE1J,UE1L,UE1M,UF1A,UF1B,UF1F,UF1H,UF1J,UF1L,UF1M,UG1A,UG1B,UG1F,UG1H,UG1J,UG1L,"
                 "UG1M,UH1A,UH1B,UH1F,UH1H,UH1J,UH1L,UH1M,UI1A,UI1B,UI1F,UI1H,UI1J,UI1L,UI1M"},
    {.continent = "EU",
     .country = "Russia - Leningradskaya",
     .cq_zone = 16,
     .itu_zone = 29,
     .lat = 60.05,
     .lon = 31.75,
     .prefixes = "R1C,R1D,RA1C,RA1D,RC1C,RC1D,RD1C,RD1D,RE1C,RE1D,RF1C,RF1D,RG1C,RG1D,RJ1C,RJ1D,RK1C,RK1D,RL1C,RL1D,RM1C,RM1D,RN1C,RN1D,RO1C,"
                 "RO1D,RQ1C,RQ1D,RT1C,RT1D,RU1C,RU1D,RV1C,RV1D,RW1C,RW1D,RX1C,RX1D,RY1C,RY1D,RZ1C,RZ1D,U1C,U1D,UA1C,UA1D,UB1C,UB1D,UC1C,UC1D,"
                 "UD1C,UD1D,UE1C,UE1D,UF1C,UF1D,UG1C,UG1D,UH1C,UH1D,UI1C,UI1D"},
    {.continent = "EU",
     .country = "Russia - Karelia",
     .cq_zone = 16,
     .itu_zone = 19,
     .lat = 63.82,
     .lon = 33,
     .prefixes = "R1N,RA1N,RC1N,RD1N,RE1N,RF1N,RG1N,RJ1N,RK1N,RL1N,RM1N,RN1N,RO1N,RQ1N,RT1N,RU1N,RV1N,RW1N,RX1N,RY1N,RZ1N,U1N,UA1N,UB1N,UC1N,"
                 "UD1N,UE1N,UF1N,UG1N,UH1N,UI1N"},
    {.continent = "EU",
     .country = "Russia - Arkhangelskaya",
     .cq_zone = 16,
     .itu_zone = 19,
     .lat = 63.50,
     .lon = 43,
     .prefixes = "R1O,RA1O,RC1O,RD1O,RE1O,RF1O,RG1O,RJ1O,RK1O,RL1O,RM1O,RN1O,RO1O,RQ1O,RT1O,RU1O,RV1O,RW1O,RX1O,RY1O,RZ1O,U1O,UA1O,UB1O,UC1O,"
                 "UD1O,UE1O,UF1O,UG1O,UH1O,UI1O"},
    {.continent = "EU",
     .country = "Russia - Nenetsky",
     .cq_zone = 16,
     .itu_zone = 20,
     .lat = 68.83,
     .lon = 54.83,
     .prefixes = "R1P,RA1P,RC1P,RD1P,RE1P,RF1P,RG1P,RJ1P,RK1P,RL1P,RM1P,RN1P,RO1P,RQ1P,RT1P,RU1P,RV1P,RW1P,RX1P,RY1P,RZ1P,U1P,UA1P,UB1P,UC1P,"
                 "UD1P,UE1P,UF1P,UG1P,UH1P,UI1P"},
    {.continent = "EU",
     .country = "Russia - Vologodskaya",
     .cq_zone = 16,
     .itu_zone = 29,
     .lat = 60.08,
     .lon = 40.45,
     .prefixes = "R1Q,R1R,R1S,RA1Q,RA1R,RA1S,RC1Q,RC1R,RC1S,RD1Q,RD1R,RD1S,RE1Q,RE1R,RE1S,RF1Q,RF1R,RF1S,RG1Q,RG1R,RG1S,RJ1Q,RJ1R,RJ1S,RK1Q,"
                 "RK1R,RK1S,RL1Q,RL1R,RL1S,RM1Q,RM1R,RM1S,RN1Q,RN1R,RN1S,RO1Q,RO1R,RO1S,RQ1Q,RQ1R,RQ1S,RT1Q,RT1R,RT1S,RU1Q,RU1R,RU1S,RV1Q,RV1R,"
                 "RV1S,RW1Q,RW1R,RW1S,RX1Q,RX1R,RX1S,RY1Q,RY1R,RY1S,RZ1Q,RZ1R,RZ1S,U1Q,U1R,U1S,UA1Q,UA1R,UA1S,UB1Q,UB1R,UB1S,UC1Q,UC1R,UC1S,"
                 "UD1Q,UD1R,UD1S,UE1Q,UE1R,UE1S,UF1Q,UF1R,UF1S,UG1Q,UG1R,UG1S,UH1Q,UH1R,UH1S,UI1Q,UI1R,UI1S"},
    {.continent = "EU",
     .country = "Russia - Novgorodskaya",
     .cq_zone = 16,
     .itu_zone = 29,
     .lat = 58.43,
     .lon = 32.38,
     .prefixes = "R1T,RA1T,RC1T,RD1T,RE1T,RF1T,RG1T,RJ1T,RK1T,RL1T,RM1T,RN1T,RO1T,RQ1T,RT1T,RU1T,RV1T,RW1T,RX1T,RY1T,RZ1T,U1T,UA1T,UB1T,UC1T,"
                 "UD1T,UE1T,UF1T,UG1T,UH1T,UI1T"},
    {.continent = "EU",
     .country = "Russia - Pskovskaya",
     .cq_zone = 16,
     .itu_zone = 29,
     .lat = 57.32,
     .lon = 29.25,
     .prefixes = "R1W,R1X,RA1W,RA1X,RC1W,RC1X,RD1W,RD1X,RE1W,RE1X,RF1W,RF1X,RG1W,RG1X,RJ1W,RJ1X,RK1W,RK1X,RL1W,RL1X,RM1W,RM1X,RN1W,RN1X,RO1W,"
                 "RO1X,RQ1W,RQ1X,RT1W,RT1X,RU1W,RU1X,RV1W,RV1X,RW1W,RW1X,RX1W,RX1X,RY1W,RY1X,RZ1W,RZ1X,U1W,U1X,UA1W,UA1X,UB1W,UB1X,UC1W,UC1X,"
                 "UD1W,UD1X,UE1W,UE1X,UF1W,UF1X,UG1W,UG1X,UH1W,UH1X,UI1W,UI1X"},
    {.continent = "EU",
     .country = "Russia - Murmanskaya",
     .cq_zone = 16,
     .itu_zone = 19,
     .lat = 68.03,
     .lon = 34.57,
     .prefixes = "R1Z,RA1Z,RC1Z,RD1Z,RE1Z,RF1Z,RG1Z,RJ1Z,RK1Z,RL1Z,RM1Z,RN1Z,RO1Z,RQ1Z,RT1Z,RU1Z,RV1Z,RW1Z,RX1Z,RY1Z,RZ1Z,U1Z,UA1Z,UB1Z,UC1Z,"
                 "UD1Z,UE1Z,UF1Z,UG1Z,UH1Z,UI1Z"},
    {.continent = "EU",
     .country = "Russia - Moscow city",
     .cq_zone = 16,
     .itu_zone = 29,
     .lat = 55.75,
     .lon = 37.62,
     .prefixes = "R2A,R2B,R2C,R3A,R3B,R3C,R5A,R5B,R5C,RA3A,RA3B,RA3C,RA5A,RA5B,RA5C,RC2A,RC2B,RC2C,RC3A,RC3B,RC3C,RC5A,RC5B,RC5C,RD2A,RD2B,RD2C,"
                 "RD3A,RD3B,RD3C,RD5A,RD5B,RD5C,RE2A,RE2B,RE2C,RE3A,RE3B,RE3C,RE5A,RE5B,RE5C,RF2A,RF2B,RF2C,RF3A,RF3B,RF3C,RF5A,RF5B,RF5C,RG2A,"
                 "RG2B,RG2C,RG3A,RG3B,RG3C,RG5A,RG5B,RG5C,RJ2A,RJ2B,RJ2C,RJ3A,RJ3B,RJ3C,RJ5A,RJ5B,RJ5C,RK2A,RK2B,RK2C,RK3A,RK3B,RK3C,RK5A,RK5B,"
                 "RK5C,RL2A,RL2B,RL2C,RL3A,RL3B,RL3C,RL5A,RL5B,RL5C,RM2A,RM2B,RM2C,RM3A,RM3B,RM3C,RM5A,RM5B,RM5C,RN2A,RN2B,RN2C,RN3A,RN3B,RN3C,"
                 "RN5A,RN5B,RN5C,RO2A,RO2B,RO2C,RO3A,RO3B,RO3C,RO5A,RO5B,RO5C,RQ2A,RQ2B,RQ2C,RQ3A,RQ3B,RQ3C,RQ5A,RQ5B,RQ5C,RT2A,RT2B,RT2C,RT3A,"
                 "RT3B,RT3C,RT5A,RT5B,RT5C,RU2A,RU2B,RU2C,RU3A,RU3B,RU3C,RU5A,RU5B,RU5C,RV2A,RV2B,RV2C,RV3A,RV3B,RV3C,RV5A,RV5B,RV5C,RW2A,RW2B,"
                 "RW2C,RW3A,RW3B,RW3C,RW5A,RW5B,RW5C,RX2A,RX2B,RX2C,RX3A,RX3B,RX3C,RX5A,RX5B,RX5C,RY2A,RY2B,RY2C,RY3A,RY3B,RY3C,RY5A,RY5B,RY5C,"
                 "RZ2A,RZ2B,RZ2C,RZ3A,RZ3B,RZ3C,RZ5A,RZ5B,RZ5C,U2A,U2B,U2C,U3A,U3B,U3C,UA3A,UA3B,UA3C,UA5A,UA5B,UA5C,UB3A,UB3B,UB3C,UB5A,UB5B,"
                 "UB5C,UC3A,UC3B,UC3C,UC5A,UC5B,UC5C,UD3A,UD3B,UD3C,UD5A,UD5B,UD5C,UE3A,UE3B,UE3C,UE5A,UE5B,UE5C,UF3A,UF3B,UF3C,UF5A,UF5B,UF5C,"
                 "UG3A,UG3B,UG3C,UG5A,UG5B,UG5C,UH3A,UH3B,UH3C,UH5A,UH5B,UH5C,UI3A,UI3B,UI3C,UI5A,UI5B,UI5C"},
    {.continent = "EU",
     .country = "Russia - Moskovskaya",
     .cq_zone = 16,
     .itu_zone = 29,
     .lat = 55.70,
     .lon = 36.97,
     .prefixes = "R2D,R2H,R3D,R3F,R3H,R5D,R5F,R5H,RA3D,RA3F,RA3H,RA5D,RA5F,RA5H,RC2D,RC2H,RC3D,RC3F,RC3H,RC5D,RC5F,RC5H,RD2D,RD2H,RD3D,RD3F,"
                 "RD3H,RD5D,RD5F,RD5H,RE2D,RE2H,RE3D,RE3F,RE3H,RE5D,RE5F,RE5H,RF2D,RF2H,RF3D,RF3F,RF3H,RF5D,RF5F,RF5H,RG2D,RG2H,RG3D,RG3F,RG3H,"
                 "RG5D,RG5F,RG5H,RJ2D,RJ2H,RJ3D,RJ3F,RJ3H,RJ5D,RJ5F,RJ5H,RK2D,RK2H,RK3D,RK3F,RK3H,RK5D,RK5F,RK5H,RL2D,RL2H,RL3D,RL3F,RL3H,RL5D,"
                 "RL5F,RL5H,RM2D,RM2H,RM3D,RM3F,RM3H,RM5D,RM5F,RM5H,RN2D,RN2H,RN3D,RN3F,RN3H,RN5D,RN5F,RN5H,RO2D,RO2H,RO3D,RO3F,RO3H,RO5D,RO5F,"
                 "RO5H,RQ2D,RQ2H,RQ3D,RQ3F,RQ3H,RQ5D,RQ5F,RQ5H,RT2D,RT2H,RT3D,RT3F,RT3H,RT5D,RT5F,RT5H,RU2D,RU2H,RU3D,RU3F,RU3H,RU5D,RU5F,RU5H,"
                 "RV2D,RV2H,RV3D,RV3F,RV3H,RV5D,RV5F,RV5H,RW2D,RW2H,RW3D,RW3F,RW3H,RW5D,RW5F,RW5H,RX2D,RX2H,RX3D,RX3F,RX3H,RX5D,RX5F,RX5H,RY2D,"
                 "RY2H,RY3D,RY3F,RY3H,RY5D,RY5F,RY5H,RZ2D,RZ2H,RZ3D,RZ3F,RZ3H,RZ5D,RZ5F,RZ5H,U2D,U2H,U3D,U3F,U3H,UA3D,UA3F,UA3H,UA5D,UA5F,UA5H,"
                 "UB3D,UB3F,UB3H,UB5D,UB5F,UB5H,UC3D,UC3F,UC3H,UC5D,UC5F,UC5H,UD3D,UD3F,UD3H,UD5D,UD5F,UD5H,UE3D,UE3F,UE3H,UE5D,UE5F,UE5H,UF3D,"
                 "UF3F,UF3H,UF5D,UF5F,UF5H,UG3D,UG3F,UG3H,UG5D,UG5F,UG5H,UH3D,UH3F,UH3H,UH5D,UH5F,UH5H,UI3D,UI3F,UI3H,UI5D,UI5F,UI5H"},
    {.continent = "EU",
     .country = "Russia - Orlovskaya",
     .cq_zone = 16,
     .itu_zone = 29,
     .lat = 52.85,
     .lon = 36.43,
     .prefixes = "R2E,R3E,R5E,RA3E,RA5E,RC2E,RC3E,RC5E,RD2E,RD3E,RD5E,RE2E,RE3E,RE5E,RF2E,RF3E,RF5E,RG2E,RG3E,RG5E,RJ2E,RJ3E,RJ5E,RK2E,RK3E,"
                 "RK5E,RL2E,RL3E,RL5E,RM2E,RM3E,RM5E,RN2E,RN3E,RN5E,RO2E,RO3E,RO5E,RQ2E,RQ3E,RQ5E,RT2E,RT3E,RT5E,RU2E,RU3E,RU5E,RV2E,RV3E,RV5E,"
                 "RW2E,RW3E,RW5E,RX2E,RX3E,RX5E,RY2E,RY3E,RY5E,RZ2E,RZ3E,RZ5E,U2E,U3E,UA3E,UA5E,UB3E,UB5E,UC3E,UC5E,UD3E,UD5E,UE3E,UE5E,UF3E,"
                 "UF5E,UG3E,UG5E,UH3E,UH5E,UI3E,UI5E"},
    {.continent = "EU",
     .country = "Russia - Lipetskaya",
     .cq_zone = 16,
     .itu_zone = 29,
     .lat = 52.70,
     .lon = 39.15,
     .prefixes = "R2G,R3G,R5G,RA3G,RA5G,RC2G,RC3G,RC5G,RD2G,RD3G,RD5G,RE2G,RE3G,RE5G,RF2G,RF3G,RF5G,RG2G,RG3G,RG5G,RJ2G,RJ3G,RJ5G,RK2G,RK3G,"
                 "RK5G,RL2G,RL3G,RL5G,RM2G,RM3G,RM5G,RN2G,RN3G,RN5G,RO2G,RO3G,RO5G,RQ2G,RQ3G,RQ5G,RT2G,RT3G,RT5G,RU2G,RU3G,RU5G,RV2G,RV3G,RV5G,"
                 "RW2G,RW3G,RW5G,RX2G,RX3G,RX5G,RY2G,RY3G,RY5G,RZ2G,RZ3G,RZ5G,U2G,U3G,UA3G,UA5G,UB3G,UB5G,UC3G,UC5G,UD3G,UD5G,UE3G,UE5G,UF3G,"
                 "UF5G,UG3G,UG5G,UH3G,UH5G,UI3G,UI5G"},
    {.continent = "EU",
     .country = "Russia - Tverskaya",
     .cq_zone = 16,
     .itu_zone = 29,
     .lat = 57.15,
     .lon = 34.62,
     .prefixes = "R2I,R2J,R3I,R3J,R5I,R5J,RA3I,RA3J,RA5I,RA5J,RC2I,RC2J,RC3I,RC3J,RC5I,RC5J,RD2I,RD2J,RD3I,RD3J,RD5I,RD5J,RE2I,RE2J,RE3I,RE3J,"
                 "RE5I,RE5J,RF2I,RF2J,RF3I,RF3J,RF5I,RF5J,RG2I,RG2J,RG3I,RG3J,RG5I,RG5J,RJ2I,RJ2J,RJ3I,RJ3J,RJ5I,RJ5J,RK2I,RK2J,RK3I,RK3J,RK5I,"
                 "RK5J,RL2I,RL2J,RL3I,RL3J,RL5I,RL5J,RM2I,RM2J,RM3I,RM3J,RM5I,RM5J,RN2I,RN2J,RN3I,RN3J,RN5I,RN5J,RO2I,RO2J,RO3I,RO3J,RO5I,RO5J,"
                 "RQ2I,RQ2J,RQ3I,RQ3J,RQ5I,RQ5J,RT2I,RT2J,RT3I,RT3J,RT5I,RT5J,RU2I,RU2J,RU3I,RU3J,RU5I,RU5J,RV2I,RV2J,RV3I,RV3J,RV5I,RV5J,RW2I,"
                 "RW2J,RW3I,RW3J,RW5I,RW5J,RX2I,RX2J,RX3I,RX3J,RX5I,RX5J,RY2I,RY2J,RY3I,RY3J,RY5I,RY5J,RZ2I,RZ2J,RZ3I,RZ3J,RZ5I,RZ5J,U2I,U2J,"
                 "U3I,U3J,UA3I,UA3J,UA5I,UA5J,UB3I,UB3J,UB5I,UB5J,UC3I,UC3J,UC5I,UC5J,UD3I,UD3J,UD5I,UD5J,UE3I,UE3J,UE5I,UE5J,UF3I,UF3J,UF5I,"
                 "UF5J,UG3I,UG3J,UG5I,UG5J,UH3I,UH3J,UH5I,UH5J,UI3I,UI3J,UI5I,UI5J"},
    {.continent = "EU",
     .country = "Russia - Smolenskaya",
     .cq_zone = 16,
     .itu_zone = 29,
     .lat = 55.00,
     .lon = 33,
     .prefixes = "R2L,R3L,R5L,RA3L,RA5L,RC2L,RC3L,RC5L,RD2L,RD3L,RD5L,RE2L,RE3L,RE5L,RF2L,RF3L,RF5L,RG2L,RG3L,RG5L,RJ2L,RJ3L,RJ5L,RK2L,RK3L,"
                 "RK5L,RL2L,RL3L,RL5L,RM2L,RM3L,RM5L,RN2L,RN3L,RN5L,RO2L,RO3L,RO5L,RQ2L,RQ3L,RQ5L,RT2L,RT3L,RT5L,RU2L,RU3L,RU5L,RV2L,RV3L,RV5L,"
                 "RW2L,RW3L,RW5L,RX2L,RX3L,RX5L,RY2L,RY3L,RY5L,RZ2L,RZ3L,RZ5L,U2L,U3L,UA3L,UA5L,UB3L,UB5L,UC3L,UC5L,UD3L,UD5L,UE3L,UE5L,UF3L,"
                 "UF5L,UG3L,UG5L,UH3L,UH5L,UI3L,UI5L"},
    {.continent = "EU",
     .country = "Russia - Yaroslavskaya",
     .cq_zone = 16,
     .itu_zone = 29,
     .lat = 57.87,
     .lon = 39.2,
     .prefixes = "R2M,R3M,R5M,RA3M,RA5M,RC2M,RC3M,RC5M,RD2M,RD3M,RD5M,RE2M,RE3M,RE5M,RF2M,RF3M,RF5M,RG2M,RG3M,RG5M,RJ2M,RJ3M,RJ5M,RK2M,RK3M,"
                 "RK5M,RL2M,RL3M,RL5M,RM2M,RM3M,RM5M,RN2M,RN3M,RN5M,RO2M,RO3M,RO5M,RQ2M,RQ3M,RQ5M,RT2M,RT3M,RT5M,RU2M,RU3M,RU5M,RV2M,RV3M,RV5M,"
                 "RW2M,RW3M,RW5M,RX2M,RX3M,RX5M,RY2M,RY3M,RY5M,RZ2M,RZ3M,RZ5M,U2M,U3M,UA3M,UA5M,UB3M,UB5M,UC3M,UC5M,UD3M,UD5M,UE3M,UE5M,UF3M,"
                 "UF5M,UG3M,UG5M,UH3M,UH5M,UI3M,UI5M"},
    {.continent = "EU",
     .country = "Russia - Kostromskaya",
     .cq_zone = 16,
     .itu_zone = 29,
     .lat = 58.55,
     .lon = 43.68,
     .prefixes = "R2N,R3N,R5N,RA3N,RA5N,RC2N,RC3N,RC5N,RD2N,RD3N,RD5N,RE2N,RE3N,RE5N,RF2N,RF3N,RF5N,RG2N,RG3N,RG5N,RJ2N,RJ3N,RJ5N,RK2N,RK3N,"
                 "RK5N,RL2N,RL3N,RL5N,RM2N,RM3N,RM5N,RN2N,RN3N,RN5N,RO2N,RO3N,RO5N,RQ2N,RQ3N,RQ5N,RT2N,RT3N,RT5N,RU2N,RU3N,RU5N,RV2N,RV3N,RV5N,"
                 "RW2N,RW3N,RW5N,RX2N,RX3N,RX5N,RY2N,RY3N,RY5N,RZ2N,RZ3N,RZ5N,U2N,U3N,UA3N,UA5N,UB3N,UB5N,UC3N,UC5N,UD3N,UD5N,UE3N,UE5N,UF3N,"
                 "UF5N,UG3N,UG5N,UH3N,UH5N,UI3N,UI5N"},
    {.continent = "EU",
     .country = "Russia - Tulskaya",
     .cq_zone = 16,
     .itu_zone = 29,
     .lat = 53.92,
     .lon = 37.58,
     .prefixes = "R2P,R3P,R5P,RA3P,RA5P,RC2P,RC3P,RC5P,RD2P,RD3P,RD5P,RE2P,RE3P,RE5P,RF2P,RF3P,RF5P,RG2P,RG3P,RG5P,RJ2P,RJ3P,RJ5P,RK2P,RK3P,"
                 "RK5P,RL2P,RL3P,RL5P,RM2P,RM3P,RM5P,RN2P,RN3P,RN5P,RO2P,RO3P,RO5P,RQ2P,RQ3P,RQ5P,RT2P,RT3P,RT5P,RU2P,RU3P,RU5P,RV2P,RV3P,RV5P,"
                 "RW2P,RW3P,RW5P,RX2P,RX3P,RX5P,RY2P,RY3P,RY5P,RZ2P,RZ3P,RZ5P,U2P,U3P,UA3P,UA5P,UB3P,UB5P,UC3P,UC5P,UD3P,UD5P,UE3P,UE5P,UF3P,"
                 "UF5P,UG3P,UG5P,UH3P,UH5P,UI3P,UI5P"},
    {.continent = "EU",
     .country = "Russia - Voronezhskaya",
     .cq_zone = 16,
     .itu_zone = 29,
     .lat = 51.05,
     .lon = 40.15,
     .prefixes = "R2O,R2Q,R3K,R3O,R3Q,R5K,R5O,R5Q,RA3K,RA3O,RA3Q,RA5K,RA5O,RA5Q,RC2O,RC2Q,RC3K,RC3O,RC3Q,RC5K,RC5O,RC5Q,RD2O,RD2Q,RD3K,RD3O,"
                 "RD3Q,RD5K,RD5O,RD5Q,RE2O,RE2Q,RE3K,RE3O,RE3Q,RE5K,RE5O,RE5Q,RF2O,RF2Q,RF3K,RF3O,RF3Q,RF5K,RF5O,RF5Q,RG2O,RG2Q,RG3K,RG3O,RG3Q,"
                 "RG5K,RG5O,RG5Q,RJ2O,RJ2Q,RJ3K,RJ3O,RJ3Q,RJ5K,RJ5O,RJ5Q,RK2O,RK2Q,RK3K,RK3O,RK3Q,RK5K,RK5O,RK5Q,RL2O,RL2Q,RL3K,RL3O,RL3Q,RL5K,"
                 "RL5O,RL5Q,RM2O,RM2Q,RM3K,RM3O,RM3Q,RM5K,RM5O,RM5Q,RN2O,RN2Q,RN3K,RN3O,RN3Q,RN5K,RN5O,RN5Q,RO2O,RO2Q,RO3K,RO3O,RO3Q,RO5K,RO5O,"
                 "RO5Q,RQ2O,RQ2Q,RQ3K,RQ3O,RQ3Q,RQ5K,RQ5O,RQ5Q,RT2O,RT2Q,RT3K,RT3O,RT3Q,RT5K,RT5O,RT5Q,RU2O,RU2Q,RU3K,RU3O,RU3Q,RU5K,RU5O,RU5Q,"
                 "RV2O,RV2Q,RV3K,RV3O,RV3Q,RV5K,RV5O,RV5Q,RW2O,RW2Q,RW3K,RW3O,RW3Q,RW5K,RW5O,RW5Q,RX2O,RX2Q,RX3K,RX3O,RX3Q,RX5K,RX5O,RX5Q,RY2O,"
                 "RY2Q,RY3K,RY3O,RY3Q,RY5K,RY5O,RY5Q,RZ2O,RZ2Q,RZ3K,RZ3O,RZ3Q,RZ5K,RZ5O,RZ5Q,U2O,U2Q,U3K,U3O,U3Q,UA3K,UA3O,UA3Q,UA5K,UA5O,UA5Q,"
                 "UB3K,UB3O,UB3Q,UB5K,UB5O,UB5Q,UC3K,UC3O,UC3Q,UC5K,UC5O,UC5Q,UD3K,UD3O,UD3Q,UD5K,UD5O,UD5Q,UE3K,UE3O,UE3Q,UE5K,UE5O,UE5Q,UF3K,"
                 "UF3O,UF3Q,UF5K,UF5O,UF5Q,UG3K,UG3O,UG3Q,UG5K,UG5O,UG5Q,UH3K,UH3O,UH3Q,UH5K,UH5O,UH5Q,UI3K,UI3O,UI3Q,UI5K,UI5O,UI5Q"},
    {.continent = "EU",
     .country = "Russia - Tambovskaya",
     .cq_zone = 16,
     .itu_zone = 29,
     .lat = 52.72,
     .lon = 41.57,
     .prefixes = "R2R,R3R,R5R,RA3R,RA5R,RC2R,RC3R,RC5R,RD2R,RD3R,RD5R,RE2R,RE3R,RE5R,RF2R,RF3R,RF5R,RG2R,RG3R,RG5R,RJ2R,RJ3R,RJ5R,RK2R,RK3R,"
                 "RK5R,RL2R,RL3R,RL5R,RM2R,RM3R,RM5R,RN2R,RN3R,RN5R,RO2R,RO3R,RO5R,RQ2R,RQ3R,RQ5R,RT2R,RT3R,RT5R,RU2R,RU3R,RU5R,RV2R,RV3R,RV5R,"
                 "RW2R,RW3R,RW5R,RX2R,RX3R,RX5R,RY2R,RY3R,RY5R,RZ2R,RZ3R,RZ5R,U2R,U3R,UA3R,UA5R,UB3R,UB5R,UC3R,UC5R,UD3R,UD5R,UE3R,UE5R,UF3R,"
                 "UF5R,UG3R,UG5R,UH3R,UH5R,UI3R,UI5R"},
    {.continent = "EU",
     .country = "Russia - Ryazanskaya",
     .cq_zone = 16,
     .itu_zone = 29,
     .lat = 54.40,
     .lon = 40.6,
     .prefixes = "R2S,R3S,R5S,RA3S,RA5S,RC2S,RC3S,RC5S,RD2S,RD3S,RD5S,RE2S,RE3S,RE5S,RF2S,RF3S,RF5S,RG2S,RG3S,RG5S,RJ2S,RJ3S,RJ5S,RK2S,RK3S,"
                 "RK5S,RL2S,RL3S,RL5S,RM2S,RM3S,RM5S,RN2S,RN3S,RN5S,RO2S,RO3S,RO5S,RQ2S,RQ3S,RQ5S,RT2S,RT3S,RT5S,RU2S,RU3S,RU5S,RV2S,RV3S,RV5S,"
                 "RW2S,RW3S,RW5S,RX2S,RX3S,RX5S,RY2S,RY3S,RY5S,RZ2S,RZ3S,RZ5S,U2S,U3S,UA3S,UA5S,UB3S,UB5S,UC3S,UC5S,UD3S,UD5S,UE3S,UE5S,UF3S,"
                 "UF5S,UG3S,UG5S,UH3S,UH5S,UI3S,UI5S"},
    {.continent = "EU",
     .country = "Russia - Nizhegorodskaya",
     .cq_zone = 16,
     .itu_zone = 29,
     .lat = 56.48,
     .lon = 44.53,
     .prefixes = "R2T,R3T,R5T,RA3T,RA5T,RC2T,RC3T,RC5T,RD2T,RD3T,RD5T,RE2T,RE3T,RE5T,RF2T,RF3T,RF5T,RG2T,RG3T,RG5T,RJ2T,RJ3T,RJ5T,RK2T,RK3T,"
                 "RK5T,RL2T,RL3T,RL5T,RM2T,RM3T,RM5T,RN2T,RN3T,RN5T,RO2T,RO3T,RO5T,RQ2T,RQ3T,RQ5T,RT2T,RT3T,RT5T,RU2T,RU3T,RU5T,RV2T,RV3T,RV5T,"
                 "RW2T,RW3T,RW5T,RX2T,RX3T,RX5T,RY2T,RY3T,RY5T,RZ2T,RZ3T,RZ5T,U2T,U3T,UA3T,UA5T,UB3T,UB5T,UC3T,UC5T,UD3T,UD5T,UE3T,UE5T,UF3T,"
                 "UF5T,UG3T,UG5T,UH3T,UH5T,UI3T,UI5T"},
    {.continent = "EU",
     .country = "Russia - Ivanovskaya",
     .cq_zone = 16,
     .itu_zone = 29,
     .lat = 57.02,
     .lon = 41.52,
     .prefixes = "R2U,R3U,R5U,RA3U,RA5U,RC2U,RC3U,RC5U,RD2U,RD3U,RD5U,RE2U,RE3U,RE5U,RF2U,RF3U,RF5U,RG2U,RG3U,RG5U,RJ2U,RJ3U,RJ5U,RK2U,RK3U,"
                 "RK5U,RL2U,RL3U,RL5U,RM2U,RM3U,RM5U,RN2U,RN3U,RN5U,RO2U,RO3U,RO5U,RQ2U,RQ3U,RQ5U,RT2U,RT3U,RT5U,RU2U,RU3U,RU5U,RV2U,RV3U,RV5U,"
                 "RW2U,RW3U,RW5U,RX2U,RX3U,RX5U,RY2U,RY3U,RY5U,RZ2U,RZ3U,RZ5U,U2U,U3U,UA3U,UA5U,UB3U,UB5U,UC3U,UC5U,UD3U,UD5U,UE3U,UE5U,UF3U,"
                 "UF5U,UG3U,UG5U,UH3U,UH5U,UI3U,UI5U"},
    {.continent = "EU",
     .country = "Russia - Vladimirskaya",
     .cq_zone = 16,
     .itu_zone = 29,
     .lat = 56.08,
     .lon = 40.62,
     .prefixes = "R2V,R3V,R5V,RA3V,RA5V,RC2V,RC3V,RC5V,RD2V,RD3V,RD5V,RE2V,RE3V,RE5V,RF2V,RF3V,RF5V,RG2V,RG3V,RG5V,RJ2V,RJ3V,RJ5V,RK2V,RK3V,"
                 "RK5V,RL2V,RL3V,RL5V,RM2V,RM3V,RM5V,RN2V,RN3V,RN5V,RO2V,RO3V,RO5V,RQ2V,RQ3V,RQ5V,RT2V,RT3V,RT5V,RU2V,RU3V,RU5V,RV2V,RV3V,RV5V,"
                 "RW2V,RW3V,RW5V,RX2V,RX3V,RX5V,RY2V,RY3V,RY5V,RZ2V,RZ3V,RZ5V,U2V,U3V,UA3V,UA5V,UB3V,UB5V,UC3V,UC5V,UD3V,UD5V,UE3V,UE5V,UF3V,"
                 "UF5V,UG3V,UG5V,UH3V,UH5V,UI3V,UI5V"},
    {.continent = "EU",
     .country = "Russia - Kurskaya",
     .cq_zone = 16,
     .itu_zone = 29,
     .lat = 51.75,
     .lon = 36.02,
     .prefixes = "R2W,R3W,R5W,RA3W,RA5W,RC2W,RC3W,RC5W,RD2W,RD3W,RD5W,RE2W,RE3W,RE5W,RF2W,RF3W,RF5W,RG2W,RG3W,RG5W,RJ2W,RJ3W,RJ5W,RK2W,RK3W,"
                 "RK5W,RL2W,RL3W,RL5W,RM2W,RM3W,RM5W,RN2W,RN3W,RN5W,RO2W,RO3W,RO5W,RQ2W,RQ3W,RQ5W,RT2W,RT3W,RT5W,RU2W,RU3W,RU5W,RV2W,RV3W,RV5W,"
                 "RW2W,RW3W,RW5W,RX2W,RX3W,RX5W,RY2W,RY3W,RY5W,RZ2W,RZ3W,RZ5W,U2W,U3W,UA3W,UA5W,UB3W,UB5W,UC3W,UC5W,UD3W,UD5W,UE3W,UE5W,UF3W,"
                 "UF5W,UG3W,UG5W,UH3W,UH5W,UI3W,UI5W"},
    {.continent = "EU",
     .country = "Russia - Kaluzhskaya",
     .cq_zone = 16,
     .itu_zone = 29,
     .lat = 54.43,
     .lon = 35.43,
     .prefixes = "R2X,R3X,R5X,RA3X,RA5X,RC2X,RC3X,RC5X,RD2X,RD3X,RD5X,RE2X,RE3X,RE5X,RF2X,RF3X,RF5X,RG2X,RG3X,RG5X,RJ2X,RJ3X,RJ5X,RK2X,RK3X,"
                 "RK5X,RL2X,RL3X,RL5X,RM2X,RM3X,RM5X,RN2X,RN3X,RN5X,RO2X,RO3X,RO5X,RQ2X,RQ3X,RQ5X,RT2X,RT3X,RT5X,RU2X,RU3X,RU5X,RV2X,RV3X,RV5X,"
                 "RW2X,RW3X,RW5X,RX2X,RX3X,RX5X,RY2X,RY3X,RY5X,RZ2X,RZ3X,RZ5X,U2X,U3X,UA3X,UA5X,UB3X,UB5X,UC3X,UC5X,UD3X,UD5X,UE3X,UE5X,UF3X,"
                 "UF5X,UG3X,UG5X,UH3X,UH5X,UI3X,UI5X"},
    {.continent = "EU",
     .country = "Russia - Bryanskaya",
     .cq_zone = 16,
     .itu_zone = 29,
     .lat = 52.95,
     .lon = 33.4,
     .prefixes = "R2Y,R3Y,R5Y,RA3Y,RA5Y,RC2Y,RC3Y,RC5Y,RD2Y,RD3Y,RD5Y,RE2Y,RE3Y,RE5Y,RF2Y,RF3Y,RF5Y,RG2Y,RG3Y,RG5Y,RJ2Y,RJ3Y,RJ5Y,RK2Y,RK3Y,"
                 "RK5Y,RL2Y,RL3Y,RL5Y,RM2Y,RM3Y,RM5Y,RN2Y,RN3Y,RN5Y,RO2Y,RO3Y,RO5Y,RQ2Y,RQ3Y,RQ5Y,RT2Y,RT3Y,RT5Y,RU2Y,RU3Y,RU5Y,RV2Y,RV3Y,RV5Y,"
                 "RW2Y,RW3Y,RW5Y,RX2Y,RX3Y,RX5Y,RY2Y,RY3Y,RY5Y,RZ2Y,RZ3Y,RZ5Y,U2Y,U3Y,UA3Y,UA5Y,UB3Y,UB5Y,UC3Y,UC5Y,UD3Y,UD5Y,UE3Y,UE5Y,UF3Y,"
                 "UF5Y,UG3Y,UG5Y,UH3Y,UH5Y,UI3Y,UI5Y"},
    {.continent = "EU",
     .country = "Russia - Belgorodskaya",
     .cq_zone = 16,
     .itu_zone = 29,
     .lat = 50.77,
     .lon = 37.45,
     .prefixes = "R2Z,R3Z,R5Z,RA3Z,RA5Z,RC2Z,RC3Z,RC5Z,RD2Z,RD3Z,RD5Z,RE2Z,RE3Z,RE5Z,RF2Z,RF3Z,RF5Z,RG2Z,RG3Z,RG5Z,RJ2Z,RJ3Z,RJ5Z,RK2Z,RK3Z,"
                 "RK5Z,RL2Z,RL3Z,RL5Z,RM2Z,RM3Z,RM5Z,RN2Z,RN3Z,RN5Z,RO2Z,RO3Z,RO5Z,RQ2Z,RQ3Z,RQ5Z,RT2Z,RT3Z,RT5Z,RU2Z,RU3Z,RU5Z,RV2Z,RV3Z,RV5Z,"
                 "RW2Z,RW3Z,RW5Z,RX2Z,RX3Z,RX5Z,RY2Z,RY3Z,RY5Z,RZ2Z,RZ3Z,RZ5Z,U2Z,U3Z,UA3Z,UA5Z,UB3Z,UB5Z,UC3Z,UC5Z,UD3Z,UD5Z,UE3Z,UE5Z,UF3Z,"
                 "UF5Z,UG3Z,UG5Z,UH3Z,UH5Z,UI3Z,UI5Z"},
    {.continent = "EU",
     .country = "Russia - Volgogradskaya",
     .cq_zone = 16,
     .itu_zone = 29,
     .lat = 49.73,
     .lon = 44.12,
     .prefixes = "R4A,R4B,RA4A,RA4B,RC4A,RC4B,RD4A,RD4B,RE4A,RE4B,RF4A,RF4B,RG4A,RG4B,RJ4A,RJ4B,RK4A,RK4B,RL4A,RL4B,RM4A,RM4B,RN4A,RN4B,RO4A,"
                 "RO4B,RQ4A,RQ4B,RT4A,RT4B,RU4A,RU4B,RV4A,RV4B,RW4A,RW4B,RX4A,RX4B,RY4A,RY4B,RZ4A,RZ4B,U4A,U4B,UA4A,UA4B,UB4A,UB4B,UC4A,UC4B,"
                 "UD4A,UD4B,UE4A,UE4B,UF4A,UF4B,UG4A,UG4B,UH4A,UH4B,UI4A,UI4B"},
    {.continent = "EU",
     .country = "Russia - Saratovskaya",
     .cq_zone = 16,
     .itu_zone = 29,
     .lat = 51.78,
     .lon = 46.73,
     .prefixes = "R4C,R4D,RA4C,RA4D,RC4C,RC4D,RD4C,RD4D,RE4C,RE4D,RF4C,RF4D,RG4C,RG4D,RJ4C,RJ4D,RK4C,RK4D,RL4C,RL4D,RM4C,RM4D,RN4C,RN4D,RO4C,"
                 "RO4D,RQ4C,RQ4D,RT4C,RT4D,RU4C,RU4D,RV4C,RV4D,RW4C,RW4D,RX4C,RX4D,RY4C,RY4D,RZ4C,RZ4D,U4C,U4D,UA4C,UA4D,UB4C,UB4D,UC4C,UC4D,"
                 "UD4C,UD4D,UE4C,UE4D,UF4C,UF4D,UG4C,UG4D,UH4C,UH4D,UI4C,UI4D"},
    {.continent = "EU",
     .country = "Russia - Penzenskaya",
     .cq_zone = 16,
     .itu_zone = 29,
     .lat = 53.25,
     .lon = 44.57,
     .prefixes = "R4F,RA4F,RC4F,RD4F,RE4F,RF4F,RG4F,RJ4F,RK4F,RL4F,RM4F,RN4F,RO4F,RQ4F,RT4F,RU4F,RV4F,RW4F,RX4F,RY4F,RZ4F,U4F,UA4F,UB4F,UC4F,"
                 "UD4F,UE4F,UF4F,UG4F,UH4F,UI4F"},
    {.continent = "EU",
     .country = "Russia - Samarskaya",
     .cq_zone = 16,
     .itu_zone = 30,
     .lat = 53.45,
     .lon = 50.45,
     .prefixes = "R4H,R4I,RA4H,RA4I,RC4H,RC4I,RD4H,RD4I,RE4H,RE4I,RF4H,RF4I,RG4H,RG4I,RJ4H,RJ4I,RK4H,RK4I,RL4H,RL4I,RM4H,RM4I,RN4H,RN4I,RO4H,"
                 "RO4I,RQ4H,RQ4I,RT4H,RT4I,RU4H,RU4I,RV4H,RV4I,RW4H,RW4I,RX4H,RX4I,RY4H,RY4I,RZ4H,RZ4I,U4H,U4I,UA4H,UA4I,UB4H,UB4I,UC4H,UC4I,"
                 "UD4H,UD4I,UE4H,UE4I,UF4H,UF4I,UG4H,UG4I,UH4H,UH4I,UI4H,UI4I"},
    {.continent = "EU",
     .country = "Russia - Ulyanovskaya",
     .cq_zone = 16,
     .itu_zone = 29,
     .lat = 53.95,
     .lon = 47.92,
     .prefixes = "R4L,R4M,RA4L,RA4M,RC4L,RC4M,RD4L,RD4M,RE4L,RE4M,RF4L,RF4M,RG4L,RG4M,RJ4L,RJ4M,RK4L,RK4M,RL4L,RL4M,RM4L,RM4M,RN4L,RN4M,RO4L,"
                 "RO4M,RQ4L,RQ4M,RT4L,RT4M,RU4L,RU4M,RV4L,RV4M,RW4L,RW4M,RX4L,RX4M,RY4L,RY4M,RZ4L,RZ4M,U4L,U4M,UA4L,UA4M,UB4L,UB4M,UC4L,UC4M,"
                 "UD4L,UD4M,UE4L,UE4M,UF4L,UF4M,UG4L,UG4M,UH4L,UH4M,UI4L,UI4M"},
    {.continent = "EU",
     .country = "Russia - Kirovskaya",
     .cq_zone = 16,
     .itu_zone = 29,
     .lat = 58.77,
     .lon = 49.83,
     .prefixes = "R4N,R4O,RA4N,RA4O,RC4N,RC4O,RD4N,RD4O,RE4N,RE4O,RF4N,RF4O,RG4N,RG4O,RJ4N,RJ4O,RK4N,RK4O,RL4N,RL4O,RM4N,RM4O,RN4N,RN4O,RO4N,"
                 "RO4O,RQ4N,RQ4O,RT4N,RT4O,RU4N,RU4O,RV4N,RV4O,RW4N,RW4O,RX4N,RX4O,RY4N,RY4O,RZ4N,RZ4O,U4N,U4O,UA4N,UA4O,UB4N,UB4O,UC4N,UC4O,"
                 "UD4N,UD4O,UE4N,UE4O,UF4N,UF4O,UG4N,UG4O,UH4N,UH4O,UI4N,UI4O"},
    {.continent = "EU",
     .country = "Russia - Tatarstan",
     .cq_zone = 16,
     .itu_zone = 29,
     .lat = 55.55,
     .lon = 50.93,
     .prefixes = "R4P,R4Q,R4R,RA4P,RA4Q,RA4R,RC4P,RC4Q,RC4R,RD4P,RD4Q,RD4R,RE4P,RE4Q,RE4R,RF4P,RF4Q,RF4R,RG4P,RG4Q,RG4R,RJ4P,RJ4Q,RJ4R,RK4P,"
                 "RK4Q,RK4R,RL4P,RL4Q,RL4R,RM4P,RM4Q,RM4R,RN4P,RN4Q,RN4R,RO4P,RO4Q,RO4R,RQ4P,RQ4Q,RQ4R,RT4P,RT4Q,RT4R,RU4P,RU4Q,RU4R,RV4P,RV4Q,"
                 "RV4R,RW4P,RW4Q,RW4R,RX4P,RX4Q,RX4R,RY4P,RY4Q,RY4R,RZ4P,RZ4Q,RZ4R,U4P,U4Q,U4R,UA4P,UA4Q,UA4R,UB4P,UB4Q,UB4R,UC4P,UC4Q,UC4R,"
                 "UD4P,UD4Q,UD4R,UE4P,UE4Q,UE4R,UF4P,UF4Q,UF4R,UG4P,UG4Q,UG4R,UH4P,UH4Q,UH4R,UI4P,UI4Q,UI4R"},
    {.continent = "EU",
     .country = "Russia - Marij-El",
     .cq_zone = 16,
     .itu_zone = 29,
     .lat = 56.70,
     .lon = 47.87,
     .prefixes = "R4S,R4T,RA4S,RA4T,RC4S,RC4T,RD4S,RD4T,RE4S,RE4T,RF4S,RF4T,RG4S,RG4T,RJ4S,RJ4T,RK4S,RK4T,RL4S,RL4T,RM4S,RM4T,RN4S,RN4T,RO4S,"
                 "RO4T,RQ4S,RQ4T,RT4S,RT4T,RU4S,RU4T,RV4S,RV4T,RW4S,RW4T,RX4S,RX4T,RY4S,RY4T,RZ4S,RZ4T,U4S,U4T,UA4S,UA4T,UB4S,UB4T,UC4S,UC4T,"
                 "UD4S,UD4T,UE4S,UE4T,UF4S,UF4T,UG4S,UG4T,UH4S,UH4T,UI4S,UI4T"},
    {.continent = "EU",
     .country = "Russia - Mordoviya",
     .cq_zone = 16,
     .itu_zone = 29,
     .lat = 54.43,
     .lon = 44.45,
     .prefixes = "R4U,RA4U,RC4U,RD4U,RE4U,RF4U,RG4U,RJ4U,RK4U,RL4U,RM4U,RN4U,RO4U,RQ4U,RT4U,RU4U,RV4U,RW4U,RX4U,RY4U,RZ4U,U4U,UA4U,UB4U,UC4U,"
                 "UD4U,UE4U,UF4U,UG4U,UH4U,UI4U"},
    {.continent = "EU",
     .country = "Russia - Udmurtiya",
     .cq_zone = 16,
     .itu_zone = 30,
     .lat = 57.28,
     .lon = 52.75,
     .prefixes = "R4W,RA4W,RC4W,RD4W,RE4W,RF4W,RG4W,RJ4W,RK4W,RL4W,RM4W,RN4W,RO4W,RQ4W,RT4W,RU4W,RV4W,RW4W,RX4W,RY4W,RZ4W,U4W,UA4W,UB4W,UC4W,"
                 "UD4W,UE4W,UF4W,UG4W,UH4W,UI4W"},
    {.continent = "EU",
     .country = "Russia - Chuvashiya",
     .cq_zone = 16,
     .itu_zone = 29,
     .lat = 55.55,
     .lon = 47.1,
     .prefixes = "R4Y,R4Z,RA4Y,RA4Z,RC4Y,RC4Z,RD4Y,RD4Z,RE4Y,RE4Z,RF4Y,RF4Z,RG4Y,RG4Z,RJ4Y,RJ4Z,RK4Y,RK4Z,RL4Y,RL4Z,RM4Y,RM4Z,RN4Y,RN4Z,RO4Y,"
                 "RO4Z,RQ4Y,RQ4Z,RT4Y,RT4Z,RU4Y,RU4Z,RV4Y,RV4Z,RW4Y,RW4Z,RX4Y,RX4Z,RY4Y,RY4Z,RZ4Y,RZ4Z,U4Y,U4Z,UA4Y,UA4Z,UB4Y,UB4Z,UC4Y,UC4Z,"
                 "UD4Y,UD4Z,UE4Y,UE4Z,UF4Y,UF4Z,UG4Y,UG4Z,UH4Y,UH4Z,UI4Y,UI4Z"},
    {.continent = "EU",
     .country = "Russia - Krasnodarskiy",
     .cq_zone = 16,
     .itu_zone = 29,
     .lat = 45.37,
     .lon = 39.43,
     .prefixes = "R6A,R6B,R6C,R6D,R7A,R7B,R7C,R7D,RA6A,RA6B,RA6C,RA6D,RA7A,RA7B,RA7C,RA7D,RC6A,RC6B,RC6C,RC6D,RC7A,RC7B,RC7C,RC7D,RD6A,RD6B,"
                 "RD6C,RD6D,RD7A,RD7B,RD7C,RD7D,RE6A,RE6B,RE6C,RE6D,RE7A,RE7B,RE7C,RE7D,RF6A,RF6B,RF6C,RF6D,RF7A,RF7B,RF7C,RF7D,RG6A,RG6B,RG6C,"
                 "RG6D,RG7A,RG7B,RG7C,RG7D,RJ6A,RJ6B,RJ6C,RJ6D,RJ7A,RJ7B,RJ7C,RJ7D,RK6A,RK6B,RK6C,RK6D,RK7A,RK7B,RK7C,RK7D,RL6A,RL6B,RL6C,RL6D,"
                 "RL7A,RL7B,RL7C,RL7D,RM6A,RM6B,RM6C,RM6D,RM7A,RM7B,RM7C,RM7D,RN6A,RN6B,RN6C,RN6D,RN7A,RN7B,RN7C,RN7D,RO6A,RO6B,RO6C,RO6D,RO7A,"
                 "RO7B,RO7C,RO7D,RQ6A,RQ6B,RQ6C,RQ6D,RQ7A,RQ7B,RQ7C,RQ7D,RT6A,RT6B,RT6C,RT6D,RT7A,RT7B,RT7C,RT7D,RU6A,RU6B,RU6C,RU6D,RU7A,RU7B,"
                 "RU7C,RU7D,RV6A,RV6B,RV6C,RV6D,RV7A,RV7B,RV7C,RV7D,RW6A,RW6B,RW6C,RW6D,RW7A,RW7B,RW7C,RW7D,RX6A,RX6B,RX6C,RX6D,RX7A,RX7B,RX7C,"
                 "RX7D,RY6A,RY6B,RY6C,RY6D,RY7A,RY7B,RY7C,RY7D,RZ6A,RZ6B,RZ6C,RZ6D,RZ7A,RZ7B,RZ7C,RZ7D,U6A,U6B,U6C,U6D,U7A,U7B,U7C,U7D,UA6A,"
                 "UA6B,UA6C,UA6D,UA7A,UA7B,UA7C,UA7D,UB6A,UB6B,UB6C,UB6D,UB7A,UB7B,UB7C,UB7D,UC6A,UC6B,UC6C,UC6D,UC7A,UC7B,UC7C,UC7D,UD6A,UD6B,"
                 "UD6C,UD6D,UD7A,UD7B,UD7C,UD7D,UE6A,UE6B,UE6C,UE6D,UE7A,UE7B,UE7C,UE7D,UF6A,UF6B,UF6C,UF6D,UF7A,UF7B,UF7C,UF7D,UG6A,UG6B,UG6C,"
                 "UG6D,UG7A,UG7B,UG7C,UG7D,UH6A,UH6B,UH6C,UH6D,UH7A,UH7B,UH7C,UH7D,UI6A,UI6B,UI6C,UI6D,UI7A,UI7B,UI7C,UI7D"},
    {.continent = "EU",
     .country = "Russia - Karachay-Cherkessia",
     .cq_zone = 16,
     .itu_zone = 29,
     .lat = 43.92,
     .lon = 41.78,
     .prefixes = "R6E,R7E,RA6E,RA7E,RC6E,RC7E,RD6E,RD7E,RE6E,RE7E,RF6E,RF7E,RG6E,RG7E,RJ6E,RJ7E,RK6E,RK7E,RL6E,RL7E,RM6E,RM7E,RN6E,RN7E,RO6E,"
                 "RO7E,RQ6E,RQ7E,RT6E,RT7E,RU6E,RU7E,RV6E,RV7E,RW6E,RW7E,RX6E,RX7E,RY6E,RY7E,RZ6E,RZ7E,U6E,U7E,UA6E,UA7E,UB6E,UB7E,UC6E,UC7E,"
                 "UD6E,UD7E,UE6E,UE7E,UF6E,UF7E,UG6E,UG7E,UH6E,UH7E,UI6E,UI7E"},
    {.continent = "EU",
     .country = "Russia - Stavropolsky",
     .cq_zone = 16,
     .itu_zone = 29,
     .lat = 45.05,
     .lon = 43.27,
     .prefixes = "R6F,R6G,R6H,R6T,R7F,R7G,R7H,R7T,RA6F,RA6G,RA6H,RA6T,RA7F,RA7G,RA7H,RA7T,RC6F,RC6G,RC6H,RC6T,RC7F,RC7G,RC7H,RC7T,RD6F,RD6G,"
                 "RD6H,RD6T,RD7F,RD7G,RD7H,RD7T,RE6F,RE6G,RE6H,RE6T,RE7F,RE7G,RE7H,RE7T,RF6F,RF6G,RF6H,RF6T,RF7F,RF7G,RF7H,RF7T,RG6F,RG6G,RG6H,"
                 "RG6T,RG7F,RG7G,RG7H,RG7T,RJ6F,RJ6G,RJ6H,RJ6T,RJ7F,RJ7G,RJ7H,RJ7T,RK6F,RK6G,RK6H,RK6T,RK7F,RK7G,RK7H,RK7T,RL6F,RL6G,RL6H,RL6T,"
                 "RL7F,RL7G,RL7H,RL7T,RM6F,RM6G,RM6H,RM6T,RM7F,RM7G,RM7H,RM7T,RN6F,RN6G,RN6H,RN6T,RN7F,RN7G,RN7H,RN7T,RO6F,RO6G,RO6H,RO6T,RO7F,"
                 "RO7G,RO7H,RO7T,RQ6F,RQ6G,RQ6H,RQ6T,RQ7F,RQ7G,RQ7H,RQ7T,RT6F,RT6G,RT6H,RT6T,RT7F,RT7G,RT7H,RT7T,RU6F,RU6G,RU6H,RU6T,RU7F,RU7G,"
                 "RU7H,RU7T,RV6F,RV6G,RV6H,RV6T,RV7F,RV7G,RV7H,RV7T,RW6F,RW6G,RW6H,RW6T,RW7F,RW7G,RW7H,RW7T,RX6F,RX6G,RX6H,RX6T,RX7F,RX7G,RX7H,"
                 "RX7T,RY6F,RY6G,RY6H,RY6T,RY7F,RY7G,RY7H,RY7T,RZ6F,RZ6G,RZ6H,RZ6T,RZ7F,RZ7G,RZ7H,RZ7T,U6F,U6G,U6H,U6T,U7F,U7G,U7H,U7T,UA6F,"
                 "UA6G,UA6H,UA6T,UA7F,UA7G,UA7H,UA7T,UB6F,UB6G,UB6H,UB6T,UB7F,UB7G,UB7H,UB7T,UC6F,UC6G,UC6H,UC6T,UC7F,UC7G,UC7H,UC7T,UD6F,UD6G,"
                 "UD6H,UD6T,UD7F,UD7G,UD7H,UD7T,UE6F,UE6G,UE6H,UE6T,UE7F,UE7G,UE7H,UE7T,UF6F,UF6G,UF6H,UF6T,UF7F,UF7G,UF7H,UF7T,UG6F,UG6G,UG6H,"
                 "UG6T,UG7F,UG7G,UG7H,UG7T,UH6F,UH6G,UH6H,UH6T,UH7F,UH7G,UH7H,UH7T,UI6F,UI6G,UI6H,UI6T,UI7F,UI7G,UI7H,UI7T"},
    {.continent = "EU",
     .country = "Russia - Kalmykiya",
     .cq_zone = 16,
     .itu_zone = 29,
     .lat = 46.57,
     .lon = 45.32,
     .prefixes = "R6I,R7I,RA6I,RA7I,RC6I,RC7I,RD6I,RD7I,RE6I,RE7I,RF6I,RF7I,RG6I,RG7I,RJ6I,RJ7I,RK6I,RK7I,RL6I,RL7I,RM6I,RM7I,RN6I,RN7I,RO6I,"
                 "RO7I,RQ6I,RQ7I,RT6I,RT7I,RU6I,RU7I,RV6I,RV7I,RW6I,RW7I,RX6I,RX7I,RY6I,RY7I,RZ6I,RZ7I,U6I,U7I,UA6I,UA7I,UB6I,UB7I,UC6I,UC7I,"
                 "UD6I,UD7I,UE6I,UE7I,UF6I,UF7I,UG6I,UG7I,UH6I,UH7I,UI6I,UI7I"},
    {.continent = "EU",
     .country = "Russia - Severnaya Osetiya",
     .cq_zone = 16,
     .itu_zone = 29,
     .lat = 43.18,
     .lon = 44.23,
     .prefixes = "R6J,R7J,RA6J,RA7J,RC6J,RC7J,RD6J,RD7J,RE6J,RE7J,RF6J,RF7J,RG6J,RG7J,RJ6J,RJ7J,RK6J,RK7J,RL6J,RL7J,RM6J,RM7J,RN6J,RN7J,RO6J,"
                 "RO7J,RQ6J,RQ7J,RT6J,RT7J,RU6J,RU7J,RV6J,RV7J,RW6J,RW7J,RX6J,RX7J,RY6J,RY7J,RZ6J,RZ7J,U6J,U7J,UA6J,UA7J,UB6J,UB7J,UC6J,UC7J,"
                 "UD6J,UD7J,UE6J,UE7J,UF6J,UF7J,UG6J,UG7J,UH6J,UH7J,UI6J,UI7J"},
    {.continent = "EU",
     .country = "Russia - Rostovskaya",
     .cq_zone = 16,
     .itu_zone = 29,
     .lat = 47.87,
     .lon = 41.18,
     .prefixes = "R6L,R6M,R6N,R7L,R7M,R7N,RA6L,RA6M,RA6N,RA7L,RA7M,RA7N,RC6L,RC6M,RC6N,RC7L,RC7M,RC7N,RD6L,RD6M,RD6N,RD7L,RD7M,RD7N,RE6L,RE6M,RE6N,RE7L,"
                 "RE7M,RE7N,RF6L,RF6M,RF6N,RF7L,RF7M,RF7N,RG6L,RG6M,RG6N,RG7L,RG7M,RG7N,RJ6L,RJ6M,RJ6N,RJ7L,RJ7M,RJ7N,RK6L,RK6M,RK6N,RK7L,RK7M,RK7N,"
                 "RL6L,RL6M,RL6N,RL7L,RL7M,RL7N,RM6L,RM6M,RM6N,RM7L,RM7M,RM7N,RN6L,RN6M,RN6N,RN7L,RN7M,RN7N,RO6L,RO6M,RO6N,RO7L,RO7M,RO7N,RQ6L,RQ6M,"
                 "RQ6N,RQ7L,RQ7M,RQ7N,RT6L,RT6M,RT6N,RT7L,RT7M,RT7N,RU6L,RU6M,RU6N,RU7L,RU7M,RU7N,RV6L,RV6M,RV6N,RV7L,RV7M,RV7N,RW6L,RW6M,RW6N,RW7L,"
                 "RW7M,RW7N,RX6L,RX6M,RX6N,RX7L,RX7M,RX7N,RY6L,RY6M,RY6N,RY7L,RY7M,RY7N,RZ6L,RZ6M,RZ6N,RZ7L,RZ7M,RZ7N,U6L,U6M,U6N,U7L,U7M,U7N,UA6L,UA6M,"
                 "UA6N,UA7L,UA7M,UA7N,UB6L,UB6M,UB6N,UB7L,UB7M,UB7N,UC6L,UC6M,UC6N,UC7L,UC7M,UC7N,UD6L,UD6M,UD6N,UD7L,UD7M,UD7N,UE6L,UE6M,UE6N,UE7L,"
                 "UE7M,UE7N,UF6L,UF6M,UF6N,UF7L,UF7M,UF7N,UG6L,UG6M,UG6N,UG7L,UG7M,UG7N,UH6L,UH6M,UH6N,UH7L,UH7M,UH7N,UI6L,UI6M,UI6N,UI7L,UI7M,UI7N"},
    {.continent = "EU",
     .country = "Russia - Chechnya",
     .cq_zone = 16,
     .itu_zone = 29,
     .lat = 43.40,
     .lon = 45.72,
     .prefixes = "R6P,R7P,RA6P,RA7P,RC6P,RC7P,RD6P,RD7P,RE6P,RE7P,RF6P,RF7P,RG6P,RG7P,RJ6P,RJ7P,RK6P,RK7P,RL6P,RL7P,RM6P,RM7P,RN6P,RN7P,RO6P,"
                 "RO7P,RQ6P,RQ7P,RT6P,RT7P,RU6P,RU7P,RV6P,RV7P,RW6P,RW7P,RX6P,RX7P,RY6P,RY7P,RZ6P,RZ7P,U6P,U7P,UA6P,UA7P,UB6P,UB7P,UC6P,UC7P,"
                 "UD6P,UD7P,UE6P,UE7P,UF6P,UF7P,UG6P,UG7P,UH6P,UH7P,UI6P,UI7P"},
    {.continent = "EU",
     .country = "Russia - Ingushetiya",
     .cq_zone = 16,
     .itu_zone = 29,
     .lat = 43.20,
     .lon = 44.97,
     .prefixes = "R6Q,R7Q,RA6Q,RA7Q,RC6Q,RC7Q,RD6Q,RD7Q,RE6Q,RE7Q,RF6Q,RF7Q,RG6Q,RG7Q,RJ6Q,RJ7Q,RK6Q,RK7Q,RL6Q,RL7Q,RM6Q,RM7Q,RN6Q,RN7Q,RO6Q,"
                 "RO7Q,RQ6Q,RQ7Q,RT6Q,RT7Q,RU6Q,RU7Q,RV6Q,RV7Q,RW6Q,RW7Q,RX6Q,RX7Q,RY6Q,RY7Q,RZ6Q,RZ7Q,U6Q,U7Q,UA6Q,UA7Q,UB6Q,UB7Q,UC6Q,UC7Q,"
                 "UD6Q,UD7Q,UE6Q,UE7Q,UF6Q,UF7Q,UG6Q,UG7Q,UH6Q,UH7Q,UI6Q,UI7Q"},
    {.continent = "EU",
     .country = "Russia - Astrakhanskaya",
     .cq_zone = 16,
     .itu_zone = 29,
     .lat = 47.23,
     .lon = 47.23,
     .prefixes = "R6U,R6V,R7U,R7V,RA6U,RA6V,RA7U,RA7V,RC6U,RC6V,RC7U,RC7V,RD6U,RD6V,RD7U,RD7V,RE6U,RE6V,RE7U,RE7V,RF6U,RF6V,RF7U,RF7V,RG6U,RG6V,"
                 "RG7U,RG7V,RJ6U,RJ6V,RJ7U,RJ7V,RK6U,RK6V,RK7U,RK7V,RL6U,RL6V,RL7U,RL7V,RM6U,RM6V,RM7U,RM7V,RN6U,RN6V,RN7U,RN7V,RO6U,RO6V,RO7U,"
                 "RO7V,RQ6U,RQ6V,RQ7U,RQ7V,RT6U,RT6V,RT7U,RT7V,RU6U,RU6V,RU7U,RU7V,RV6U,RV6V,RV7U,RV7V,RW6U,RW6V,RW7U,RW7V,RX6U,RX6V,RX7U,RX7V,"
                 "RY6U,RY6V,RY7U,RY7V,RZ6U,RZ6V,RZ7U,RZ7V,U6U,U6V,U7U,U7V,UA6U,UA6V,UA7U,UA7V,UB6U,UB6V,UB7U,UB7V,UC6U,UC6V,UC7U,UC7V,UD6U,UD6V,"
                 "UD7U,UD7V,UE6U,UE6V,UE7U,UE7V,UF6U,UF7U,UF7V,UG6U,UG6V,UG7U,UG7V,UH6U,UH6V,UH7U,UH7V,UI6U,UI6V,UI7U,UI7V"},
    {.continent = "EU",
     .country = "Russia - Dagestan",
     .cq_zone = 16,
     .itu_zone = 29,
     .lat = 43.10,
     .lon = 46.88,
     .prefixes = "R6W,R7W,RA6W,RA7W,RC6W,RC7W,RD6W,RD7W,RE6W,RE7W,RF6W,RF7W,RG6W,RG7W,RJ6W,RJ7W,RK6W,RK7W,RL6W,RL7W,RM6W,RM7W,RN6W,RN7W,RO6W,"
                 "RO7W,RQ6W,RQ7W,RT6W,RT7W,RU6W,RU7W,RV6W,RV7W,RW6W,RW7W,RX6W,RX7W,RY6W,RY7W,RZ6W,RZ7W,U6W,U7W,UA6W,UA7W,UB6W,UB7W,UC6W,UC7W,"
                 "UD6W,UD7W,UE6W,UE7W,UF6W,UF7W,UG6W,UG7W,UH6W,UH7W,UI6W,UI7W"},
    {.continent = "EU",
     .country = "Russia - Kabardino-Balkaria",
     .cq_zone = 16,
     .itu_zone = 29,
     .lat = 43.58,
     .lon = 43.4,
     .prefixes = "R6X,R7X,RA6X,RA7X,RC6X,RC7X,RD6X,RD7X,RE6X,RE7X,RF6X,RF7X,RG6X,RG7X,RJ6X,RJ7X,RK6X,RK7X,RL6X,RL7X,RM6X,RM7X,RN6X,RN7X,RO6X,"
                 "RO7X,RQ6X,RQ7X,RT6X,RT7X,RU6X,RU7X,RV6X,RV7X,RW6X,RW7X,RX6X,RX7X,RY6X,RY7X,RZ6X,RZ7X,U6X,U7X,UA6X,UA7X,UB6X,UB7X,UC6X,UC7X,"
                 "UD6X,UD7X,UE6X,UE7X,UF6X,UF7X,UG6X,UG7X,UH6X,UH7X,UI6X,UI7X"},
    {.continent = "EU",
     .country = "Russia - Adygeya",
     .cq_zone = 16,
     .itu_zone = 29,
     .lat = 44.65,
     .lon = 40,
     .prefixes = "R6Y,R7Y,RA6Y,RA7Y,RC6Y,RC7Y,RD6Y,RD7Y,RE6Y,RE7Y,RF6Y,RF7Y,RG6Y,RG7Y,RJ6Y,RJ7Y,RK6Y,RK7Y,RL6Y,RL7Y,RM6Y,RM7Y,RN6Y,RN7Y,RO6Y,"
                 "RO7Y,RQ6Y,RQ7Y,RT6Y,RT7Y,RU6Y,RU7Y,RV6Y,RV7Y,RW6Y,RW7Y,RX6Y,RX7Y,RY6Y,RY7Y,RZ6Y,RZ7Y,U6Y,U7Y,UA6Y,UA7Y,UB6Y,UB7Y,UC6Y,UC7Y,"
                 "UD6Y,UD7Y,UE6Y,UE7Y,UF6Y,UF7Y,UG6Y,UG7Y,UH6Y,UH7Y,UI6Y,UI7Y"},
    {.continent = "EU",
     .country = "Russia - Republic of Krim",
     .cq_zone = 16,
     .itu_zone = 29,
     .lat = 44.93,
     .lon = 34.1,
     .prefixes = "R6K,R7K,RA6K,RA7K,RC6K,RC7K,RD6K,RD7K,RE6K,RE7K,RF6K,RF7K,RG6K,RG7K,RJ6K,RJ7K,RK6K,RK7K,RL6K,RL7K,RM6K,RM7K,RN6K,RN7K,RO6K,"
                 "RO7K,RQ6K,RQ7K,RT6K,RT7K,RU6K,RU7K,RV6K,RV7K,RW6K,RW7K,RX6K,RX7K,RY6K,RY7K,RZ6K,RZ7K,U6K,U7K,UA6K,UA7K,UB6K,UB7K,UC6K,UC7K,"
                 "UD6K,UD7K,UE6K,UE7K,UF6K,UF7K,UG6K,UG7K,UH6K,UH7K,UI6K,UI7K"},
    {.continent = "EU",
     .country = "Russia - Sevastopol",
     .cq_zone = 16,
     .itu_zone = 29,
     .lat = 50.00,
     .lon = 30,
     .prefixes = "R6R,R7R,RA6R,RA7R,RC6R,RC7R,RD6R,RD7R,RE6R,RE7R,RF6R,RF7R,RG6R,RG7R,RJ6R,RJ7R,RK6R,RK7R,RL6R,RL7R,RM6R,RM7R,RN6R,RN7R,RO6R,"
                 "RO7R,RQ6R,RQ7R,RT6R,RT7R,RU6R,RU7R,RV6R,RV7R,RW6R,RW7R,RX6R,RX7R,RY6R,RY7R,RZ6R,RZ7R,U6R,U7R,UA6R,UA7R,UB6R,UB7R,UC6R,UC7R,"
                 "UD6R,UD7R,UE6R,UE7R,UF6R,UF7R,UG6R,UG7R,UH6R,UH7R,UI6R,UI7R"},
    {.continent = "EU",
     .country = "Russia - Permskaya",
     .cq_zone = 17,
     .itu_zone = 30,
     .lat = 58.00,
     .lon = 56.32,
     .prefixes = "R8F,R8G,R9F,R9G,RA8F,RA8G,RA9F,RA9G,RC8F,RC8G,RC9F,RC9G,RD8F,RD8G,RD9F,RD9G,RE8F,RE8G,RE9F,RE9G,RF8F,RF8G,RF9F,RF9G,RG8F,RG8G,"
                 "RG9F,RG9G,RJ8F,RJ8G,RJ9F,RJ9G,RK8F,RK8G,RK9F,RK9G,RL8F,RL8G,RL9F,RL9G,RM8F,RM8G,RM9F,RM9G,RN8F,RN8G,RN9F,RN9G,RO8F,RO8G,RO9F,"
                 "RO9G,RQ8F,RQ8G,RQ9F,RQ9G,RT8F,RT8G,RT9F,RT9G,RU8F,RU8G,RU9F,RU9G,RV8F,RV8G,RV9F,RV9G,RW8F,RW8G,RW9F,RW9G,RX8F,RX8G,RX9F,RX9G,"
                 "RY8F,RY8G,RY9F,RY9G,RZ8F,RZ8G,RZ9F,RZ9G,U8F,U8G,U9F,U9G,UA8F,UA8G,UA9F,UA9G,UB8F,UB8G,UB9F,UB9G,UC8F,UC8G,UC9F,UC9G,UD8F,UD8G,"
                 "UD9F,UD9G,UE8F,UE8G,UE9F,UE9G,UF8F,UF8G,UF9F,UF9G,UG8F,UG8G,UG9F,UG9G,UH8F,UH8G,UH9F,UH9G,UI8F,UI8G,UI9F,UI9G"},
    {.continent = "EU",
     .country = "Russia - Komi",
     .cq_zone = 17,
     .itu_zone = 20,
     .lat = 64.28,
     .lon = 54.47,
     .prefixes = "R1I,R8X,R9X,RA1I,RA8X,RA9X,RC1I,RC8X,RC9X,RD1I,RD8X,RD9X,RE1I,RE8X,RE9X,RF1I,RF8X,RF9X,RG1I,RG8X,RG9X,RI8X,RI9X,RJ1I,RJ8X,"
                 "RJ9X,RK1I,RK8X,RK9X,RL1I,RL8X,RL9X,RM1I,RM8X,RM9X,RN1I,RN8X,RN9X,RO1I,RO8X,RO9X,RQ1I,RQ8X,RQ9X,RT1I,RT8X,RT9X,RU1I,RU8X,RU9X,"
                 "RV1I,RV8X,RV9X,RW1I,RW8X,RW9X,RX1I,RX8X,RX9X,RY1I,RY8X,RY9X,RZ1I,RZ8X,RZ9X,U1I,U8X,U9X,UA1I,UA8X,UA9X,UB1I,UB8X,UB9X,UC1I,"
                 "UC8X,UC9X,UD1I,UD8X,UD9X,UE1I,UE8X,UE9X,UF1I,UF8X,UF9X,UG1I,UG8X,UG9X,UH1I,UH8X,UH9X,UI1I,UI8X,UI9X"},
    {.continent = "EU",
     .country = "Russia - Kaliningrad",
     .cq_zone = 15,
     .itu_zone = 29,
     .lat = 54.72,
     .lon = 20.52,
     .prefixes = "R2F,R2K,RA2,RC2F,RC2K,RD2F,RD2K,RE2F,RE2K,RF2F,RF2K,RG2F,RG2K,RJ2F,RJ2K,RK2F,RK2K,RL2F,RL2K,RM2F,RM2K,RN2F,RN2K,RO2F,RO2K,RQ2F,RQ2K,"
                 "RT2F,RT2K,RU2F,RU2K,RV2F,RV2K,RW2F,RW2K,RX2F,RX2K,RY2F,RY2K,RZ2F,RZ2K,U2F,U2K,UA2,UB2,UC2,UD2,UE2,UF2,UG2,UH2,UI2,R2MWO"},
    {.continent = "AS",
     .country = "Russia - Chelyabinskaya",
     .cq_zone = 17,
     .itu_zone = 30,
     .lat = 54.75,
     .lon = 60.7,
     .prefixes = "R8A,R8B,R9A,R9B,RA8A,RA8B,RA9A,RA9B,RC8A,RC8B,RC9A,RC9B,RD8A,RD8B,RD9A,RD9B,RE8A,RE8B,RE9A,RE9B,RF8A,RF8B,RF9A,RF9B,RG8A,RG8B,"
                 "RG9A,RG9B,RJ8A,RJ8B,RJ9A,RJ9B,RK8A,RK8B,RK9A,RK9B,RL8A,RL8B,RL9A,RL9B,RM8A,RM8B,RM9A,RM9B,RN8A,RN8B,RN9A,RN9B,RO8A,RO8B,RO9A,"
                 "RO9B,RQ8A,RQ8B,RQ9A,RQ9B,RT8A,RT8B,RT9A,RT9B,RU8A,RU8B,RU9A,RU9B,RV8A,RV8B,RV9A,RV9B,RW8A,RW8B,RW9A,RW9B,RX8A,RX8B,RX9A,RX9B,"
                 "RY8A,RY8B,RY9A,RY9B,RZ8A,RZ8B,RZ9A,RZ9B,U8A,U8B,U9A,U9B,UA8A,UA8B,UA9A,UA9B,UB8A,UB8B,UB9A,UB9B,UC8A,UC8B,UC9A,UC9B,UD8A,UD8B,"
                 "UD9A,UD9B,UE8A,UE8B,UE9A,UE9B,UF8A,UF8B,UF9A,UF9B,UG8A,UG8B,UG9A,UG9B,UH8A,UH8B,UH9A,UH9B,UI8A,UI8B,UI9A,UI9B"},
    {.continent = "AS",
     .country = "Russia - Sverdlovskaya",
     .cq_zone = 17,
     .itu_zone = 30,
     .lat = 58.70,
     .lon = 61.33,
     .prefixes = "R8C,R8D,R8E,R9C,R9D,R9E,RA8C,RA8D,RA8E,RA9C,RA9D,RA9E,RC8C,RC8D,RC8E,RC9C,RC9D,RC9E,RD8C,RD8D,RD8E,RD9C,RD9D,RD9E,RE8C,RE8D,RE8E,RE9C,"
                 "RE9D,RE9E,RF8C,RF8D,RF8E,RF9C,RF9D,RF9E,RG8C,RG8D,RG8E,RG9C,RG9D,RG9E,RJ8C,RJ8D,RJ8E,RJ9C,RJ9D,RJ9E,RK8C,RK8D,RK8E,RK9C,RK9D,RK9E,"
                 "RL8C,RL8D,RL8E,RL9C,RL9D,RL9E,RM8C,RM8D,RM8E,RM9C,RM9D,RM9E,RN8C,RN8D,RN8E,RN9C,RN9D,RN9E,RO8C,RO8D,RO8E,RO9C,RO9D,RO9E,RQ8C,RQ8D,"
                 "RQ8E,RQ9C,RQ9D,RQ9E,RT8C,RT8D,RT8E,RT9C,RT9D,RT9E,RU8C,RU8D,RU8E,RU9C,RU9D,RU9E,RV8C,RV8D,RV8E,RV9C,RV9D,RV9E,RW8C,RW8D,RW8E,RW9C,"
                 "RW9D,RW9E,RX8C,RX8D,RX8E,RX9C,RX9D,RX9E,RY8C,RY8D,RY8E,RY9C,RY9D,RY9E,RZ8C,RZ8D,RZ8E,RZ9C,RZ9D,RZ9E,U8C,U8D,U8E,U9C,U9D,U9E,UA8C,UA8D,"
                 "UA8E,UA9C,UA9D,UA9E,UB8C,UB8D,UB8E,UB9C,UB9D,UB9E,UC8C,UC8D,UC8E,UC9C,UC9D,UC9E,UD8C,UD8D,UD8E,UD9C,UD9D,UD9E,UE8C,UE8D,UE8E,UE9C,"
                 "UE9D,UE9E,UF8C,UF8D,UF8E,UF9C,UF9D,UF9E,UG8C,UG8D,UG8E,UG9C,UG9D,UG9E,UH8C,UH8D,UH8E,UH9C,UH9D,UH9E,UI8C,UI8D,UI8E,UI9C,UI9D,UI9E"},
    {.continent = "AS",
     .country = "Russia - Tomskaya",
     .cq_zone = 18,
     .itu_zone = 31,
     .lat = 58.75,
     .lon = 82.13,
     .prefixes = "R8H,R8I,R9H,R9I,RA8H,RA8I,RA9H,RA9I,RC8H,RC8I,RC9H,RC9I,RD8H,RD8I,RD9H,RD9I,RE8H,RE8I,RE9H,RE9I,RF8H,RF8I,RF9H,RF9I,RG8H,RG8I,"
                 "RG9H,RG9I,RJ8H,RJ8I,RJ9H,RJ9I,RK8H,RK8I,RK9H,RK9I,RL8H,RL8I,RL9H,RL9I,RM8H,RM8I,RM9H,RM9I,RN8H,RN8I,RN9H,RN9I,RO8H,RO8I,RO9H,"
                 "RO9I,RQ8H,RQ8I,RQ9H,RQ9I,RT8H,RT8I,RT9H,RT9I,RU8H,RU8I,RU9H,RU9I,RV8H,RV8I,RV9H,RV9I,RW8H,RW8I,RW9H,RW9I,RX8H,RX8I,RX9H,RX9I,"
                 "RY8H,RY8I,RY9H,RY9I,RZ8H,RZ8I,RZ9H,RZ9I,U8H,U8I,U9H,U9I,UA8H,UA8I,UA9H,UA9I,UB8H,UB8I,UB9H,UB9I,UC8H,UC8I,UC9H,UC9I,UD8H,UD8I,"
                 "UD9H,UD9I,UE8H,UE8I,UE9H,UE9I,UF8H,UF8I,UF9H,UF9I,UG8H,UG8I,UG9H,UG9I,UH8H,UH8I,UH9H,UH9I,UI8H,UI8I,UI9H,UI9I"},
    {.continent = "AS",
     .country = "Russia - Khanty-Mansyisky",
     .cq_zone = 17,
     .itu_zone = 20,
     .lat = 62.25,
     .lon = 70.17,
     .prefixes = "R8J,R9J,RA8J,RA9J,RC8J,RC9J,RD8J,RD9J,RE8J,RE9J,RF8J,RF9J,RG8J,RG9J,RJ8J,RJ9J,RK8J,RK9J,RL8J,RL9J,RM8J,RM9J,RN8J,RN9J,RO8J,"
                 "RO9J,RQ8J,RQ9J,RT8J,RT9J,RU8J,RU9J,RV8J,RV9J,RW8J,RW9J,RX8J,RX9J,RY8J,RY9J,RZ8J,RZ9J,U8J,U9J,UA8J,UA9J,UB8J,UB9J,UC8J,UC9J,"
                 "UD8J,UD9J,UE8J,UE9J,UF8J,UF9J,UG8J,UG9J,UH8J,UH9J,UI8J,UI9J"},
    {.continent = "AS",
     .country = "Russia - Yamalo-Nenetsky",
     .cq_zone = 17,
     .itu_zone = 20,
     .lat = 65.30,
     .lon = 74.03,
     .prefixes = "R8K,R9K,RA8K,RA9K,RC8K,RC9K,RD8K,RD9K,RE8K,RE9K,RF8K,RF9K,RG8K,RG9K,RI9K,RJ8K,RJ9K,RK8K,RK9K,RL8K,RL9K,RM8K,RM9K,RN8K,RN9K,"
                 "RO8K,RO9K,RQ8K,RQ9K,RT8K,RT9K,RU8K,RU9K,RV8K,RV9K,RW8K,RW9K,RX8K,RX9K,RY8K,RY9K,RZ8K,RZ9K,U8K,U9K,UA8K,UA9K,UB8K,UB9K,UC8K,"
                 "UC9K,UD8K,UD9K,UE8K,UE9K,UF8K,UF9K,UG8K,UG9K,UH8K,UH9K,UI8K,UI9K"},
    {.continent = "AS",
     .country = "Russia - Tyumenskaya",
     .cq_zone = 17,
     .itu_zone = 30,
     .lat = 57.83,
     .lon = 69,
     .prefixes = "R8L,R9L,RA8L,RA9L,RC8L,RC9L,RD8L,RD9L,RE8L,RE9L,RF8L,RF9L,RG8L,RG9L,RJ8L,RJ9L,RK8L,RK9L,RL8L,RL9L,RM8L,RM9L,RN8L,RN9L,RO8L,"
                 "RO9L,RQ8L,RQ9L,RT8L,RT9L,RU8L,RU9L,RV8L,RV9L,RW8L,RW9L,RX8L,RX9L,RY8L,RY9L,RZ8L,RZ9L,U8L,U9L,UA8L,UA9L,UB8L,UB9L,UC8L,UC9L,"
                 "UD8L,UD9L,UE8L,UE9L,UF8L,UF9L,UG8L,UG9L,UH8L,UH9L,UI8L,UI9L"},
    {.continent = "AS",
     .country = "Russia - Omskaya",
     .cq_zone = 17,
     .itu_zone = 30,
     .lat = 56.22,
     .lon = 73.27,
     .prefixes = "R8M,R8N,R9M,R9N,RA8M,RA8N,RA9M,RA9N,RC8M,RC8N,RC9M,RC9N,RD8M,RD8N,RD9M,RD9N,RE8M,RE8N,RE9M,RE9N,RF8M,RF8N,RF9M,RF9N,RG8M,RG8N,"
                 "RG9M,RG9N,RJ8M,RJ8N,RJ9M,RJ9N,RK8M,RK8N,RK9M,RK9N,RL8M,RL8N,RL9M,RL9N,RM8M,RM8N,RM9M,RM9N,RN8M,RN8N,RN9M,RN9N,RO8M,RO8N,RO9M,"
                 "RO9N,RQ8M,RQ8N,RQ9M,RQ9N,RT8M,RT8N,RT9M,RT9N,RU8M,RU8N,RU9M,RU9N,RV8M,RV8N,RV9M,RV9N,RW8M,RW8N,RW9M,RW9N,RX8M,RX8N,RX9M,RX9N,"
                 "RY8M,RY8N,RY9M,RY9N,RZ8M,RZ8N,RZ9M,RZ9N,U8M,U8N,U9M,U9N,UA8M,UA8N,UA9M,UA9N,UB8M,UB8N,UB9M,UB9N,UC8M,UC8N,UC9M,UC9N,UD8M,UD8N,"
                 "UD9M,UD9N,UE8M,UE8N,UE9M,UE9N,UF8M,UF8N,UF9M,UF9N,UG8M,UG8N,UG9M,UG9N,UH8M,UH8N,UH9M,UH9N,UI8M,UI8N,UI9M,UI9N"},
    {.continent = "AS",
     .country = "Russia - Novosibirskaya",
     .cq_zone = 18,
     .itu_zone = 31,
     .lat = 55.45,
     .lon = 79.55,
     .prefixes = "R8O,R8P,R9O,R9P,RA8O,RA8P,RA9O,RA9P,RC8O,RC8P,RC9O,RC9P,RD8O,RD8P,RD9O,RD9P,RE8O,RE8P,RE9O,RE9P,RF8O,RF8P,RF9O,RF9P,RG8O,RG8P,"
                 "RG9O,RG9P,RJ8O,RJ8P,RJ9O,RJ9P,RK8O,RK8P,RK9O,RK9P,RL8O,RL8P,RL9O,RL9P,RM8O,RM8P,RM9O,RM9P,RN8O,RN8P,RN9O,RN9P,RO8O,RO8P,RO9O,"
                 "RO9P,RQ8O,RQ8P,RQ9O,RQ9P,RT8O,RT8P,RT9O,RT9P,RU8O,RU8P,RU9O,RU9P,RV8O,RV8P,RV9O,RV9P,RW8O,RW8P,RW9O,RW9P,RX8O,RX8P,RX9O,RX9P,"
                 "RY8O,RY8P,RY9O,RY9P,RZ8O,RZ8P,RZ9O,RZ9P,U8O,U8P,U9O,U9P,UA8O,UA8P,UA9O,UA9P,UB8O,UB8P,UB9O,UB9P,UC8O,UC8P,UC9O,UC9P,UD8O,UD8P,"
                 "UD9O,UD9P,UE8O,UE8P,UE9O,UE9P,UF8O,UF8P,UF9O,UF9P,UG8O,UG8P,UG9O,UG9P,UH8O,UH8P,UH9O,UH9P,UI8O,UI8P,UI9O,UI9P"},
    {.continent = "AS",
     .country = "Russia - Kurganskaya",
     .cq_zone = 17,
     .itu_zone = 30,
     .lat = 55.57,
     .lon = 64.75,
     .prefixes = "R8Q,R8R,R9Q,R9R,RA8Q,RA8R,RA9Q,RA9R,RC8Q,RC8R,RC9Q,RC9R,RD8Q,RD8R,RD9Q,RD9R,RE8Q,RE8R,RE9Q,RE9R,RF8Q,RF8R,RF9Q,RF9R,RG8Q,RG8R,"
                 "RG9Q,RG9R,RJ8Q,RJ8R,RJ9Q,RJ9R,RK8Q,RK8R,RK9Q,RK9R,RL8Q,RL8R,RL9Q,RL9R,RM8Q,RM8R,RM9Q,RM9R,RN8Q,RN8R,RN9Q,RN9R,RO8Q,RO8R,RO9Q,"
                 "RO9R,RQ8Q,RQ8R,RQ9Q,RQ9R,RT8Q,RT8R,RT9Q,RT9R,RU8Q,RU8R,RU9Q,RU9R,RV8Q,RV8R,RV9Q,RV9R,RW8Q,RW8R,RW9Q,RW9R,RX8Q,RX8R,RX9Q,RX9R,"
                 "RY8Q,RY8R,RY9Q,RY9R,RZ8Q,RZ8R,RZ9Q,RZ9R,U8Q,U8R,U9Q,U9R,UA8Q,UA8R,UA9Q,UA9R,UB8Q,UB8R,UB9Q,UB9R,UC8Q,UC8R,UC9Q,UC9R,UD8Q,UD8R,"
                 "UD9Q,UD9R,UE8Q,UE8R,UE9Q,UE9R,UF8Q,UF8R,UF9Q,UF9R,UG8Q,UG8R,UG9Q,UG9R,UH8Q,UH8R,UH9Q,UH9R,UI8Q,UI8R,UI9Q,UI9R"},
    {.continent = "AS",
     .country = "Russia - Orenburgskaya",
     .cq_zone = 16,
     .itu_zone = 30,
     .lat = 52.13,
     .lon = 55.6,
     .prefixes = "R8S,R8T,R9S,R9T,RA8S,RA8T,RA9S,RA9T,RC8S,RC8T,RC9S,RC9T,RD8S,RD8T,RD9S,RD9T,RE8S,RE8T,RE9S,RE9T,RF8S,RF8T,RF9S,RF9T,RG8S,RG8T,"
                 "RG9S,RG9T,RJ8S,RJ8T,RJ9S,RJ9T,RK8S,RK8T,RK9S,RK9T,RL8S,RL8T,RL9S,RL9T,RM8S,RM8T,RM9S,RM9T,RN8S,RN8T,RN9S,RN9T,RO8S,RO8T,RO9S,"
                 "RO9T,RQ8S,RQ8T,RQ9S,RQ9T,RT8S,RT8T,RT9S,RT9T,RU8S,RU8T,RU9S,RU9T,RV8S,RV8T,RV9S,RV9T,RW8S,RW8T,RW9S,RW9T,RX8S,RX8T,RX9S,RX9T,"
                 "RY8S,RY8T,RY9S,RY9T,RZ8S,RZ8T,RZ9S,RZ9T,U8S,U8T,U9S,U9T,UA8S,UA8T,UA9S,UA9T,UB8S,UB8T,UB9S,UB9T,UC8S,UC8T,UC9S,UC9T,UD8S,UD8T,"
                 "UD9S,UD9T,UE8S,UE8T,UE9S,UE9T,UF8S,UF8T,UF9S,UF9T,UG8S,UG8T,UG9S,UG9T,UH8S,UH8T,UH9S,UH9T,UI8S,UI8T,UI9S,UI9T"},
    {.continent = "AS",
     .country = "Russia - Kemerovskaya",
     .cq_zone = 18,
     .itu_zone = 31,
     .lat = 54.93,
     .lon = 87.23,
     .prefixes = "R8U,R8V,R9U,R9V,RA8U,RA8V,RA9U,RA9V,RC8U,RC8V,RC9U,RC9V,RD8U,RD8V,RD9U,RD9V,RE8U,RE8V,RE9U,RE9V,RF8U,RF8V,RF9U,RF9V,RG8U,RG8V,"
                 "RG9U,RG9V,RJ8U,RJ8V,RJ9U,RJ9V,RK8U,RK8V,RK9U,RK9V,RL8U,RL8V,RL9U,RL9V,RM8U,RM8V,RM9U,RM9V,RN8U,RN8V,RN9U,RN9V,RO8U,RO8V,RO9U,"
                 "RO9V,RQ8U,RQ8V,RQ9U,RQ9V,RT8U,RT8V,RT9U,RT9V,RU8U,RU8V,RU9U,RU9V,RV8U,RV8V,RV9U,RV9V,RW8U,RW8V,RW9U,RW9V,RX8U,RX8V,RX9U,RX9V,"
                 "RY8U,RY8V,RY9U,RY9V,RZ8U,RZ8V,RZ9U,RZ9V,U8U,U8V,U9U,U9V,UA8U,UA8V,UA9U,UA9V,UB8U,UB8V,UB9U,UB9V,UC8U,UC8V,UC9U,UC9V,UD8U,UD8V,"
                 "UD9U,UD9V,UE8U,UE8V,UE9U,UE9V,UF8U,UF8V,UF9U,UF9V,UG8U,UG8V,UG9U,UG9V,UH8U,UH8V,UH9U,UH9V,UI8U,UI8V,UI9U,UI9V"},
    {.continent = "AS",
     .country = "Russia - Bashkortostan",
     .cq_zone = 16,
     .itu_zone = 30,
     .lat = 54.47,
     .lon = 56.27,
     .prefixes = "R8W,R9W,RA8W,RA9W,RC8W,RC9W,RD8W,RD9W,RE8W,RE9W,RF8W,RF9W,RG8W,RG9W,RJ8W,RJ9W,RK8W,RK9W,RL8W,RL9W,RM8W,RM9W,RN8W,RN9W,RO8W,"
                 "RO9W,RQ8W,RQ9W,RT8W,RT9W,RU8W,RU9W,RV8W,RV9W,RW8W,RW9W,RX8W,RX9W,RY8W,RY9W,RZ8W,RZ9W,U8W,U9W,UA8W,UA9W,UB8W,UB9W,UC8W,UC9W,"
                 "UD8W,UD9W,UE8W,UE9W,UF8W,UF9W,UG8W,UG9W,UH8W,UH9W,UI8W,UI9W"},
    {.continent = "AS",
     .country = "Russia - Altaysky",
     .cq_zone = 18,
     .itu_zone = 31,
     .lat = 52.77,
     .lon = 82.62,
     .prefixes = "R8Y,R9Y,RA8Y,RA9Y,RC8Y,RC9Y,RD8Y,RD9Y,RE8Y,RE9Y,RF8Y,RF9Y,RG8Y,RG9Y,RJ8Y,RJ9Y,RK8Y,RK9Y,RL8Y,RL9Y,RM8Y,RM9Y,RN8Y,RN9Y,RO8Y,"
                 "RO9Y,RQ8Y,RQ9Y,RT8Y,RT9Y,RU8Y,RU9Y,RV8Y,RV9Y,RW8Y,RW9Y,RX8Y,RX9Y,RY8Y,RY9Y,RZ8Y,RZ9Y,U8Y,U9Y,UA8Y,UA9Y,UB8Y,UB9Y,UC8Y,UC9Y,"
                 "UD8Y,UD9Y,UE8Y,UE9Y,UF8Y,UF9Y,UG8Y,UG9Y,UH8Y,UH9Y,UI8Y,UI9Y"},
    {.continent = "AS",
     .country = "Russia - Gorno Altayskaya",
     .cq_zone = 18,
     .itu_zone = 31,
     .lat = 50.92,
     .lon = 86.92,
     .prefixes = "R8Z,R9Z,RA8Z,RA9Z,RC8Z,RC9Z,RD8Z,RD9Z,RE8Z,RE9Z,RF8Z,RF9Z,RG8Z,RG9Z,RJ8Z,RJ9Z,RK8Z,RK9Z,RL8Z,RL9Z,RM8Z,RM9Z,RN8Z,RN9Z,RO8Z,"
                 "RO9Z,RQ8Z,RQ9Z,RT8Z,RT9Z,RU8Z,RU9Z,RV8Z,RV9Z,RW8Z,RW9Z,RX8Z,RX9Z,RY8Z,RY9Z,RZ8Z,RZ9Z,U8Z,U9Z,UA8Z,UA9Z,UB8Z,UB9Z,UC8Z,UC9Z,"
                 "UD8Z,UD9Z,UE8Z,UE9Z,UF8Z,UF9Z,UG8Z,UG9Z,UH8Z,UH9Z,UI8Z,UI9Z"},
    {.continent = "AS",
     .country = "Russia - Krasnoyarsky",
     .cq_zone = 18,
     .itu_zone = 32,
     .lat = 59.88,
     .lon = 91.67,
     .prefixes = "R0A,R0B,R0H,RA0A,RA0B,RA0H,RC0A,RC0B,RC0H,RD0A,RD0B,RD0H,RE0A,RE0B,RE0H,RF0A,RF0B,RF0H,RG0A,RG0B,RG0H,RI0A,RI0B,RI0H,RJ0A,"
                 "RJ0B,RJ0H,RK0A,RK0B,RK0H,RL0A,RL0B,RL0H,RM0A,RM0B,RM0H,RN0A,RN0B,RN0H,RO0A,RO0B,RO0H,RQ0A,RQ0B,RQ0H,RT0A,RT0B,RT0H,RU0A,RU0B,"
                 "RU0H,RV0A,RV0B,RV0H,RW0A,RW0B,RW0H,RX0A,RX0B,RX0H,RY0A,RY0B,RY0H,RZ0A,RZ0B,RZ0H,U0A,U0B,U0H,UA0A,UA0B,UA0H,UB0A,UB0B,UB0H,"
                 "UC0A,UC0B,UC0H,UD0A,UD0B,UD0H,UE0A,UE0B,UE0H,UF0A,UF0B,UF0H,UG0A,UG0B,UG0H,UH0A,UH0B,UH0H,UI0A,UI0B,UI0H"},
    {.continent = "AS",
     .country = "Russia - Khabarovsky",
     .cq_zone = 19,
     .itu_zone = 34,
     .lat = 54.80,
     .lon = 136.83,
     .prefixes = "R0C,RA0C,RC0C,RD0C,RE0C,RF0C,RG0C,RI0C,RJ0C,RK0C,RL0C,RM0C,RN0C,RO0C,RQ0C,RT0C,RU0C,RV0C,RW0C,RX0C,RY0C,RZ0C,U0C,UA0C,UB0C,"
                 "UC0C,UD0C,UE0C,UF0C,UG0C,UH0C,UI0C"},
    {.continent = "AS",
     .country = "Russia - Yevreyskaya",
     .cq_zone = 19,
     .itu_zone = 33,
     .lat = 48.60,
     .lon = 132.2,
     .prefixes = "R0D,RA0D,RC0D,RD0D,RE0D,RF0D,RG0D,RJ0D,RK0D,RL0D,RM0D,RN0D,RO0D,RQ0D,RT0D,RU0D,RV0D,RW0D,RX0D,RY0D,RZ0D,U0D,UA0D,UB0D,UC0D,"
                 "UD0D,UE0D,UF0D,UG0D,UH0D,UI0D"},
    {.continent = "AS",
     .country = "Russia - Sakhalinskaya",
     .cq_zone = 19,
     .itu_zone = 34,
     .lat = 50.55,
     .lon = 142.6,
     .prefixes = "R0E,R0F,RA0E,RA0F,RC0E,RC0F,RD0E,RD0F,RE0E,RE0F,RF0E,RF0F,RG0E,RG0F,RI0F,RJ0E,RJ0F,RK0E,RK0F,RL0E,RL0F,RM0E,RM0F,RN0E,RN0F,"
                 "RO0E,RO0F,RQ0E,RQ0F,RT0E,RT0F,RU0E,RU0F,RV0E,RV0F,RW0E,RW0F,RX0E,RX0F,RY0E,RY0F,RZ0E,RZ0F,U0E,U0F,UA0E,UA0F,UB0E,UB0F,UC0E,"
                 "UC0F,UD0E,UD0F,UE0E,UE0F,UF0E,UF0F,UG0E,UG0F,UH0E,UH0F,UI0E,UI0F"},
    {.continent = "AS",
     .country = "Russia - Magadanskaya",
     .cq_zone = 19,
     .itu_zone = 24,
     .lat = 62.90,
     .lon = 153.7,
     .prefixes = "R0I,RA0I,RC0I,RD0I,RE0I,RF0I,RG0I,RI0I,RJ0I,RK0I,RL0I,RM0I,RN0I,RO0I,RQ0I,RT0I,RU0I,RV0I,RW0I,RX0I,RY0I,RZ0I,U0I,UA0I,UB0I,"
                 "UC0I,UD0I,UE0I,UF0I,UG0I,UH0I,UI0I"},
    {.continent = "AS",
     .country = "Russia - Amurskaya",
     .cq_zone = 19,
     .itu_zone = 33,
     .lat = 53.55,
     .lon = 127.83,
     .prefixes = "R0G,R0J,RA0G,RA0J,RC0G,RC0J,RD0G,RD0J,RE0G,RE0J,RF0G,RF0J,RG0G,RG0J,RJ0G,RJ0J,RK0G,RK0J,RL0G,RL0J,RM0G,RM0J,RN0G,RN0J,RO0G,"
                 "RO0J,RQ0G,RQ0J,RT0G,RT0J,RU0G,RU0J,RV0G,RV0J,RW0G,RW0J,RX0G,RX0J,RY0G,RY0J,RZ0G,RZ0J,U0G,U0J,UA0G,UA0J,UB0G,UB0J,UC0G,UC0J,"
                 "UD0G,UD0J,UE0G,UE0J,UF0G,UF0J,UG0G,UG0J,UH0G,UH0J,UI0G,UI0J"},
    {.continent = "AS",
     .country = "Russia - Chukotsky",
     .cq_zone = 19,
     .itu_zone = 25,
     .lat = 66.67,
     .lon = 171,
     .prefixes = "R0K,RA0K,RC0K,RD0K,RE0K,RF0K,RG0K,RI0K,RJ0K,RK0K,RL0K,RM0K,RN0K,RO0K,RQ0K,RT0K,RU0K,RV0K,RW0K,RX0K,RY0K,RZ0K,U0K,UA0K,UB0K,"
                 "UC0K,UD0K,UE0K,UF0K,UG0K,UH0K,UI0K"},
    {.continent = "AS",
     .country = "Russia - Primorsky",
     .cq_zone = 19,
     .itu_zone = 34,
     .lat = 45.33,
     .lon = 134.67,
     .prefixes = "R0L,R0M,R0N,RA0L,RA0M,RA0N,RC0L,RC0M,RC0N,RD0L,RD0M,RD0N,RE0L,RE0M,RE0N,RF0L,RF0M,RF0N,RG0L,RG0M,RG0N,RI0L,RJ0L,RJ0M,RJ0N,"
                 "RK0L,RK0M,RK0N,RL0L,RL0M,RL0N,RM0L,RM0M,RM0N,RN0L,RN0M,RN0N,RO0L,RO0M,RO0N,RQ0L,RQ0M,RQ0N,RT0L,RT0M,RT0N,RU0L,RU0M,RU0N,RV0L,"
                 "RV0M,RV0N,RW0L,RW0M,RW0N,RX0L,RX0M,RX0N,RY0L,RY0M,RY0N,RZ0L,RZ0M,RZ0N,U0L,U0M,U0N,UA0L,UA0M,UA0N,UB0L,UB0M,UB0N,UC0L,UC0M,"
                 "UC0N,UD0L,UD0M,UD0N,UE0L,UE0M,UE0N,UF0L,UF0M,UF0N,UG0L,UG0M,UG0N,UH0L,UH0M,UH0N,UI0L,UI0M,UI0N"},
    {.continent = "AS",
     .country = "Russia - Buryatiya",
     .cq_zone = 18,
     .itu_zone = 32,
     .lat = 53.80,
     .lon = 109.33,
     .prefixes = "R0O,RA0O,RC0O,RD0O,RE0O,RF0O,RG0O,RJ0O,RK0O,RL0O,RM0O,RN0O,RO0O,RQ0O,RT0O,RU0O,RV0O,RW0O,RX0O,RY0O,RZ0O,U0O,UA0O,UB0O,UC0O,"
                 "UD0O,UE0O,UF0O,UG0O,UH0O,UI0O"},
    {.continent = "AS",
     .country = "Russia - Sakha",
     .cq_zone = 19,
     .itu_zone = 23,
     .lat = 66.40,
     .lon = 129.17,
     .prefixes = "R0Q,RA0Q,RC0Q,RD0Q,RE0Q,RF0Q,RG0Q,RI0Q,RJ0Q,RK0Q,RL0Q,RM0Q,RN0Q,RO0Q,RQ0Q,RT0Q,RU0Q,RV0Q,RW0Q,RX0Q,RY0Q,RZ0Q,U0Q,UA0Q,UB0Q,"
                 "UC0Q,UD0Q,UE0Q,UF0Q,UG0Q,UH0Q,UI0Q"},
    {.continent = "AS",
     .country = "Russia - Irkutskaya",
     .cq_zone = 18,
     .itu_zone = 32,
     .lat = 57.37,
     .lon = 106,
     .prefixes = "R0R,R0S,R0T,RA0R,RA0S,RA0T,RC0R,RC0S,RC0T,RD0R,RD0S,RD0T,RE0R,RE0S,RE0T,RF0R,RF0S,RF0T,RG0R,RG0S,RG0T,RJ0R,RJ0S,RJ0T,RK0R,"
                 "RK0S,RK0T,RL0R,RL0S,RL0T,RM0R,RM0S,RM0T,RN0R,RN0S,RN0T,RO0R,RO0S,RO0T,RQ0R,RQ0S,RQ0T,RT0R,RT0S,RT0T,RU0R,RU0S,RU0T,RV0R,RV0S,"
                 "RV0T,RW0R,RW0S,RW0T,RX0R,RX0S,RX0T,RY0R,RY0S,RY0T,RZ0R,RZ0S,RZ0T,U0R,U0S,U0T,UA0R,UA0S,UA0T,UB0R,UB0S,UB0T,UC0R,UC0S,UC0T,"
                 "UD0R,UD0S,UD0T,UE0R,UE0S,UE0T,UF0R,UF0S,UF0T,UG0R,UG0S,UG0T,UH0R,UH0S,UH0T,UI0R,UI0S,UI0T"},
    {.continent = "AS",
     .country = "Russia - Khakassiya",
     .cq_zone = 18,
     .itu_zone = 31,
     .lat = 53.50,
     .lon = 90,
     .prefixes = "R0W,RA0W,RC0W,RD0W,RE0W,RF0W,RG0W,RJ0W,RK0W,RL0W,RM0W,RN0W,RO0W,RQ0W,RT0W,RU0W,RV0W,RW0W,RX0W,RY0W,RZ0W,U0W,UA0W,UB0W,UC0W,"
                 "UD0W,UE0W,UF0W,UG0W,UH0W,UI0W"},
    {.continent = "AS",
     .country = "Russia - Tuva",
     .cq_zone = 23,
     .itu_zone = 32,
     .lat = 51.78,
     .lon = 94.75,
     .prefixes = "R0Y,RA0Y,RC0Y,RD0Y,RE0Y,RF0Y,RG0Y,RJ0Y,RK0Y,RL0Y,RM0Y,RN0Y,RO0Y,RQ0Y,RT0Y,RU0Y,RV0Y,RW0Y,RX0Y,RY0Y,RZ0Y,U0Y,UA0Y,UB0Y,UC0Y,"
                 "UD0Y,UE0Y,UF0Y,UG0Y,UH0Y,UI0Y"},
    {.continent = "AS",
     .country = "Russia - Kamchatskaya",
     .cq_zone = 19,
     .itu_zone = 35,
     .lat = 55.00,
     .lon = 159,
     .prefixes = "R0X,R0Z,RA0X,RA0Z,RC0X,RC0Z,RD0X,RD0Z,RE0X,RE0Z,RF0X,RF0Z,RG0X,RG0Z,RI0X,RI0Z,RJ0X,RJ0Z,RK0X,RK0Z,RL0X,RL0Z,RM0X,RM0Z,RN0X,"
                 "RN0Z,RO0X,RO0Z,RQ0X,RQ0Z,RT0X,RT0Z,RU0X,RU0Z,RV0X,RV0Z,RW0X,RW0Z,RX0X,RX0Z,RY0X,RY0Z,RZ0X,RZ0Z,U0X,U0Z,UA0X,UA0Z,UB0X,UB0Z,"
                 "UC0X,UC0Z,UD0X,UD0Z,UE0X,UE0Z,UF0X,UF0Z,UG0X,UG0Z,UH0X,UH0Z,UI0X,UI0Z"},
    {.continent = "AS",
     .country = "Russia - Zabaykalsky",
     .cq_zone = 18,
     .itu_zone = 32,
     .lat = 54.00,
     .lon = 118,
     .prefixes = "R0U,R0V,RA0U,RA0V,RC0U,RC0V,RD0U,RD0V,RE0U,RE0V,RF0U,RF0V,RG0U,RG0V,RJ0U,RJ0V,RK0U,RK0V,RL0U,RL0V,RM0U,RM0V,RN0U,RN0V,RO0U,"
                 "RO0V,RQ0U,RQ0V,RT0U,RT0V,RU0U,RU0V,RV0U,RV0V,RW0U,RW0V,RX0U,RX0V,RY0U,RY0V,RZ0U,RZ0V,U0U,U0V,UA0U,UA0V,UB0U,UB0V,UC0U,UC0V,"
                 "UD0U,UD0V,UE0U,UE0V,UF0U,UF0V,UG0U,UG0V,UH0U,UH0V,UI0U,UI0V"},
};

// Public variables
bool SYSMENU_callsign_info_opened = false;

// Private Variables
char entered_callsign[MAX_CALLSIGN_LENGTH] = {0};

// Prototypes

// start
void CALLSIGN_INFO_Start(void) {
	LCD_busy = true;

	memset(entered_callsign, 0x00, sizeof(entered_callsign));

	LCD_busy = false;
	LCD_UpdateQuery.SystemMenu = true;
}

// stop
void CALLSIGN_INFO_Stop(void) { LCD_hideKeyboard(); }

static void CALLSIGN_INFO_keyboardHandler(char *string, uint32_t max_size, char entered) {
	char str[2] = {0};
	str[0] = entered;
	if (entered == '<') // backspace
	{
		if (string[0] != '\0') {
			string[strlen(string) - 1] = 0;
		}
	} else if (strlen(string) < 8) {
		strcat(string, str);
	}

	LCD_UpdateQuery.SystemMenuRedraw = true;
}

// draw
void CALLSIGN_INFO_Draw(void) {
	if (LCD_busy) {
		LCD_UpdateQuery.SystemMenuRedraw = true;
		return;
	}
	LCD_busy = true;

	// draw the GUI
	LCDDriver_Fill(BG_COLOR);

	char tmp[64] = {0};

	CALLSIGN_INFO_LINE *info;
	CALLSIGN_getInfoByCallsign(&info, entered_callsign);

	sprintf(tmp, "Enter Callsign: %s", entered_callsign);
	addSymbols(tmp, tmp, 15 + 8, " ", true);
	LCDDriver_printText(tmp, 10, 30, FG_COLOR, BG_COLOR, 2);

	if (entered_callsign[0] != '\0') {
		if (info == NULL) {
			sprintf(tmp, "No info :(");
			addSymbols(tmp, tmp, 15 + 15, " ", true);
			LCDDriver_printText(tmp, 10, 50, FG_COLOR, BG_COLOR, 2);
		} else {
			float32_t lat = info->lat;
			float32_t lon = info->lon;
			float32_t my_lat = LOCINFO_get_latlon_from_locator(TRX.LOCATOR, true);
			float32_t my_lon = LOCINFO_get_latlon_from_locator(TRX.LOCATOR, false);
			uint16_t distance = LOCINFO_distanceInKmBetweenEarthCoordinates(my_lat, my_lon, lat, lon);
			int16_t azimuth = LOCINFO_azimuthFromCoordinates(my_lat, my_lon, lat, lon);

			sprintf(tmp, "Continent: %s", info->continent);
			addSymbols(tmp, tmp, 15 + 15, " ", true);
			LCDDriver_printText(tmp, 10, 50, FG_COLOR, BG_COLOR, 2);

			sprintf(tmp, "Country: %s", info->country);
			addSymbols(tmp, tmp, 15 + 15, " ", true);
			LCDDriver_printText(tmp, 10, 70, FG_COLOR, BG_COLOR, 2);

			sprintf(tmp, "CQ Zone: %u", info->cq_zone);
			addSymbols(tmp, tmp, 15 + 15, " ", true);
			LCDDriver_printText(tmp, 10, 90, FG_COLOR, BG_COLOR, 2);

			sprintf(tmp, "ITU Zone: %u", info->itu_zone);
			addSymbols(tmp, tmp, 15 + 15, " ", true);
			LCDDriver_printText(tmp, 10, 110, FG_COLOR, BG_COLOR, 2);

			sprintf(tmp, "LAT: %.2f LON: %.2f", (double)info->lat, (double)info->lon);
			addSymbols(tmp, tmp, 15 + 15, " ", true);
			LCDDriver_printText(tmp, 10, 130, FG_COLOR, BG_COLOR, 2);

			sprintf(tmp, "Distance: %d km", distance);
			addSymbols(tmp, tmp, 15 + 8, " ", true);
			LCDDriver_printText(tmp, 10, 150, FG_COLOR, BG_COLOR, 2);

			sprintf(tmp, "Azimuth: %d deg", azimuth);
			addSymbols(tmp, tmp, 15 + 8, " ", true);
			LCDDriver_printText(tmp, 10, 170, FG_COLOR, BG_COLOR, 2);
		}
	}

	LCD_printKeyboard(CALLSIGN_INFO_keyboardHandler, entered_callsign, MAX_CALLSIGN_LENGTH - 1, false);

	LCD_busy = false;
}

// events to the encoder
void CALLSIGN_INFO_EncRotate(int8_t direction) {
	/*if (LCD_busy)
	  return;
	LCD_busy = true;

	LCD_busy = false;*/
}

void CALLSIGN_getInfoByCallsign(CALLSIGN_INFO_LINE **ret, char *call) {
	*ret = NULL;
	if (strlen(call) < 1) {
		return;
	}

	// global search
	for (uint16_t i = 0; i < CALLSIGN_DB_GLOBAL_COUNT; i++) {
		uint32_t prefixes_len = strlen(CALLSIGN_INFO_GLOBAL_DB[i].prefixes);
		char *start_ptr = (char *)&CALLSIGN_INFO_GLOBAL_DB[i].prefixes[0];
		uint32_t prefix_tmp_len = 0;

		for (uint32_t j = 0; j < (prefixes_len + 1); j++) {
			prefix_tmp_len++;
			if (CALLSIGN_INFO_GLOBAL_DB[i].prefixes[j] == ',' || CALLSIGN_INFO_GLOBAL_DB[i].prefixes[j] == '\0') {
				char str_tmp[16] = {0};
				strncpy(str_tmp, start_ptr, prefix_tmp_len - 1);

				prefix_tmp_len = 0;
				start_ptr = (char *)&CALLSIGN_INFO_GLOBAL_DB[i].prefixes[j] + 1;

				if (textStartsWith(call, str_tmp)) {
					*ret = (CALLSIGN_INFO_LINE *)&CALLSIGN_INFO_GLOBAL_DB[i];
				}
			}
		}
	}

	// russian search
	for (uint16_t i = 0; i < CALLSIGN_DB_RUS_COUNT; i++) {
		uint32_t prefixes_len = strlen(CALLSIGN_INFO_RUSSIAN_DB[i].prefixes);
		char *start_ptr = (char *)&CALLSIGN_INFO_RUSSIAN_DB[i].prefixes[0];
		uint32_t prefix_tmp_len = 0;

		for (uint32_t j = 0; j < (prefixes_len + 1); j++) {
			prefix_tmp_len++;
			if (CALLSIGN_INFO_RUSSIAN_DB[i].prefixes[j] == ',' || CALLSIGN_INFO_RUSSIAN_DB[i].prefixes[j] == '\0') {
				char str_tmp[16] = {0};
				strncpy(str_tmp, start_ptr, prefix_tmp_len - 1);

				prefix_tmp_len = 0;
				start_ptr = (char *)&CALLSIGN_INFO_RUSSIAN_DB[i].prefixes[j] + 1;

				if (textStartsWith(call, str_tmp)) {
					*ret = (CALLSIGN_INFO_LINE *)&CALLSIGN_INFO_RUSSIAN_DB[i];
				}
			}
		}
	}
}
