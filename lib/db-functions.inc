<?php

function db_query($query, array $values)
{
    global $REPO_db;
    try
    {
        $row = NULL;
        $st = $REPO_db->prepare($query);
        if ($st->execute($values)) $row = $st->fetch(PDO::FETCH_ASSOC);
        return $row;
    } catch (PDOException $e)
    {
        $err = "Database error: " . $e->getMessage() .
            ", at " . $e->getLine() . " of " . $e->getFile() . "\n" .
            "doing $query\nwith values:";
        $err .= print_r($values, true);
        #print("<!--<p>$err\n-->");
    }
    return NULL;
}

function db_query_all($query, array $values)
{
    global $REPO_db;
    try
    {
        $row = NULL;
        $st = $REPO_db->prepare($query);
        if ($st->execute($values)) $row = $st->fetchAll(PDO::FETCH_ASSOC);
        return $row;
    } catch (PDOException $e)
    {
        print "Database error: " . $e->getMessage() .
            ", at " . $e->getLine() . " of " . $e->getFile() . "\n" .
            "doing $query\nwith values:";
        print_r($values);
        print "\n";
        die();
    }
    return NULL;
}


?>
