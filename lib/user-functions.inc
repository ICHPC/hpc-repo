<?php

function is_service_account( $id ) {
	$ret = db_query( "SELECT service FROM account WHERE user_id = ?", array($id));
	return ($ret['service'] === true );

}

function get_username_by_authtoken( $token ) {
	$ret = db_query( "SELECT username FROM account WHERE access_token=?", array($token) );
	return $ret['username'];
}

function make_embargo_pass() {
  $let='2345679acdefghkmnpqrtuvwxyz';
  $code="";
  for($i=0;$i<8;$i++) {
    if( ($i % 4 == 0) && ($i>0) ) {
      $code=$code."-";
    }
		$code=$code . $let[ rand(0, strlen($let)-1 ) ];
	}
	return $code;
}

function get_user_id( $username ) {
	$q = "INSERT INTO account ( username ) VALUES ( ? )";
	$r = db_query( $q, array($username) );
	$q = "SELECT * FROM account WHERE username=?";
	$r = db_query( $q, array($username) );
	return $r['user_id'];
}

function insert_in_doi_table( $user_id, $title, $desc, $is_collection, $url ) {
	$r = db_query( "SELECT nextval('doi_seq' )", array() );
	$doi_seq = $r['nextval'];
#	print "DOI IS $doi_seq";

	if( $url === 0 ) {
    if( $is_collection ) {
			$pass = make_embargo_pass();
			db_query( "INSERT INTO DOI( doi, creator, title, description, collection, embargoed, embargo_pass  ) VALUES ( ?,?,?,?, ?, true, ? )", 
			array( $doi_seq, $user_id, $title, $desc, $is_collection, $pass  ) );
    }
    else {
			db_query( "INSERT INTO DOI( doi, creator, title, description, collection  ) VALUES ( ?,?,?,?, ? )", 
			array( $doi_seq, $user_id, $title, $desc, $is_collection  ) );
    }
	}
	else {
		db_query( "INSERT INTO DOI( doi, creator, title, description,  urlmapping ) VALUES ( ?,?,?,?, ? )", 
		array( $doi_seq, $user_id, $title, $desc, $url ) );
		
	}

	return $doi_seq;
}

function reset_mimes() {
 $R = db_query_all( "SELECT * FROM file", array() );
 foreach($R as $r) {
   $mime = guess_mime( $r['filename'] );
	 db_query_all( "UPDATE file SET mimetype=? WHERE file_seq=?", array( $mime, $r['file_seq'] ) );
	 print("<p>" .  $r['filename'] . " --> " . $mime . "</p>" );
 }
}

function guess_mime( $filename ) {
	$R = db_query_all( "SELECT * FROM mime", array() );
	$mime ="application/octet-stream";
	$val=-1;

	foreach ($R as $r) {
		if( preg_match( "/" . $r['regex'] ."/", $filename ) && ($r['priority'] > $val)  && strlen($r['mime'])<64 ) {
			$mime = $r['mime'];
		}
		
	}
 
	return $mime;
}

function get_urlmapping_by_uid( $user_id ) {
	global $REPO_options;

	$R = db_query_all( "SELECT * FROM DOI WHERE collection = false AND urlmapping is not NULL  AND creator=? order by creation_date desc", array( $user_id) );
	$RR=array();
	foreach( $R as $A ) {
		$RR[] = array(
		# index, description, doi 	
			'creation_date' => $A['creation_date'],
			"index" => $A['doi'],
			"description" => $A['title'],
			"url" => $A['urlmapping'],
			"doi" => $REPO_options['dc_prefix'] . $A['doi']
		);
	}


	return $RR;

}

function get_entry_by_uid( $user_id ) {
	global $REPO_options;

	$R = db_query_all( "SELECT * FROM DOI WHERE collection = false AND urlmapping=NULL  AND creator=?", array( $user_id) );
	$RR=array();
	foreach( $R as $A ) {
		$RR[] = array(
		# index, description, doi	
			"index" => $A['doi'],
			"description" => $A['title'],
			"doi" => $REPO_options['dc_prefix'] . $A['doi']
		);
	}


	return $RR;
}

function get_collections_and_collaborations_by_uid( $user_id ) {
	$r1 = get_collections_by_uid( $user_id );
	$r2 = get_collaborations_by_uid( $user_id );
	$r3 = array_merge( $r1, $r2 );
  $none = array( array(
   'index' => -1,
   'doi' => '',
   'description' => '---'
  ) );
  $r3 = array_merge( $none, $r3 );
	return $r3;

}

function get_collaborations_by_uid( $user_id ) {
	global $REPO_options;
	$R = db_query_all( "SELECT * FROM collaboration WHERE user_id = ?", array($user_id) );
  $rr=array();
  foreach( $R as $r ) {
    $Q = db_query( "SELECT * FROM doi A LEFT JOIN account B on A.creator = B.user_id WHERE A.doi = ? ORDER BY B.name, A.creation_date", array($r['doi'] ) );
     $rr[] = array(
     'index' => $Q['doi'],
		 'doi'   => $REPO_options['dc_prefix'] .  $Q['doi'],
     'description' =>  "(" . $Q['name'] . ") " . $Q['title']  ,
   );
  }
	return $rr;

}

