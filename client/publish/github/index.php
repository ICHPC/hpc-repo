<?php

require_once( '../../../lib/repository.inc' );

#print_r( $_REQUEST);
#print_r( $_SERVER);

$headers = apache_request_headers();
$method  = $_SERVER['REQUEST_METHOD'];
$username= $_SERVER['PHP_AUTH_USER'];

#print ("<-- $username -->" );

$id = get_user_id( $username );

if( array_key_exists("code", $_GET ) && array_key_exists("state", $_GET ) ) {
  github_auth( $id, $_GET['code'], $_GET['state'] );
  
}
else {
	github_register( $id );
}

$method  = $_SERVER['REQUEST_METHOD'];

if ( $method === "GET" ) {
	github_repos_get( $id );
}
else if ( $method == "POST" ) {
	github_repos_set( $id );
}
?>
