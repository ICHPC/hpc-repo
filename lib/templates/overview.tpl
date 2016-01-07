{include file="header.tpl"}
    <div id="hmMain">


<h1>Overview</h1>

{if isset($collections)}
<h2>My Collections</h2>
<table class="sortable" width="100%" border=1>
<tr>
<th width="25%">Creation date</th><th width="10%">DOI</th><th>Title</th><th width="5%">Embargo</th>
</tr>

	{section name=s loop=$collections}
	<tr>
	<td>{$collections[s].creation_date}</td>
	<td><a href="{$collections[s].url}">{$collections[s].doi}</a></td>
	<td>{$collections[s].title}</td>
  <td>
  {if {$collections[s].embargoed=="1"}}
  <a href="/publish/release?doi={$collections[s].index}">Release</a>
  <!--<a href="/publish/release?collection={$collections[s].embargo_pass}">Password</a>-->
  {/if}
  </td>
	</tr>
	{/section}
</table>
{/if}

{if isset($collaborations) }
<h2>My Collaborations</h2>

<table class="sortable" width="100%" border=1>
<tr>
<th width="25%">Creation date</th><th width="10%">DOI</th><th>Title</th><th>Owner</th>
</tr>

	{section name=s loop=$collaborations}
	<tr>
	<td>{$collaborations[s].creation_date}</td>
	<td><a href="{$collaborations[s].url}">{$collaborations[s].doi}</a></td>
	<td>{$collaborations[s].title}</td>
	<td>{$collaborations[s].owner}</td>
	</tr>
	{/section}
</table>
{/if}


{if isset($misc)}
<h2>Individual Datasets</h2>
<table class="sortable" width="100%" border=1>
<tr>
<th width="25%">Creation date</th><th width="10%">DOI</th><th>Title</th>
</tr>

	{section name=s loop=$misc}
	<tr>
	<td>{$misc[s].creation_date}</td>
	<td><a href="{$misc[s].url}">{$misc[s].doi}</a></td>
	<td>{$misc[s].title}</td>
	</tr>
	{/section}
</table>
{/if}



</div>
{include file="footer.tpl"}
