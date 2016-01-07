<?php

include_once '../../lib/repository.inc';
include_once 'lib/oauth/OAuthStore.php';
include_once 'lib/oauth/OAuthRequester.php';


$proto = "https";
$host  = $_SERVER['HTTP_HOST'];
$uri   = rtrim(dirname($_SERVER['PHP_SELF']), '/\\');

session_start();
$username= $_SERVER['PHP_AUTH_USER'];
$id = get_user_id( $username );
$_SESSION['uid'] =  $id;


if( !array_key_exists( 'code', $_GET ) ) {
    $_SESSION["oauthnonce"] = md5(uniqid(rand(), true));
    $redirect_uri = "$proto://$host$uri/orcid.php?state=" . $_SESSION["oauthnonce"];
    header( 'Location: https://orcid.org/oauth/authorize?client_id=' . $REPO_options['orcid_client_id'] .'&response_type=code&scope=/authenticate&redirect_uri=' . $redirect_uri );
}
else {
//  curl -i -L -H 'Accept: application/json' --data 'client_id=0000-0001-7197-7095&client_secret=2801423d-88b0-4809-b6d8-87eede5ec00c&grant_type=authorization_code&code=MsjXNS' 'https://api.sandbox.orcid.org/oauth/token'
    $req = new HttpRequest( "https://pub.orcid.org/oauth/token", HTTP_METH_POST );
    $req->addHeaders( array("Accept"=> "application/json")  );
    $req->addPostFields ( array(
            "client_id" => $REPO_options['orcid_client_id'],
            "client_secret" => $REPO_options['orcid_client_secret'],
            "grant_type" => "authorization_code",
            "code" => $_GET["code"]
        )  );
    $req->send();
    $req->getResponseBody() ;
    $req->getResponseCode() ;
    $obj = json_decode( $req->getResponseBody() );
    if( $req->getResponseCode() === 200  || $req->getResponseCode() === 302 ) {
        if( $_GET["state"] === $_SESSION[ "oauthnonce" ] ) {
            var_dump( $obj );
            $token = $obj->{"access_token"};
            $orcid = $obj->{"orcid"};
            $name  = $obj->{"name"};
            $uid = $_SESSION["uid"];
            $query = 'UPDATE account SET orcid=?, name=? WHERE user_id=?';
            $results = db_query( $query, array($orcid, $name, $uid) );
            $extra = '/publish';
            header( "Location: $proto://$host/$extra" );
#            echo( "<p>$uid :: $name :: $token :: $orcid" );

        }
        else {
            echo ("Nonce mismatch");
        }


    }
    else {
        echo( "Failed: ". $req->getResponseBody()  );
        echo( "Code: ".  $req->getResponseCode() );
    }
}
