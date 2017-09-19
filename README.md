# odbcclient
Simple ODBC wrapper

# BUILD & RUN

```
sudo apt-get install unixodbc-dev
mkdir build
cd build
cmake ../
make
cd ..

LD_LIBRARY_PATH=<path-of-odbc-library> ODBCSYSINI=<folder-path-of-odbcinst.ini> ./build/odbcclient <options>

LD_LIBRARY_PATH=/opt/SAPIQ/IQ-16_1/lib64/ ODBCSYSINI=./sapiq160/ ./build/odbcchecker -f queries/bench.sql
```

# TODO

- Parse sql file with bison & flex for proper meta syntax
