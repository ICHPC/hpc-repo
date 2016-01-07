#!/usr/bin/env python
from psycopg2 import *
from psycopg2.extras import *
try:
	f=open("/var/www/data.hpc.imperial.ac.uk/repo/client/sitemap.txt", "w" )
	con = connect("dbname='repository' user='repository' host='localhost' password='repository'")
	cur = con.cursor( cursor_factory = RealDictCursor )
	cur.execute("SELECT * FROM doi WHERE embargoed = FALSE ORDER BY doi ASC")
	row=cur.fetchall()
	for a in row:
		f.write("https://data.hpc.imperial.ac.uk/resolve/?doi=" + str(a['doi'])  + "\n" )

	f.close()
except:
	raise

