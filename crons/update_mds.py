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
import datetime


def has_orcid( con, doi ):
	cur = con.cursor( cursor_factory = RealDictCursor )
	cur.execute( "SELECT orcid FROM account A JOIN doi B on A.user_id = B.creator WHERE doi = %s",  [doi]  )
	ret = cur.fetchall()
	cur.close()
	return ret[0]['orcid']



def update_files( con, doi, full_doi, doiauth, files ):
			headers={
				'Content-Type' : "application/xml;charset=UTF-8",
				"Authorization" : "Basic " +  doiauth 
			}
			x=1
			for f in files:
				sanitisefn = re.sub( "[ ><,+!'()&%$/;:]", "_", f['filename'] )
				key = f['mimetype'] 
				key = key.lower()
				value = "https://data.hpc.imperial.ac.uk/resolve/?doi=" + str(doi) + "&file=" + str(x)
				md = key + "=" + value
				r = requests.post( "https://mds.datacite.org/media/" + full_doi, headers= headers, data = md )
				
				print(" - POST MEDIA"  )
				if(  r.status_code != requests.codes.ok ):
					print( "ERROR: Status code:" + str( r.status_code) )
					print( "REQUEST  : " + md )
					print( "RESPONSE : " + r.content.decode("ascii"))
					return False

	

				x=x+1
			return True

def update_virtual_files( con, doi, full_doi, doiauth, files ):
			headers={
				'Content-Type' : "application/xml;charset=UTF-8",
				"Authorization" : "Basic " +  doiauth 
			}
			x=1
			for f in files:
				v = False

				key = f['mimetype'] 
				key = key.lower()
				value = "https://data.hpc.imperial.ac.uk/resolve/?doi=" + str(doi) + "&file=" + str(-x)

        # Virtual files
				if key == "chemical/x-mnova":
					key = "chemical/x-mnpub"
					v = True
				elif key == "application/zip":
					key = "chemical/x-mnpub"
					v = True

				if v:
					md = key + "=" + value
					r = requests.post( "https://mds.datacite.org/media/" + full_doi, headers= headers, data = md )
					print(" - POST MEDIA"  )
					if(  r.status_code != requests.codes.ok ):
						print( "ERROR: Status code:" + str( r.status_code) )
						print( "REQUEST  : " + md )
						print( "RESPONSE : " + r.content.decode("ascii"))
						return False

				x=x+1
			return True



def update_metadata( con, doi, full_doi, doiauth, md ):
			headers={
				'Content-Type' : "application/xml;charset=UTF-8",
				"Authorization" : "Basic " +  doiauth 
			}
		#	print(headers)
			r = requests.post( "https://mds.datacite.org/metadata", headers= headers, data = md )

			print(" - POST METADATA")

#			r2 = requests.get( "https://mds.datacite.org/metadata/" + full_doi, headers=headers );
#			print(r2.content.decode("ascii"))
			if(  r.status_code > 201 ):
				print("ERROR: Status code:" + str( r.status_code) )
				print( "RESPONSE\n" + r.content.decode("ascii"))
				return False
			else:
				return True

			
#		sys.exit(0)


def get_heirarchy( con, doi ):
	cur = con.cursor( cursor_factory = RealDictCursor )
	cur.execute( "SELECT child FROM membership WHERE parent=%s", [doi]  )
	children = cur.fetchall()
	cur.close()

	cur = con.cursor( cursor_factory = RealDictCursor )
	cur.execute( "SELECT parent FROM membership WHERE child=%s", [doi]  )
	parents = cur.fetchall()
	cur.close()
	return  { "parents": parents, "children" : children }

def get_assoc( con, doi ):
	cur = con.cursor( cursor_factory = RealDictCursor )
	cur.execute( "SELECT * FROM associated WHERE doi = %s ORDER BY associated ASC", [doi]  )
	ret = cur.fetchall()
	cur.close()
	return ret




def get_collaborators( con, doi ):
	cur = con.cursor( cursor_factory = RealDictCursor )
	cur.execute( "SELECT * FROM collaboration A LEFT JOIN account B on A.user_id = B.user_id WHERE A.doi = %s", [doi]  )
	ret = cur.fetchall()
	cur.close()
	return ret

def get_external_collaborators( con, doi ):
	cur = con.cursor( cursor_factory = RealDictCursor )
	cur.execute( "SELECT * FROM external_collaborator WHERE doi = %s", [doi]  )
	ret = cur.fetchall()
	cur.close()
	return ret



def get_files( con, doi ):
	cur = con.cursor( cursor_factory = RealDictCursor )
	cur.execute( "SELECT * FROM file WHERE doi = %s AND deprecated=FALSE ORDER BY seq ASC", [doi]  )
	ret = cur.fetchall()
	cur.close()
	return ret


def get_metadata( con, doi ):
	cur = con.cursor( cursor_factory = RealDictCursor )
	cur.execute( "SELECT * FROM metadata WHERE doi = %s ORDER BY key ASC", [doi]  )
	ret = cur.fetchall()
	cur.close()
	return ret