function get_collections_by_uid( $user_id ) {
	global $REPO_options;

	$R = db_query_all( "SELECT * FROM DOI WHERE collection=true and creator=?", array( $user_id) );
	$RR=array();
	foreach( $R as $A ) {
		$RR[] = array(
		# index, description, doi	
			"index" => $A['doi'],
			"description" => $A['title'],
			"doi" => $REPO_options['dc_prefix'] . $A['doi'],
      "embargoed" => $A['embargoed'],
      "embargo_pass" => $A['embargo_pass'],
     
		);
	}

#print_r($RR);
	return $RR;
}
function store_file( $doi, $tmpname, $name, $size, $desc ) {

	# Get the location of the filestore
	global $REPO_options ;
	$prefix= $REPO_options[ 'filestore_path' ];
	$prefix= ( $prefix . "/" . $doi );
	if( !is_dir($prefix) && ! mkdir( $prefix ) ) {
		return "Internal error making directory";
	}

	$r=db_query( "SELECT COUNT(*) FROM FILE WHERE DOI=?", array( $doi ) );
	$count=$r['count'];
	$prefix = $prefix . "/" . $count ;	
	if( ! move_uploaded_file( $tmpname , $prefix ) ) {
		return "Internal error storing file";
	}

	$mimetype = guess_mime( $name );
	db_query( "INSERT INTO FILE (doi, seq, filename, object, mimetype, description, size ) VALUES ( ?, ?, ?, ?, ?, ?, ? )", array( $doi, $count, $name,  "file://" . $prefix, $mimetype, $desc, $size ) );

	return 0;
}

function doi_owner( $doi ) {
	$R = db_query( "SELECT * FROM DOI WHERE doi=?", array( (int)($doi) ) );
	return $R['creator'];
}

function publish( $user_id , $is_collection ) {

	if(!array_key_exists( 'title', $_REQUEST )) { return  array( "error" => "'title' not set" ); }
	if(!array_key_exists( 'description', $_REQUEST )) { return array( "error" => "'description' not set" ); }
	$title = $_REQUEST['title'];
	$desc  = $_REQUEST['description'];

	$memberof = -1;
	if( array_key_exists( 'memberof', $_REQUEST ) ) {
		$memberof = (int)($_REQUEST['memberof' ]);
	}

	if( ((int)$memberof) !== -1 ) {
		if( (doi_owner( $memberof ) !== $user_id)  &&  (! is_collaborator( $user_id, $memberof )) ) {
			return array( "error" => "Not owner $user_id  $memberof " . doi_owner($memberof)  );
		}
	}	
	if( $title === "" ) { return array( "error" => "'title' not set"); }
	if( $desc  === "" ) { return array( "error" => "'description' not set"); }

	# Step 1 - insert into DOI table
	if( $is_collection ) { $is_collection=1; } else { $is_collection=0; }
	$doi_idx = insert_in_doi_table( $user_id, $title, $desc, $is_collection, 0 );

	# Step 1b insert into membership
	if( $memberof !== -1 ) {
		db_query( "INSERT INTO membership ( parent, child ) VALUES (?,?)", array( $memberof, $doi_idx ) );
	  mark_doi_updated($memberof);
	}
	# Step 2 - store files

#  print_r($_FILES);

	foreach( $_FILES as $key => $value ) {
		if ( substr($key, 0, 5) === "file-" ) {
			$f=$_FILES[$key];
			$idx=substr($key, 5, strlen($key) );
      if( is_array( $f['error'] ) ) { # New file[]  upload method
 	     for( $subset = 0 ; $subset < count($f['error']); $subset++ ) {
				if( $f['error'][$subset]===0 ) {
					$size = $f['size'][$subset];
					$name = $f['name'][$subset];
					$tmpname= $f['tmp_name'][$subset];
					$desc = $_REQUEST['desc-'.$idx];
					make_metadata( $doi_idx, $tmpname, $name );
					$ret = store_file( $doi_idx, $tmpname, $name, $size, $desc ); 
					if ($ret !== 0 ) {
						return array( "error" => $ret );
					}
				}
				else {
						return array( "error" => "Upload of file failed -- max file size is 128MB" );
				}
       }
			}
			else {
				if( $f['error']===0 ) {
					$size = $f['size'];
					$name = $f['name'];
					$tmpname= $f['tmp_name'];
					$desc = $_REQUEST['desc-'.$idx];
					make_metadata( $doi_idx, $tmpname, $name );
					$ret = store_file( $doi_idx, $tmpname, $name, $size, $desc ); 
					if ($ret !== 0 ) {
						return array( "error" => $ret );
					}
				}
				else {
						return array( "error" => "Upload of file failed -- max file size is 128MB" );
				}

			}
		}
	}


	# Step 3 - register DOI
	$doi =  register_doi( $doi_idx, $user_id, $title );
   $doi["index"] = $doi_idx;
  $doi['embargo_pass'] = get_embargo_pass($doi_idx);
	return $doi;
}

function get_repo_url() {
	# TODO _ give this a configuration option (Eg if we are behind a proxy

	return "https://". $_SERVER['HTTP_HOST'];
}

