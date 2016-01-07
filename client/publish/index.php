<?php

require_once( '../../lib/repository.inc' );

#print_r( $_REQUEST);
#print_r( $_SERVER);

$headers = apache_request_headers();
$method  = $_SERVER['REQUEST_METHOD'];
$username= $_SERVER['PHP_AUTH_USER'];

$id = get_user_id( $username );

$accept = $headers['Accept'];

if ( $method === "POST" ) {
	$ret=publish( $id, false );
	if( is_array($ret) && array_key_exists( "error", $ret ) ) {
		if(strstr( $accept, "text/html" ) ) {
			$smarty->assign( 'error', $ret['error'] );
			$smarty->display('error.tpl');
			die;
		}
		else {
			header("HTTP/1.1 400 Bad Request" );
      print(json_encode( $ret ) );
			die;
		}
	}
	else {
		if(strstr( $accept, "text/html" ) ) {
			$smarty->assign('doi', $ret['doi'] );
			$smarty->display('success.tpl');
			die;
		}
		else {
      header( "Content-Type: application/json" );
			print ( json_encode( $ret )); #$ret['doi'];
			die;
		}
	}

} else {

	if(strstr( $accept, "text/html" ) ) {
		$rr =get_collections_by_uid( $id );
		if(!empty($rr) ) {
			$smarty->assign( "memberof", $rr );
		}
		$smarty->display('upload.tpl');
	}
	else {
		header("HTTP/1.1 400 Bad Request" );
		die( "Error: GET method unsupported\n");
	}

}
?>
