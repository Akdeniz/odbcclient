-- CONNECT: takes a connection string and connects.
-- RECONNECT disconnects existing connection and reconnect by using last connection string. It is useful in order to clear cashing etc on client side.
-- IGNOREERROR: continues to execute further queries even if this query is failed.
-- SHOWRESULT: display results of query. Overwrites just_execute program option.
-- NORESULT: do not display results of query. Overwrites just_execute program option.
-- DEFINE <key>=<value> variable declaration. Then variables can be used as such: $(<key>)
-- RESETFOLDER:<folder/path> tries to delete and recreate given folder

DEFINE DBDIR=/tmp/sapiq
RESETFOLDER:$(DBDIR)
DEFINE LOADFILES='/stash/data.csv'

CONNECT:DRIVER=HDBODBC;uid=System;pwd=1234;SERVERNODE=127.0.0.1:30315;DATABASENAME=TT3


--ALTER SYSTEM ALTER CONFIGURATION ('indexserver.ini', 'system') set ('import_export', 'csv_import_path_filter') = '/tmp' with reconfigure
ALTER SYSTEM ALTER CONFIGURATION ('indexserver.ini', 'system') set ('import_export', 'enable_csv_import_path_filter') = 'false' with reconfigure

IGNOREERROR:CREATE SCHEMA MUHAMMED

SET SCHEMA SCHEMA1

IGNOREERROR:DROP TABLE LINEITEM
IGNOREERROR:CREATE TABLE LINEITEM ( "L_ORDERKEY" integer NOT NULL,"L_PARTKEY" BIGINT NOT NULL,"L_SUPPKEY" BIGINT NOT NULL,"L_LINENUMBER" integer NOT NULL,"L_QUANTITY" double NOT NULL,"L_EXTENDEDPRICE" double NOT NULL,"L_DISCOUNT" double NOT NULL,"L_TAX" double NOT NULL,"L_RETURNFLAG" char(1) NOT NULL,"L_LINESTATUS" char(1) NOT NULL,"L_SHIPDATE" date NOT NULL,"L_COMMITDATE" date NOT NULL,"L_RECEIPTDATE" date NOT NULL,"L_SHIPINSTRUCT" char(25) NOT NULL,"L_SHIPMODE" char(10) NOT NULL,"L_COMMENT" varchar(44) NOT NULL)
IMPORT FROM CSV FILE '/tmp/lineitem.tbl' INTO LINEITEM  WITH RECORD DELIMITED BY '\n' FIELD DELIMITED BY '|' DATE FORMAT 'YYYY-MM-DD' ERROR LOG '/tmp/data.err' THREADS 64 BATCH 100000
SHOWRESULT:select * from M_TABLES WHERE TABLE_NAME='LINEITEM'
SHOWRESULT:select count(*) from LINEITEM