function register_doi( $doi_idx, $uid, $title ) {

	$realname = get_gecos_by_id( $uid );
	$orcid = get_orcid_by_id( $uid );
	$repo_url = get_repo_url();
	$title = $title;

	global $REPO_options ;
	$doi = $REPO_options[ 'dc_prefix' ];
	$doi = $doi .  $doi_idx;
	$doi_full = $doi;


	# Step 1  is to make the simple metadata xml
	$xml = new SimpleXMLElement('<resource xmlns="http://datacite.org/schema/kernel-3"></resource>' );
	$xml->addAttribute( 'xsi:schemaLocation', "http://datacite.org/schema/kernel-3 http://schema.datacite.org/meta/kernel-3/metadata.xsd",  "http://www.w3.org/2001/XMLSchema-instance" );
	$doi = $xml->addChild( "identifier", $doi );
	$doi->addAttribute( "identifierType", "DOI" );

	$creators = $xml->addChild( "creators" );
	$creator  = $creators->addChild( "creator" );
	$creator->addChild( "creatorName", $realname );
	$o = $creator->addChild( "nameIdentifier", $orcid );

	$o->addAttribute( "schemeURI", "http://orcid.org" );
	$o->addAttribute( "nameIdentifierScheme", "ORCID" );

#	$creator = $creators->addChild( "creator" );
#	$creator->addChild( "creatorName", "Imperial College London High Performance Computing Service" );


	$creators = $xml->addChild( "contributors" );
	$creator  = $creators->addChild( "contributor" );
	$creator->addAttribute( "contributorType", "Researcher" );
	$creator->addChild( "contributorName", $realname );
	$o = $creator->addChild( "nameIdentifier", $orcid );

	$o->addAttribute( "schemeURI", "http://orcid.org" );
	$o->addAttribute( "nameIdentifierScheme", "ORCID" );

	$t = $xml->addChild( "titles" );
	$t->addChild( "title", $title );
	$xml->addChild( "publisher", "Imperial College London" );
	$xml->addChild( "publicationYear", date("Y")  );


	$r = $xml->addChild("relatedIdentifiers" );
	$ore = $r->addChild( "relatedIdentifier", $repo_url . "/resolve/?ore=" .$doi_idx );
	$ore->addAttribute( "relatedIdentifierType", "URL" );
	$ore->addAttribute( "relationType", "HasMetadata" );
	$ore->addAttribute( "relatedMetadataScheme", "ORE" );
	$ore->addAttribute( "schemeURI", "http://www.openarchives.org/ore/" );

	$RR = db_query_all( "select * from membership a left join doi b on  a.parent=b.doi where a.child=?" , array( $doi_idx ) );
	foreach( $RR as $x ) {
		$f = $r->addChild( "relatedIdentifier", $REPO_options['dc_prefix'] . $x['parent'] );
		$f->addAttribute( "relatedIdentifierType", "DOI" );
		$f->addAttribute( "relationType", "IsPartOf" );
	}

	$rights = $xml->addChild( "rightsList" );
	#$r = $rights->addChild("rights", "Creative Commons Attribution (CC BY 3.0)" );
	#$r->addAttribute( "rightsURI", "http://creativecommons.org/licenses/by/3.0/" );
	$r = $rights->addChild("rights", "Creative Commons Public Domain (CC0)" );
	$r->addAttribute( "rightsURI", "https://creativecommons.org/publicdomain/zero/1.0/");
#print "<!--\n";
#	print $xml->asXML();
#print "-->\n";
	$xmltext = $xml->asXML();

	# Step 2 - publish metadata to datacite

	$headers=array ( 'Content-Type:application/xml;charset=UTF-8', "Authorization: Basic " .  base64_encode( $REPO_options['dc_user']. ":" . $REPO_options['dc_password'] ) );


	$p = curl_init();
	curl_setopt( $p, CURLOPT_URL, "https://mds.datacite.org/metadata" );
	curl_setopt( $p, CURLOPT_HTTPHEADER, $headers );
	curl_setopt( $p, CURLOPT_HEADER, 1 );

	curl_setopt( $p, CURLOPT_POST, 1 );
	curl_setopt( $p, CURLOPT_POSTFIELDS, $xmltext );
	curl_setopt( $p, CURLOPT_RETURNTRANSFER, TRUE );
	$ret = curl_exec( $p );
	$info= curl_getinfo( $p );
	$code = $info['http_code' ];
	curl_close( $p );

#print "$xmltext";
#print "\n\n";
#print_r( $info );
#print "<p>$ret";
	if( $code !== 201 ) { 
		return array( "error" => "Unable to register DOI metadata ( $code )");
	}



	# Step 3 register DOI->Ul mapping
	$maptext = "doi:$doi\n";
	$maptext = $maptext . "url=" . $repo_url . "/resolve/?doi=" .$doi_idx;

	$p = curl_init();
	curl_setopt( $p, CURLOPT_URL, "https://mds.datacite.org/doi" );
	curl_setopt( $p, CURLOPT_HTTPHEADER, $headers );
	curl_setopt( $p, CURLOPT_HEADER, 1 );

	curl_setopt( $p, CURLOPT_POST, 1 );
	curl_setopt( $p, CURLOPT_POSTFIELDS, $maptext );
	curl_setopt( $p, CURLOPT_RETURNTRANSFER, TRUE );
	$ret = curl_exec( $p );
	$info= curl_getinfo( $p );
	$code = $info['http_code' ];
	curl_close( $p );



	if( $code !== 201 ) { 
		return array( "error" => "Unable to register DOI mapping (2)");
	}

	return array( "doi" => $doi_full );	
}

function get_gecos_by_id( $uid ) {
	$uid=(int)$uid;
	$o = false;
	$R = db_query( "SELECT * FROM account WHERE user_id=?", array($uid) );
	if( array_key_exists( 'name', $R ) ) {
		$o = $R['name'];
		if( $o == "" ) { $o = false; }
	}
	return $o;

}