#<ns0:resource xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:ns0="http://datacite.org/schema/kernel-2.2" xsi:schemaLocation="http://datacite.org/schema/kernel-2.2 http://schema.datacite.org/meta/kernel-2.2/metadata.xsd">


def create_metadata( doi_prefix,record, collabs, externalcollabs, files, metadata , heir, assoc ):
	doi = doi_prefix + str(record['doi'])
	ns = "http://datacite.org/schema/kernel-3"
	xsi="http://www.w3.org/2001/XMLSchema-instance"
	resource =  Element(  '{'+ns+'}resource', nsmap={ None:ns, "xsi":xsi } )
	
	resource.set( "{" + xsi+ "}schemaLocation", "http://datacite.org/schema/kernel-3 http://schema.datacite.org/meta/kernel-3/metadata.xsd" )
	identifier = Element( "identifier" );
	identifier.text = doi
	identifier.set( "identifierType", "DOI" )
	resource.append( identifier )

	creators = SubElement( resource, "creators" );

	creator = SubElement( creators, "creator" )
	SubElement( creator, "creatorName" ).text = record['name']
	ni = SubElement( creator, "nameIdentifier" )
	ni.set( "schemeURI", "http://orcid.org" )
	ni.set( "nameIdentifierScheme", "ORCID" )
	ni.text = record['orcid']
	SubElement( creator, "affiliation" ).text = "Imperial College London"

	contributors = SubElement( resource, "contributors" )
	contributor = SubElement( contributors, "contributor" )
	contributor.set( "contributorType", "HostingInstitution" )
	SubElement( contributor, "contributorName" ).text = "Imperial College London"

	contributor = SubElement( contributors, "contributor" )
	contributor.set( "contributorType", "DataManager" )
	SubElement( contributor, "contributorName" ).text = "Imperial College High Performance Computing Service"

	# Add the creator as a contrubtor so it shos up in the MDS landing page
	contributor = SubElement( contributors, "contributor" )
	contributor.set( "contributorType", "Researcher" )
	SubElement( contributor, "contributorName" ).text = record['name']
	ni = SubElement( contributor, "nameIdentifier" )
	ni.set( "schemeURI", "http://orcid.org" )
	ni.set( "nameIdentifierScheme", "ORCID" )
	ni.text = record['orcid']
	SubElement( contributor, "affiliation" ).text = "Imperial College London"


		
	for rec in collabs:
		contributor = SubElement( contributors, "contributor" )
		contributor.set( "contributorType", "Researcher" )
		SubElement( contributor, "contributorName" ).text = rec['name']
#	SubElement( contributor, "affiliation" ).text = "Imperial College London"
		ni = SubElement( contributor , "nameIdentifier" )
		ni.set( "schemeURI", "http://orcid.org" )
		ni.set( "nameIdentifierScheme", "ORCID" )
		ni.text = rec['orcid']
		SubElement( contributor, "affiliation" ).text = "Imperial College London"

	for rec in externalcollabs:
		contributor = SubElement( contributors, "contributor" )
		contributor.set( "contributorType", "Researcher" )
		SubElement( contributor, "contributorName" ).text = rec['name']
#	SubElement( contributor, "affiliation" ).text = "Imperial College London"
		ni = SubElement( contributor , "nameIdentifier" )
		ni.set( "schemeURI", "http://orcid.org" )
		ni.set( "nameIdentifierScheme", "ORCID" )
		ni.text = rec['orcid']
