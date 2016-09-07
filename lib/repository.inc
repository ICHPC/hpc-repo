<?php

# To shut PHP up
date_default_timezone_set( 'Europe/London');

chdir( $_SERVER['DOCUMENT_ROOT'] . '/../' );

$REPO_options = parse_ini_file( 'configuration.ini' );

if( ! $REPO_options ) {
	print "No configuration file found. Installation incomplete";
	die;
}


require_once( 'lib/smarty/libs/Smarty.class.php' );



function db_connect($dbuser, $dbpassword, $dbhost, $dbport, $dbname)
{
    try
    {
        $dsn = "pgsql:host=$dbhost;port=$dbport;dbname=$dbname";
        #$db = new PDO('pgsql:host=localhost;dbname=uportal;port=5432', $dbuser, $dbpassword);
        $db = new PDO($dsn, $dbuser, $dbpassword);
        $db->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
    } catch (PDOException $e)
    {
        $err = "Database connection error: " . $e->getMessage() .
            ", at " . $e->getLine() . " of " . $e->getFile() . "\n";
        die("$err\n");
    }
    return $db;
}


global $REPO_db;
$REPO_db = db_connect( $REPO_options['db_user'], $REPO_options['db_password'], $REPO_options['db_host'], $REPO_options['db_port'], $REPO_options['db_dbname']);

global $smarty;
$smarty = new Smarty();
$smarty->setTemplateDir( './lib/templates' );
$smarty->setCompileDir( '/tmp' );
$smarty->caching = false;
$smarty->cache_lifetime = 0;
$smarty->compile_check = true;
$smarty->debugging = false;
$smarty->force_compile = true;


require_once( 'lib/db-functions.inc' );
require_once( 'lib/user-functions.inc' );

#print_r( $_SERVER );
#print_r( $_REQUEST );

if(array_key_exists( 'PHP_AUTH_USER', $_SERVER ) ) {
	$username= $_SERVER['PHP_AUTH_USER'];

	$id = get_user_id( $username );

	if( is_service_account($id) ) {
		if( array_key_exists ('authtoken', $_REQUEST) ) {
			$authtoken= $_REQUEST['authtoken'];
			$username = get_username_by_authtoken( $authtoken );

			if( !isset($username) ) { die("Invalid authorisation token"); }

			$id = get_user_id( $username );
			$_SERVER['PHP_AUTH_USER'] = $username;
			#print ("ID [$id]" );
			#print("ASSUMING USERID [$username]");
		}
		else {
			die("Service accounts need to supply an authorisation token");
		}
	}

	if( $id ) {
    $smarty->assign('gecos', get_gecos_by_id( $id )); 
    $smarty->assign('orcid', get_orcid_by_id( $id )); 
	}
}
else {
#	print("<-- NOT AUTHORIZED -->" );
}
    $smarty->assign('version', $REPO_options[ 'version' ]);
    $smarty->assign( 'menulinks', array( 
        array( 'url' => '/publish/browse', 'name' => 'Browse' ), 
        array( 'url' => '/publish/collection', 'name' => 'Add Collection' ), 
        array( 'url' => '/publish/', 'name' => 'Deposit Data' ), 
        array( 'url' => '/publish/url', 'name' => 'Persistent URL ' ), 
        array( 'url' => '/publish/tools', 'name' => 'Tools' ), 
        array( 'url' => '/publish/github', 'name' => 'Github Integration' ), 
        array( 'url' => 'https://wiki.ch.ic.ac.uk/wiki/index.php?title=Rdm:data ', 'name' => 'Help' ), 
      )); 




require_once( 'lib/github-functions.inc' )

?>
