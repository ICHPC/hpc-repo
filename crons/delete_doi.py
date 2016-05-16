#!/usr/local/python/bin/python
from psycopg2 import *
from psycopg2.extras import *
from lxml.etree import *
import re
import configparser
import requests
import base64
import sys
import os

def delete( doiauth, full_doi ):
			headers={
				'Content-Type' : "application/xml;charset=UTF-8",
				"Authorization" : "Basic " +  doiauth 
			}
		#	print(headers)
			url = "https://mds.datacite.org/metadata/" + full_doi;
			print(url)
			r = requests.delete( url, headers= headers )
			if(  r.status_code > 201 ):
				print("ERROR: Status code:" + str( r.status_code) )
				print( "RESPONSE\n" + r.content.decode("ascii"))
				return False
			else:
				return True


if __name__ == "__main__":
	
	cp = configparser.ConfigParser( interpolation=None )
	cp.read('/var/www/data.hpc.imperial.ac.uk/repo/configuration.ini' )


	doi_prefix = re.sub( '"', '', cp.get( "datacite", "dc_prefix")) 
	doi_user   = re.sub( '"', '', cp.get( "datacite", "dc_user")) 
	doi_pass   = re.sub( '"', '', cp.get( "datacite", "dc_password"))
	db_name    = re.sub( '"', '', cp.get( "database", "db_dbname" ))
	db_user    = re.sub( '"', '', cp.get( "database", "db_user" ))
	db_host    = re.sub( '"', '', cp.get( "database", "db_host" ))
	db_password= re.sub( '"', '', cp.get( "database", "db_password" ))

	doiauth = base64.b64encode( (doi_user + ":" + doi_pass).encode("ascii") )
	doiauth = doiauth.decode("ascii")

	for a in [ 122, 123, 124, 125, 126, 127, 128, 129, 130, 132, 133, 142, 89, 90, 91, 92, 93, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110 ]:	
		full_doi = doi_prefix + str(a)

		print("=== Deleting DOI " + full_doi )

		ret1 = delete   ( doiauth, full_doi )

