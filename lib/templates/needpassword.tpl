{include file="header.tpl"}
    <div id="hmMain">


<h1>Access code required</h1>

This entry is currently embargoed. Please enter access code to view it.
<p>
<form action="{$destination}" method="get">
<input type="textfield" name="access"> {$error}
<input type="hidden" name="doi" value="{$doi}">
<p>
<input type="submit">
</form>

</div>
{include file="footer.tpl"}
