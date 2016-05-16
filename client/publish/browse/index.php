<?php

require_once( '../../../lib/repository.inc' );

#print_r( $_REQUEST);
#print_r( $_SERVER);

$headers = apache_request_headers();
$method  = $_SERVER['REQUEST_METHOD'];
$username= $_SERVER['PHP_AUTH_USER'];

$id = get_user_id( $username );

$accept = $headers['Accept'];

if ( $method === "GET"  || $method==="POST") {
		$pubs = get_collection_list( $id );
		$collabs = get_collaborations_list( $id );
		$pubsmisc = get_misc( $id );
    if(strstr( $accept, "text/html" ) ) {
			$smarty->assign('collections', $pubs );
      if(count($collabs)>0 ) {
			  $smarty->assign('collaborations', $collabs );
      }
			$smarty->assign('misc', $pubsmisc );
			$smarty->display('overview.tpl');
		}
		else {
			print( json_encode( array( "collections"=>$pubs, "miscellaneous"=>$pubsmisc, "collaborations" => $collabs ) ) ); 
		}
		die;
}
else {
	header("HTTP/1.1 400 Bad Request" );
	die( "Error: Method $method unsupported\n");
}

?>