function get_orcid_by_id( $uid ) {
	$uid=(int)$uid;
	$o = false;
	$R = db_query( "SELECT * FROM account WHERE user_id=?", array($uid) );
	if( array_key_exists( 'orcid', $R ) ) {
		$o = $R['orcid'];
		if( $o == "" ) { $o = false; }
	}
	return $o;
}

function make_landing_page( $doi ) {
	global $REPO_options;
	global $smarty;
	$doi=(int)$doi;

	$R = db_query( "SELECT * FROM DOI WHERE doi=?", array($doi) );
	if(!array_key_exists( 'title', $R ) ) {
		header("HTTP/1.0 404 Not Found");
		exit;
	}

	if( strpos( $R['urlmapping'], "http://" ) === 0 
			|| strpos( $R['urlmapping'], "https://" ) === 0 ) {
		header( "Location: " . $R['urlmapping'], true, 302 );
		die();
	}

 $S = db_query( " select a.orcid from account A join doi B  on A.user_id = B.creator where B.doi=?", array($doi) ) ;
 if( $S['orcid'] == "" ) $smarty->assign( "no_orcid_warning", 1 );
 
   
	if ( $R['collection'] === true  ) {
		make_collection_landing_page( $doi );
	}
	else {
		make_entry_landing_page( $doi );
	}
}


function make_collection_landing_page( $doi ) {
	make_entry_landing_page( $doi ) ; # For now they look the same
}


function make_entry_landing_page( $doi ) {
	global $REPO_options;
	$doi=(int)$doi;
	$R = db_query( "SELECT * FROM DOI WHERE doi=?", array($doi) );

	if(!array_key_exists( 'title', $R ) ) {
		header("HTTP/1.0 404 Not Found");
		exit;
	}

	global $smarty;
	$smarty->assign( "embargoed", $R['embargoed'] ? 1 : 0 );
  $smarty->assign( 'embargo_pass', $R['embargo_pass'] );
	$smarty->assign( "suffix", $doi );
	$smarty->assign( "title", $R['title'] );
	$smarty->assign( "doi", $REPO_options['dc_prefix'] . $doi );
	$smarty->assign( "description", $R['description'] );

	$R= db_query( "SELECT * FROM account WHERE user_id = ? ", array( $R['creator'] ) );

	$smarty->assign( "author",  $R['name'] );
	$smarty->assign( "orcid",  $R['orcid'] );
	$smarty->assign( "index",  $doi );

  $smarty->assign( "collaborators", get_selected_users($doi) );
	$R=db_query_all( "SELECT * FROM file WHERE doi=?" , array($doi) );

	$filename=array();
	$size=array();
	$description=array();
	$mime=array();

	$RR=array();
	if( !empty( $R ) ) {
		foreach($R as $Q ) {
			$RR[] = array(
				'filename' => $Q['filename'],
				'description' => $Q['description'],
				'size' => $Q['size'],
				'mimetype' => $Q['mimetype'],
			  'url' => get_repo_url() . "/resolve/?doi=$doi&file=" . $Q['seq'] ."&access=" . $_REQUEST['access'],
			);
		}
		$smarty->assign( "files", $RR );
	}

	$R=db_query_all( "SELECT * FROM associated WHERE doi=? ORDER BY associated" , array($doi) );

	$RR=array();
	if( !empty( $R ) ) {
		foreach($R as $Q ) {
			$RR[] = array(
				'doi' => $Q['associated'],
				'description' => $Q['description'],
			);
		}
		$smarty->assign( "associated", $RR );
	}

	$R=db_query_all( "SELECT * FROM metadata WHERE doi=? ORDER BY key" , array($doi) );

	$RR=array();
	if( !empty( $R ) ) {
		foreach($R as $Q ) {
			$RR[] = array(
				'key' => $Q['key'],
				'value' => $Q['value'],
			);
		}
		$smarty->assign( "metadata", $RR );
	}





	$R = db_query_all( "select * from membership a left join doi b on  a.child=b.doi where a.parent=?" , array( $doi ) );
	$RR=array();
	if( !empty( $R ) ) {
		foreach($R as $Q ) {
			$RR[] = array(
        'index' => $Q['child'],
				'doi' => $REPO_options['dc_prefix'] .$Q['child'],
				'description' => $Q['title'],
        'embargo_pass' => get_embargo_pass( $Q['child'] ) 
			);
		}
		$smarty->assign( "members", $RR );
	}


	$R = db_query_all( "select * from membership a left join doi b on  a.parent=b.doi where a.child=?" , array( $doi ) );

	$RR=array();
	if( !empty( $R ) ) {
		foreach($R as $Q ) {
			$RR[] = array(
        'embargo_pass' => $_REQUEST['access'],
        'index' => $Q['parent'],
				'doi' => $REPO_options['dc_prefix'] . $Q['parent'],
				'description' => $Q['title'],
			);
		}
		$smarty->assign( "memberof", $RR );
	}



	$smarty->display( "landing.tpl" );
	
}


