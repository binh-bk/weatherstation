### install sqlite3:
```sudo apt-get install sqlite3```

live@tp ~/Desktop $ sudo apt-get install sqlite3
[sudo] password for live: 
Reading package lists... Done
Building dependency tree       
Reading state information... Done
sqlite3 is already the newest version (3.11.0-1ubuntu1).
0 upgraded, 0 newly installed, 0 to remove and 61 not upgraded.

### create database and a table, in the terminal
live@tp ~/Desktop $ sqlite3 weatherstation.db
SQLite version 3.11.0 2016-02-15 17:29:24
Enter ".help" for usage hints.
sqlite> CREATE TABLE weatherdata (id INT PRIMARY KEY, thetime DATETIME, ldr INT, tls2561 INT, ds18b20 REAL, tsht21 REAL, hsht21 REAL);
sqlite> .database
seq  name             file                                                      
---  ---------------  ----------------------------------------------------------
0    main             /home/live/Desktop/weatherstation.db                      
sqlite> .fullschema
CREATE TABLE weatherdata (id INT PRIMARY KEY, thetime DATETIME, ldr INT, tls2561 INT, ds18b20 REAL, tsht21 REAL, hsht21 REAL);
/* No STAT tables available */
sqlite> .exit
live@tp ~/Desktop $ 

### link the full path of the database in the Python script 

dbFile = "/path/to/databse/weatherstation.db"

### screenshot
<img src='https://github.com/binh-bk/weatherstation/blob/master/esp8266_deepsleep_Aug4/Desktop_026.png'/>




