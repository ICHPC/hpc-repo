<?php

require_once( '../../../lib/repository.inc' );

#print_r( $_REQUEST);
#print_r( $_SERVER);

$headers = apache_request_headers();
$method  = $_SERVER['REQUEST_METHOD'];
$username= $_SERVER['PHP_AUTH_USER'];
$doi     = $_REQUEST['doi'];

$id = get_user_id( $username );

$accept = $headers['Accept'];

if ( $method === "POST" ) {
	edit_entry_put( $id, $doi );
	die;
}
else {
	edit_entry_get( $id, $doi );
	die;
#	$smarty->display('edit.tpl');
}
?>