function make_ore( $doi ) {
	global $REPO_options;
	$R = db_query( "SELECT * FROM DOI WHERE doi=?", array($doi) );

	if(!array_key_exists( 'title', $R ) ) {
		header("HTTP/1.0 404 Not Found");
		exit;
	}

	$url = get_repo_url() . $_SERVER['REQUEST_URI'];
	$xml = new SimpleXMLElement('<atom:entry xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#" xmlns:atom="http://www.w3.org/2005/Atom" xmlns:ore="http://www.openarchives.org/ore/terms/" xmlns:oreatom="http://www.openarchives.org/ore/atom/" xmlns:dcterms="http://purl.org/dc/terms/"></atom:entry>');
	$xml->addChild( 'atom:id', $url );
	$r = $xml->addChild( 'atom:link', $url );
	$r->addAttribute( 'rel', 'alternate' );
	$r->AddAttribute( 'href', 'https://doi.org/' . $REPO_options['dc_prefix'] . $doi );
	$r = $xml->addChild( 'atom:link', $url );
	$r->addAttribute( 'rel','http://www.openarchives.org/ore/terms/describes' );
	$r->addAttribute( 'href', $url );

	$r = $xml->addChild( 'atom:link', $url );
	$r->addAttribute( 'rel','self' );
	$r->addAttribute( 'href', $url );
	$r->addAttribute( 'type', 'application/atom+xml' );

	$xml->addChild( 'atom:published', $R['creation_date'] );
	$r = $xml->addChild( 'atom:source' );
	$r = $r->addChild( 'atom:generator', 'Imperial College London HPC Service Data Repository' );
	$r->addAttribute( 'uri', get_repo_url() );


	$xml->addChild( 'atom:title', $R['title'] );
	$r = $xml->addChild( 'atom:category' );
	$r->addAttribute( 'scheme', 'http://Www.dspace.org/objectModel/' );
	$r->addAttribute( 'term', 'DSpaceItem' );
	$r->addAttribute( 'label', 'DSpace Item' );

	$R = db_query_all( "SELECT * FROM FILE WHERE doi=? ORDER BY seq ASC", array($doi) );
	foreach( $R as $A ) {
		$r = $xml->addChild( 'atom:link' );
		$r->addAttribute( 'rel',  'http://www.openarchives.org/ore/terms/aggregates' );
		$r->addAttribute( 'href',  get_repo_url() . '/resolve?doi=' . $doi  ."&file=" . $A['seq']);
		$r->addAttribute( 'title', $A['filename'] );
		$r->addAttribute( 'type', $A['mimetype'] );
		$r->addAttribute( 'length', $A['size'] );
	}

	$s = $xml->addChild( 'oreatom:triples' , NULL, 'oreatom' );
	foreach( $R as $A ) {
		$r = $s->addChild( 'rdf:Description', NULL, 'rdf' );
		$r->addAttribute( 'rdf:about', get_repo_url() . '/resolve?doi=' .$doi . "&file=" . $A['seq'] );
		$r->addChild( 'dcterms:description', 'ORIGINAL' , 'dcterms' );
		$r = $r->addChild( 'rdf:type', NULL, 'rdf' );
		$r->addAttribute( 'rdf:resource', 'http://www.dspace.org/objectModel/DSpaceBitstream' );

	}
	
	header( "Content-Type: text/xml;charset=utf-8");

	print $xml->asXML();
	exit;
}


function return_file( $doi, $file ) {
	$doi = (int)$doi;
	$file= (int)$file;
	$r = db_query( "SELECT * FROM file WHERE doi=? AND seq=?", array( $doi, $file) );

	if( !array_key_exists( 'doi', $r ) ) {
		header("HTTP/1.0 404 Not Found");
		exit;
	}

	$object = $r['object' ];
	$mime   = $r['mimetype' ];

	header( "Content-Type: " . $mime );

	if( 0 === strpos( $object, "file://") ) { 
		$object = substr( $object, 6 );
		passthru( $object );
    header('Content-Description: File Transfer');
    header('Content-Type: '.$mime );
    header('Content-Disposition: attachment; filename="'. $r['filename'].'"');
    header('Expires: 0');
    header('Cache-Control: must-revalidate');
    header('Pragma: public');
    header('Content-Length: ' . $r['size'] );
    readfile($object);
		exit;
	}
	else {
		header( "Location: " .$object );
	}
}


function get_publication_list( $uid ) {
	global $REPO_options;
	$R=db_query_all( "SELECT * FROM doi WHERE collection=false and  creator=? ORDER BY creation_date DESC", array($uid) );

	$rr=array();
	foreach( $R as $r ){
		$doi = $REPO_options['dc_prefix'] .  $r['doi'];
		$rr[]=array(
			'title' => $r['title'],
			'doi' => $doi,
			'url' => "https://doi.org/$doi",
			'creation_date' => $r['creation_date']
		);
	}
  
	return $rr;
}

function get_external_collaborators( $doi ) {
	$R = db_query_all("SELECT * FROM external_collaborator WHERE doi=?", array($doi) );
  $rr=array();
  $i=0;
  foreach( $R as $r ) {
		$rr[] = array( "index"=> $i,"orcid" => $r['orcid'], "name" => $r['name'] );
    $i++;
	}
	return $rr;
}

