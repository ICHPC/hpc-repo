#!/usr/bin/env python
from subprocess import check_output
from getpass import getpass
import json
from argparse import *
import sys
import os
import re

def error(errstr):
	print("Error: " + errstr );
	sys.exit(1)


def	publish( username, password, title, description, ff, dd ):
	url="https://" + username + ":" + password + "@data.hpc.imperial.ac.uk/publish/";
	args = [ "/usr/bin/curl", url, "--post302", "-k", "-L", "-s", "-F", "title=" + title , "-F",  "description=" + description ] ;
	for i in range(len(ff)):
		args.append( "-F" )
		args.append( "file-" + str(i) + "=@" + ff[i] )
		args.append( "-F" )
		args.append( "desc-" + str(i) + "=" + dd[i] )

	output = check_output( args, shell=False )
	#print (output)
	x=json.loads(output.decode("utf-8"))
	if 'error' in x:
		error( x['error'] )
	else:
		print( x['doi'] )
	


def find_collection( username, password, col ):
	url="https://" + username + ":" + password + "@data.hpc.imperial.ac.uk/publish/browse/";
	output=check_output( [ "/usr/bin/curl", url, "-k", "-L", "-s" ] );
	x=json.loads(output.decode("utf-8"))
	coll=col.lower()
	for f in (x['collections']):
		if f['doi'].lower() == coll or f['title'].lower() == coll:
			return re.sub( r'^.*\/', '', f['doi'] )
	# Make a new collection

def make_collection( username, password, title, description ):

	url="https://" + username + ":" + password + "@data.hpc.imperial.ac.uk/publish/collection/";
	args = [ "/usr/bin/curl", url, "-k", "-L", "-s", "-F", "title=" + title , "-F",  "description=" + description ] ;
	output = check_output( args, shell=False )
	#print (output)
	x=json.loads(output.decode("utf-8"))
	if 'error' in x:
		error( x['error'] )
	else:
		print( x['doi'] )
	

if __name__ == "__main__":

	p = ArgumentParser( "Publish to data.hpc.imperial.ac.uk. See https://wiki.ch.ic.ac.uk/wiki/index.php?title=Rdm:data" ) 
	p.add_argument( "--username", default=None, dest="username", nargs=1, action="store",  help="Imperial College username" )
	p.add_argument( "--password", default=None, dest="password", nargs=1, action="store" , help="Imperial College password" )
	p.add_argument( "--title", required=True, dest="title", nargs=1, action="store", help="Deposition title" )
	p.add_argument( "--description", required=True, dest="desc", nargs=1, action="store", help="Deposition description. String or @<filename>" )
	p.add_argument( "--file", dest="files", default=None, action="append", nargs=2, help="List of files to deposit [<filename> description]" )
	p.add_argument( "--collection", default=None, dest="collection", nargs=1, action="store", help="Collection to include deposition in" )
	p.add_argument( "--make-collection", default=False,  dest="makecollection", action="store_const", const=True,  help="Make a new collection. Requires --title and --description" )

	x=p.parse_args( sys.argv[1:] )

	if x.username:
		username=(x.username[0])
	else:
		username=None

	if x.password:
		password=(x.password[0])
	else:
		password=None

	files=x.files
	if x.title:
		title=str(x.title[0])
	if x.desc:
		desc=str(x.desc[0])
	if x.collection:
		collection=str(x.collection[0])
	else:
		collection=None

	title=title.strip()
	desc=desc.strip()

	if desc.startswith('@'):
		ff=re.sub( r'@', '', desc )
		if not os.path.isfile(ff):
			error( "File " +ff+" does not exist" )

		myf = open( ff, "r" )
		desc=myf.read()
		desc=desc.strip()
		myf.close()

	if not len(title) or not len(desc):
		error("Title and description must be set")
	
	while not username:
		print( "Username: ", end="" )
		sys.stdout.flush()
		username=input().strip()

	while not password:
		sys.stdout.flush()
		password=getpass().strip()

	if x.makecollection:
		make_collection( username, password, title, desc )
		sys.exit(0)

	ff=[]	
	dd=[]
	for f in files:
		filename=f[0]
		filedesc=f[1]
		if not os.path.isfile( filename ):
			error( "File " + filename + " not found" )
		ff.append( filename )
		dd.append( filedesc )

	if collection:
		doi=find_collection( username, password, collection )
		if not doi:
			error( "Collection not found." )


	publish( username, password, title, desc, ff, dd )

	sys.exit(0)

