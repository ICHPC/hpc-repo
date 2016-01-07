<?php
# Redirect to orcid if no orcid/username present
require_once( '../../lib/repository.inc' );

#print_r( $_REQUEST);
#print_r( $_SERVER);

$headers = apache_request_headers();
$method  = $_SERVER['REQUEST_METHOD'];
$username= $_SERVER['PHP_AUTH_USER'];

$id = get_user_id( $username );

$orcid = get_orcid_by_id( $id );

if( $orcid === false ) {
	header( "Location: /oauth2/orcid.php" );
}
else {
	header( "Location: /publish/" );
}

?>