function map_orcid_to_name( $orcid ) {
	$orcid = preg_replace( "/^https:\/\/orcid.org\//", "", $orcid );
	$orcid = preg_replace( "/^http:\/\/orcid.org\//",  "",$orcid );
	$orcid = preg_replace( "/^https:\/\/www.orcid.org\//", "", $orcid );
	$orcid = preg_replace( "/^http:\/\/www.orcid.org\//", "", $orcid );
  if( !preg_match( "/^[0-9A-Z]{4}-[0-9A-Z]{4}-[0-9A-Z]{4}-[0-9A-Z]{4}$/", $orcid ) ) {
		return NULL;
	}
	exec("curl -s -H 'Accept: application/orcid+json' https://orcid.org/$orcid", $ret, $retval );

	$ret = json_decode(implode($ret), true);

	return $ret["orcid-profile"]["orcid-bio"]["personal-details"]["given-names"]["value"] . " " . $ret["orcid-profile"]["orcid-bio"]["personal-details"]["family-name"]["value"];

}

function set_external_collaborators( $doi ) {
  db_query("DELETE FROM external_collaborator WHERE doi=?", array($doi) );

	foreach( $_REQUEST as $key => $value ) {
		if ( substr($key, 0, 9) === "external-" ) {
			$idx=substr($key, 9, strlen($key) );
			$orcid  = $_REQUEST['external-' . $idx];
			$orcid  = trim($orcid);
      $name = map_orcid_to_name($orcid );
print("[$orcid][$name]\n");
			if( $name !== NULL ) {
				db_query("INSERT INTO external_collaborator ( doi, orcid, name ) VALUES ( ?, ?, ? )", array( $doi, $orcid, $name ) );
			}
		}
	}
}

function get_collaborations_list( $uid ) {
	global $REPO_options;
	$R = db_query_all( "SELECT * FROM collaboration WHERE user_id = ?", array($uid) );
  $rr=array();
  foreach( $R as $r ) {
    $Q = db_query( "SELECT * FROM doi A LEFT JOIN account B on A.creator = B.user_id WHERE A.doi = ? ORDER BY B.name, A.creation_date", array($r['doi'] ) );
    $url_suffix="";
    if( $Q['embargoed'] ) { $url_suffix = "&access=" . $Q['embargo_pass']; }
   $rr[] = array(
		 'doi'   => $REPO_options['dc_prefix'] .  $Q['doi'],
     'title' => $Q['title'],
     'creation_date' => $Q['creation_date'],
     'index' => $Q['doi'],
     'owner' => $Q['name'],
			'url' => "/resolve?doi=". $r['doi'] . $url_suffix,
   );
  }

	return $rr;
}

function get_collection_list( $uid ) {
	global $REPO_options;
	$R=db_query_all( "SELECT * FROM doi WHERE collection=true and  creator=? ORDER BY creation_date DESC", array($uid) );

	$rr=array();
	foreach( $R as $r ){
		$doi = $REPO_options['dc_prefix'] .  $r['doi'];
    $url_suffix="";
    if( $r['embargoed'] ) { $url_suffix = "&access=" . $r['embargo_pass']; }
		$rr[]=array(
			'title' => $r['title'],
			'doi' => $doi,
			'url' => "/resolve?doi=". $r['doi'] . $url_suffix,
			'creation_date' => $r['creation_date'],
      'index' => $r['doi'],
      "embargoed" => $r['embargoed'] ? 1 : 0,
      "embargo_pass" => $r['embargo_pass'],
		);
	}
	return $rr;
}

function get_misc( $uid ) {
	global $REPO_options;
	$R=db_query_all( "SELECT * FROM doi WHERE collection=false and  creator=?  AND urlmapping is NULL  ORDER BY creation_date DESC", array($uid) );

	$rr=array();
	foreach( $R as $r ){
		
		$B=db_query( "SELECT count(*) FROM membership where child=?", array( $r['doi']) );
		if( $B['count'] === 0 ) {
			$doi = $REPO_options['dc_prefix'] .  $r['doi'];
			$rr[]=array(
				'title' => $r['title'],
				'doi' => $doi,
				'url' => "https://doi.org/$doi",
				'creation_date' => $r['creation_date']
			);
		};
	}
	return $rr;
}



function map_url( $user_id ) {
	if(!array_key_exists( 'title', $_REQUEST )) {  return array ( "error" => "title not set" ); }

	if(!array_key_exists( 'url', $_REQUEST )) { return array( "error" => "url not set" ); }
	$title = $_REQUEST['title'];
	$url = $_REQUEST['url'];
	$desc  = "";

	if( strpos( $url, "http://" ) !== 0  && strpos($url, "https://" ) !==0) {
		return array( "error" => "Not a valid URL" );
	}
	if( $title === "" ) { return array( "error" => "'title' not set"); }
	$memberof = -1;
	if( array_key_exists( 'memberof', $_REQUEST ) ) {
		$memberof = (int)($_REQUEST['memberof' ]);
	}

	if( ((int)$memberof) !== -1 ) {
		if( doi_owner( $memberof ) !== $user_id ) {
			return array( "error" => "Not owner $user_id  $memberof " . doi_owner($memberof)  );
		}
	}	

	# Step 1 - insert into DOI table
	$doi_idx = insert_in_doi_table( $user_id, $title, $desc, 0 , $url );

	# Step 1b insert into membership
	if( $memberof !== -1 ) {
		db_query( "INSERT INTO membership ( parent, child ) VALUES (?,?)", array( $memberof, $doi_idx ) );
	}
	# Step 2 - store files



	# Step 3 - register DOI
	$doi =  register_doi( $doi_idx, $user_id, $title );
  $doi['index'] = $doi_idx;
	return $doi;
}