#		SubElement( contributor, "affiliation" ).text = "Imperial College London"



	descriptions= SubElement( resource, "descriptions" )
	description = SubElement( descriptions, "description" )
	description.text = record['description']
	description.set( "descriptionType", "Other" )

	titles = SubElement( resource, "titles" )
	SubElement( titles, "title" ).text = record['title']
	SubElement( resource, "publisher" ).text="Imperial College London" 
	SubElement( resource, "publicationYear").text = str( record['creation_date'].timetuple().tm_year )
	dds= SubElement( resource, "dates" )
	dd = SubElement( dds, "date")
	dd.text = str( record['creation_date'] )
	dd.set( "dateType", "Created" )

	dd = SubElement( dds, "date")
	dd.text = str( datetime.datetime.now() )
	dd.set( "dateType", "Updated" )

	relateds = SubElement( resource, "relatedIdentifiers" )
	related  = SubElement( relateds, "relatedIdentifier" )
	related.set( "relatedIdentifierType", "URL" )
	related.set( "relationType", "HasMetadata" )
	related.set( "relatedMetadataScheme", "ORE" )
	related.set( "schemeURI", "http://www.openarchives.org/ore/" )
	related.text="https://data.hpc.imperial.ac.uk/resolve/?ore=" + str(record['doi'])

	for f in files:
		related  = SubElement( relateds, "relatedIdentifier" )
		related.set( "relatedIdentifierType", "URL" )
		related.set( "relationType", "HasPart" )
		related.set( "relatedMetadataScheme", "Filename" )
		related.set( "schemeURI", "filename://" + f['filename'] )
		related.text="https://data.hpc.imperial.ac.uk/resolve/?doi=" + str(record['doi']) + "&file=" + str(f['seq']+1)

		

	subjects = SubElement( resource, "subjects" )
	for a in metadata:
		subject = SubElement( subjects, "subject" )
		subject.text = a['value']
		subject.set( "subjectScheme", a['key'] )
		if a['key'] == "inchi":
			subject = SubElement( subjects, "subject" )
			subject.set( "subjectScheme", "The InChI Trust")
			subject.set( "subjectURI", "http://www.inchi-trust.org/technical-faq")
			

	for a in assoc:
		if a['associated']:
			related  = SubElement( relateds, "relatedIdentifier" )
			related.set( "relatedIdentifierType", "DOI" )
			related.set( "relationType", "IsReferencedBy" )
			related.text= a['associated']
		


	children = heir['children']
	parents  = heir['parents']	
	for p in parents:
		related  = SubElement( relateds, "relatedIdentifier" )
		related.set( "relatedIdentifierType", "DOI" )
		related.set( "relationType", "IsPartOf" )
		related.text= doi_prefix + str(p['parent'])
	for p in children:
		related  = SubElement( relateds, "relatedIdentifier" )
		related.set( "relatedIdentifierType", "DOI" )
		related.set( "relationType", "HasPart" )
		related.text= doi_prefix + str(p['child'])




		

	rightslist = SubElement( resource, "rightsList" )
	rights     = SubElement( rightslist, "rights" )
	rights.set("rightsURI", "https://creativecommons.org/publicdomain/zero/1.0/");
	rights.text="Creative Commons Public Domain Dedication (CC0 1.0)"

	resourcetype = SubElement( resource, "resourceType" )

	if( record['collection'] ):
		resourcetype.set( "resourceTypeGeneral", "Collection" )
	else:
		resourcetype.set( "resourceTypeGeneral", "Dataset" )


	return tostring( resource, pretty_print=True ).decode("utf-8")



if __name__ == "__main__":
	import sys
	
	cp = configparser.ConfigParser( interpolation=None )
	cp.read('/var/www/data.hpc.imperial.ac.uk/repo/configuration.ini' )

	do_all = ( "--all" in sys.argv )
	doi_prefix = re.sub( '"', '', cp.get( "datacite", "dc_prefix")) 
	doi_user   = re.sub( '"', '', cp.get( "datacite", "dc_user")) 
	doi_pass   = re.sub( '"', '', cp.get( "datacite", "dc_password"))
	db_name    = re.sub( '"', '', cp.get( "database", "db_dbname" ))
	db_user    = re.sub( '"', '', cp.get( "database", "db_user" ))
	db_host    = re.sub( '"', '', cp.get( "database", "db_host" ))
	db_password= re.sub( '"', '', cp.get( "database", "db_password" ))
	
	con = connect("dbname='%s' user='%s' host='%s' password='%s'" % ( db_name, db_user, db_host, db_password) )
	cur = con.cursor( cursor_factory = RealDictCursor )

	if do_all:
		cur.execute("SELECT * FROM doi A LEFT JOIN account B ON A.creator = B.user_id WHERE A.embargoed = FALSE ORDER BY doi ASC")
	else:
		cur.execute("SELECT * FROM doi A LEFT JOIN account B ON A.creator = B.user_id WHERE A.embargoed = FALSE AND A.updated = TRUE ORDER BY doi ASC")

	row=cur.fetchall()
	for a in row:
##		try:
			doi=int(a['doi'])
			collabs = get_collaborators( con, doi ) 
			externalcollabs = get_external_collaborators( con, doi );
			files   = get_files( con, doi )
			metadata= get_metadata( con, doi )
			heirarchy=get_heirarchy( con, doi )
			assoc    =get_assoc( con, doi )
#		print(collabs)
#		print(files)
#		print(metadata)

			doiauth = base64.b64encode( (doi_user + ":" + doi_pass).encode("ascii") )
			doiauth = doiauth.decode("ascii")
			full_doi = doi_prefix + str(a['doi'])

			if( has_orcid( con, doi ) ):      
				print("=== Updating metadata for DOI " + full_doi )

				md = create_metadata( doi_prefix , a, collabs, externalcollabs, files, metadata, heirarchy, assoc )

#			print(md)
				ret2 = False
				ret1 = False

				ret2 = update_metadata( con, doi, full_doi, doiauth, md )
				if ret2:
					ret1 = update_files   ( con, doi, full_doi, doiauth, files )
				else:
					print(" - UPDATE FAILED: " )
					print( md )
				if ret2:
					ret1 = update_virtual_files   ( con, doi, full_doi, doiauth, files )
				else:
					print(" - UPDATE FAILED: " )
					print( md )



				if( ret1 and ret2 ):
					print(" - MARKING COMPLETE" )
					cur = con.cursor( cursor_factory = RealDictCursor )
					cur.execute("UPDATE doi SET updated = FALSE WHERE doi=%s", [ doi ] )
					cur.close()
					con.commit()
			else:
				print("=== Skipping DOI " + full_doi + " : no ORCID" );

#		except:
#			raise
	
	cur.close()

