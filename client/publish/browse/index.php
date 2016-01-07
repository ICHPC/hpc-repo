<?php

require_once( '../../../lib/repository.inc' );

#print_r( $_REQUEST);
#print_r( $_SERVER);

$headers = apache_request_headers();
$method  = $_SERVER['REQUEST_METHOD'];
$username= $_SERVER['PHP_AUTH_USER'];

$id = get_user_id( $username );

$accept = $headers['Accept'];

if ( $method === "GET" ) {
		$pubs = get_collection_list( $id );
		$pubsmisc = get_misc( $id );
    if(strstr( $accept, "text/html" ) ) {
			$smarty->assign('collections', $pubs );
			$smarty->assign('misc', $pubsmisc );
			$smarty->display('overview.tpl');
		}
		else {
			print( json_encode( array( "collections"=>$pubs, "miscellaneous"=>$pubsmisc ) ) ); 
		}
		die;
}
else {
	header("HTTP/1.1 400 Bad Request" );
	die( "Error: Method unsupported\n");
}

?>