function edit_entry_get( $id, $doi ){
	global $smarty;

	if( doi_owner( $doi ) !== $id ) {
		$smarty->assign("error", "Not owner" );
		$smarty->display("400.tpl");
		die;
	}

	$R=db_query( "SELECT * FROM doi WHERE doi=?", array($doi) );
	$smarty->assign( "title", $R['title'] );
	$smarty->assign( "description", $R['description'] );

	$R=db_query_all( "SELECT * FROM associated WHERE doi=? ORDER BY associated", array($doi) );

	$RR=array();
	$i=0;
	foreach( $R as $A ) {
		$RR[] = array(
		# index, description, doi 	
			'doi' => $A['associated'],
			"description" => $A['description'],
			"index" => $i
		);
		$i=$i+1;
	}

	$smarty->assign( "associated", $RR );

  $smarty->assign( "external", get_external_collaborators($doi ) );
  $owner=-1;
  $r=db_query("SELECT * FROM membership WHERE child=?", array($doi) );
  if(array_key_exists('parent', $r ) ) { $owner=$r['parent']; }

  $rr = get_collections_and_collaborations_by_uid( $id );
  for( $i=0; $i<count($rr); $i++ ) {
   if( $rr[$i]['index'] === $owner ) {
    $rr[$i]['selected']= 'selected';
   } else {
    $rr[$i]['selected']= '';
   }
  }
  $smarty->assign("memberof", $rr );

	$smarty->assign("allusers", get_all_users( $id ) );
  $smarty->assign("selectedusers", get_selected_users( $doi ) );
	$smarty->display( "edit.tpl" );
	die;

}

function get_all_users( $id ) {
	$R = db_query_all( "SELECT * FROM account WHERE user_id != ? ORDER BY name ASC", array( $id ) );
	$RR=array();
	foreach( $R as $A ) {
    if( strlen($A['name']) > 0 ) {
			$RR[] = array(
     	"name" => $A['name'],
    	 "user_id" => $A['user_id']
  	  );
		}
  } 
	return $RR;
}

function get_selected_users( $doi ) {
	$R = db_query_all( "SELECT * FROM collaboration A JOIN account B on A.user_id = B.user_id WHERE A.doi = ?  ORDER BY B.name ASC", array( $doi ) );
	$RR=array();
	foreach( $R as $A ) {
		$RR[] = array(
     "name" => $A['name'],
     "user_id" => $A['user_id'],
     "orcid" => $A['orcid']
    );
  } 

	return $RR;
}



function edit_entry_put( $id, $doi ){
	global $smarty;


  $doi=(int)($doi);

	if( doi_owner( $doi ) !== $id ) {
		$smarty->assign("error", "Not owner" );
		$smarty->display("400.tpl");
		die;
	}

	mark_doi_updated($doi);

	  db_query( "UPDATE doi SET title=?, description=? WHERE doi=?", array($_REQUEST['title'], $_REQUEST['description'], $doi ) );

  # Invalidate the parent too
  $r =db_query( "SELECT * FROM membership WHERE child=?", array($doi) );
  if( $r and array_key_exists( 'parent', $r ) ) { mark_doi_updated($r['parent']); }
  # Update membership
  db_query( "DELETE FROM membership WHERE child=?", array($doi) );

  $memberof = $_REQUEST['memberof'];
  if( $memberof != -1 ) {
		if( (doi_owner( $memberof ) !== $id)  &&  (! is_collaborator( $id, $memberof )) ) {
				return array( "error" => "Not owner $id  $memberof " . doi_owner($memberof)  );
  	}
  	mark_doi_updated( $memberof );
		db_query("INSERT INTO membership (parent, child) VALUES( ?, ?)", array( $memberof, $doi ) ); 
	}

  db_query("DELETE FROM collaboration WHERE doi=?", array($doi) );
  if(array_key_exists( 'selectedUsers', $_REQUEST ) ) {
  	$sel = $_REQUEST['selectedUsers'];
  	foreach( $sel as $A ) {
      db_query( "INSERT INTO collaboration (doi, user_id ) VALUES ( ?, ? )", array($doi, $A) );
    } 
	}

	db_query( "DELETE FROM associated WHERE doi=?", array($doi) );
#	db_query_all( "SELECT * FROM associated WHERE doi=? ORDER BY associated", array($doi) );

	foreach( $_REQUEST as $key => $value ) {
		if ( substr($key, 0, 4) === "doi-" ) {
			$idx=substr($key, 4, strlen($key) );
			$associated  = $_REQUEST['doi-' . $idx];
			$description = $_REQUEST['desc-' . $idx];
			$associated  = trim($associated);
			$description = trim($description);
			if( strlen($associated) && strlen($description) ) {
				db_query("INSERT INTO associated ( doi, associated, description ) VALUES ( ?, ?, ? )", array( $doi, $associated, $description ) );
			}
		}
#		print($key);
#		print("\n");
#		print($value);
#		print("\n");
	}
#	print_r($_REQUEST);
#	die();

#	print("MARK");

	# Store new files
#	print_r($_FILES);
#	print("</br>\n");
#	print_r($_REQUEST);
	foreach( $_FILES as $key => $value ) {
		if ( substr($key, 0, 5) === "file-" ) {
			$f=$_FILES[$key];
			$idx=substr($key, 5, strlen($key) );
      if( is_array( $f['error'] ) ) { # New file[]  upload method
 	     for( $subset = 0 ; $subset < count($f['error']); $subset++ ) {
				if( $f['error'][$subset]===0 ) {
					$size = $f['size'][$subset];
					$name = $f['name'][$subset];
					$tmpname= $f['tmp_name'][$subset];
					$desc = $_REQUEST['filedesc-'.$idx];
					make_metadata( $doi, $tmpname, $name );
					$ret = store_file( $doi, $tmpname, $name, $size, $desc ); 
					if ($ret !== 0 ) {
						return array( "error" => $ret );
					}
				}
				else if ( $f['error'][$subset]===1 ){
						return array( "error" => "Upload of file failed -- max file size is 128MB" );
				}
       }
			}
			else {
				if( $f['error']===0 ) {
					$size = $f['size'];
					$name = $f['name'];
					$tmpname= $f['tmp_name'];
					$desc = $_REQUEST['filedesc-'.$idx];
					make_metadata( $doi, $tmpname, $name );
					$ret = store_file( $doi, $tmpname, $name, $size, $desc ); 
					if ($ret !== 0 ) {
						return array( "error" => $ret );
					}
				}
				else if ( $f['error']===1 ){
						return array( "error" => "Upload of file failed -- max file size is 128MB" );
				}

			}
		}
	}

 
  set_external_collaborators($doi);
#print_r($_REQUEST);
#die; 
	header( "Location: /resolve?doi=" . $doi . "&access=" . get_embargo_pass($doi), true, 302 );
}

