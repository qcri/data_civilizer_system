from __future__ import print_function
import sys
from collections import namedtuple
import psycopg2
import csv
import pandas as pd
import json
from os import listdir
import ntpath
# from itertools import izip


DB_Connector = namedtuple('Connector','DB, user, passwd, host, port')
class Connector(DB_Connector):
	def __repr__(self):
		# to_print = ('Database credentials will be set ...')
		to_print = (self.DB + '@' + self.user + '@' + self.host + '@' + self.port)
		return to_print

	def __str__(self):
		return self.__repr__()

def eprint(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)

Conn = Connector('','','','','')


def run_sql_postgres(SQL_str):
	global Conn
	db = Conn.DB
	usr = Conn.user
	pw = Conn.passwd
	hst = Conn.host
	prt = Conn.port
	try:
		conn = psycopg2.connect(database=db, user=usr, password=pw, 
			host=hst, port=prt)
	except:
		print ("I am unable to connect to the database.")
	cur = conn.cursor()
	cur.execute(SQL_str)
	ccc = cur.fetchall()
	return ccc

def read_csv_file(tab_name):
	t_name = ntpath.basename(tab_name)
	try:
		df = pd.read_csv(filepath_or_buffer=tab_name, delimiter=',', low_memory=False,
			quoting=csv.QUOTE_ALL, doublequote=True)
	except ValueError:
		try:
			df = pd.read_csv(filepath_or_buffer=tab_name, delimiter=',', low_memory=False,
				quoting=csv.QUOTE_ALL, doublequote=True, encoding = "ISO-8859-1")
		except:
			print ("Error reading csv file .. file encoding is not recognizable")
	return t_name, df

def read_csv_directory(dir_name):
	csv_datafreames = []
	csv_tables_names = []
	if csv_datafreames:
		for i in range(len(csv_datafreames)):
			csv_datafreames.remove(csv_datafreames[0])
	if csv_tables_names:
		for i in range(len(csv_tables_names)):
			csv_datafreames.remove(csv_tables_names[0])		
	file_extension = '.csv'
	try:
		filenames = listdir(dir_name)
	except:
		print ("Data path not found")
		return None, None
	for f in filenames:
		if f.endswith (file_extension):
			t_name = f.replace(file_extension, "")
			csv_tables_names.append(t_name)
			if dir_name.endswith('/'):
				tab_name = dir_name + f
			else:
				tab_name = dir_name + '/' + f
			try:
				df = read_csv_file(tab_name)
				csv_datafreames.append(df)
			except:
				print("Cannot read table (", tab_name, ")")
				return None, None
	if not csv_datafreames:
		print("This directory contains no (CSV) files .. Returning None")
	return csv_tables_names, csv_datafreames
	

def read_DB_table(jdbc, dbase, hst, prt, usr, pw, table_name, flag = 0):  # see values in a table
	tab_content = []
	header = []
	global Conn
	if not flag:
		Conn = Conn._replace(DB=dbase.strip());
		Conn = Conn._replace(user=usr.strip());
		Conn = Conn._replace(passwd=pw.strip());
		Conn = Conn._replace(host=hst.strip());
		Conn = Conn._replace(port=prt.strip());

	if "postgres" in jdbc.lower().strip():
		sql_str = "SELECT COLUMN_NAME FROM INFORMATION_SCHEMA.COLUMNS "
		sql_str = sql_str + " WHERE TABLE_NAME = \'" + str(table_name)+ "\'"
		sql_str = sql_str + " AND TABLE_SCHEMA = \'public\'"
		H = run_sql_postgres(sql_str)
		for el in H:
			header.append(el[0])

		sql_str = "SELECT * FROM " + str(table_name)
		try:
			contents = run_sql_postgres(sql_str)
			df = pd.DataFrame(contents, columns=header)
			return table_name, df
		except:
			print("Unexpected error:", sys.exc_info()[1])
			return None, None

	if "mysql" in jdbc.lower().strip():
		if not flag:
			print("Not implemented yet ")
		return None, None	
def read_DB(jdbc, dbase, hst, prt, usr, pw):  # see values in a table
	db_datafreames = []
	if db_datafreames:
		for i in range(len(db_datafreames)):
			db_datafreames.removes(db_datafreames[0])
	db_tablenames = []
	if db_tablenames:
		for i in range(len(db_tablenames)):
			db_tablenames.removes(db_tablenames[0])
	global Conn
	Conn = Conn._replace(DB=dbase.strip()); 
	Conn = Conn._replace(user=usr.strip()); 
	Conn = Conn._replace(passwd=pw.strip());
	Conn = Conn._replace(host=hst.strip()); 
	Conn = Conn._replace(port=prt.strip()); 

	if "postgres" in jdbc.lower().strip():
		# sql_str = "SELECT TABLE_NAME FROM INFORMATION_SCHEMA.TABLES WHERE"
		sql_str = "SELECT TABLE_NAME FROM INFORMATION_SCHEMA.TABLES WHERE"
		sql_str = sql_str + " TABLE_TYPE = \'BASE TABLE\'"
		sql_str = sql_str + " AND TABLE_SCHEMA = \'public\' "
		H = run_sql_postgres(sql_str)
		for el in H:
			# print("Reading table (", el[0], ")")
			t, df = read_DB_table(jdbc, dbase, hst, prt, usr, pw, el[0], 1)
			db_datafreames.append(df)
			db_tablenames.append(t)
		return db_tablenames, db_datafreames
		
	if "mysql" in jdbc.lower().strip():
		print("Not implemented yet ")
		return None, None

