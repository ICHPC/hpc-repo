<?php

require_once( '../../../lib/repository.inc' );

#print_r( $_REQUEST);
#print_r( $_SERVER);

$headers = apache_request_headers();
$method  = $_SERVER['REQUEST_METHOD'];
$username= $_SERVER['PHP_AUTH_USER'];

$id = get_user_id( $username );

$accept = $headers['Accept'];

if ( $method === "POST" ) {
	$ret=publish( $id, true );
	if( array_key_exists( "error", $ret ) ) {
		if(strstr( $accept, "text/html" ) ) {
			$smarty->assign( 'error', $ret['error'] );
			$smarty->display('error.tpl');
			die;
		}
		else {
			header("HTTP/1.1 400 Bad Request" );
      print( json_encode( $ret ) );
      die;
		}
	}
	else {
		if(strstr( $accept, "text/html" ) ) {
			$smarty->assign('doi', $ret['doi'] );
			$smarty->assign('index', $ret['index'] );
			$smarty->assign('embargo_pass', $ret['embargo_pass'] );
			$smarty->display('success.tpl');
			die;
		}
		else {
      print( json_encode( $ret ) );;
			die;
		}
	}

} else {

	if(strstr( $accept, "text/html" ) ) {
		$rr =get_collections_by_uid( $id );
		if(!empty($rr) ) {
			$smarty->assign( "memberof", $rr );
		}
		$smarty->display('addcollection.tpl');
	}
	else {
		header("HTTP/1.1 400 Bad Request" );
		die( "Error: GET method unsupported\n");
	}

}
?>