function make_metadata( $doi, $tmpname, $name ) {
	$format=strtolower(substr($name, -3 ));
#	print("$format\n");
	if( preg_match( '/^[a-zA-Z][a-zA-Z][a-zA-Z]$/', $format ) ) {
		$cmd="/usr/local/bin/babel -i $format $tmpname -o inchi - 2>/dev/null | /bin/grep InChI | /usr/bin/head -1";
		$inchi = exec($cmd, $ret );
		$cmd="/usr/local/bin/babel -i $format $tmpname -o inchikey - 2>/dev/null | /bin/grep -v Babel |  /usr/bin/head -1";
		$inchikey = exec($cmd , $ret);

		if( strlen($inchi) ) {
			add_metadata($doi, "inchi", $inchi );
		}
		if( strlen($inchikey) ) {
			add_metadata($doi, "inchikey", $inchikey );
		}
	}

}

function add_metadata( $doi, $key, $value ) {
	db_query("INSERT INTO metadata ( doi, key, value ) VALUES ( ? , ? , ? ) ", array($doi, $key, $value ) );
}

function get_embargo_pass( $doi ) {
  global $smarty;
  $col = $doi;
  $r =db_query( "SELECT * FROM doi WHERE doi=?", array($col) );
  if( !$r['collection'] ) {
    # try again, 
    $r =db_query( "SELECT * FROM membership WHERE child=?", array($col) );
    if( !array_key_exists( 'parent', $r ) ) { return ""; } # No parent collection
    $col = $r['parent'];
  }

  $r =db_query( "SELECT * FROM doi WHERE doi=?", array($col) );
  if( $r['collection'] && $r['embargoed'] ) {
    return $r['embargo_pass']; 
	}
	return "";

}

function check_access( $doi, $access, $human_readable ) {
  global $smarty;
  $col = $doi;
#  print( "[" . $doi . "][".$access."]" );
  $r =db_query( "SELECT * FROM doi WHERE doi=?", array($col) );
  if( !$r['collection'] ) {
    # try again, 
    $r =db_query( "SELECT * FROM membership WHERE child=?", array($col) );
    if( !array_key_exists( 'parent', $r ) ) { return 1; } # No parent collection
    $col = $r['parent'];
  }

  $r =db_query( "SELECT * FROM doi WHERE doi=?", array($col) );
  if( $r['collection'] && $r['embargoed'] ) {
   # print( "Collection and embargoed");
    if( $access == "" ) { 
    #  print("No access token");
      $smarty->assign( "destination", "/resolve" );
      $smarty->assign( "doi", $doi );
      $smarty->display("needpassword.tpl");
      die;
    }
    if( $r['embargo_pass'] === $access ) { 
			return 1; 
		}
    else { 
      $smarty->assign( "destination", "/resolve" );
      $smarty->assign( "doi", $doi );
      $smarty->assign( "error", "Incorrect access code" );
      $smarty->display("needpassword.tpl");
      die;
		}
  }
	return 1;
}

function remove_embargo( $uid, $doi ) {
	$doi=(int)($doi);
	if( doi_owner($doi) === $uid ) {
		db_query('UPDATE doi SET embargoed=FALSE and embargo_pass=NULL where doi=?', array($doi));
		mark_doi_updated($doi);
		return true;
	}
	return false;
}

function mark_doi_updated( $doi ) {
 db_query( "UPDATE doi SET updated = TRUE WHERE doi=?", array($doi) );
}

function is_collaborator( $user_id, $doi ) {
 $R=db_query( "SELECT * FROM collaboration WHERE user_id=? AND doi=?", array( $user_id, $doi) );
 if( !$R ) { return false;}
 return array_key_exists( 'doi', $R );

}

function make_access_token( $user_id ) {
	$token = exec( '/usr/bin/xxd -l 80 -p -c 160 /dev/urandom' );
	db_query( "UPDATE account SET access_token = ? WHERE user_id = ?", array( $token, $user_id ) );
	return $token;
}
?>
