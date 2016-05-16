<?php

require_once( '../../lib/repository.inc' );
if ( array_key_exists( "doi", $_REQUEST ) ) {
  check_access( $_REQUEST['doi'], $_REQUEST['access'], true );
	if( array_key_exists( "file", $_REQUEST ) ) {
		return_file( $_REQUEST['doi'], $_REQUEST['file'] );
	}
	else {
		make_landing_page( $_REQUEST['doi'] );
	}
}
else if ( array_key_exists( "ore", $_REQUEST ) ) {
  check_access( $_REQUEST['ore'], $_REQUEST['access'], false );
	make_ore( $_REQUEST['ore'] );
}
else {
	header("HTTP/1.0 404 Not Found" );
}

?>
