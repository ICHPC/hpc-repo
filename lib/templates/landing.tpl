{include file="header.tpl"}
    <div id="hmMain">


<h1>{$title}</h1>

<p class="doi">DOI: <a href="https://doi.org/{$doi}">{$doi}</a> 
{if {$no_orcid_warning}==1 }
<p class="warning">This DOI will not become public until you have registered your ORCID identifier <a href='/publish/register.php'>here</a></p>
{/if}
<a href="https://data.datacite.org/{$doi}">Metadata</a>
</p>
<p class="date">Created: {$creation_date}</p>
{if {$modified_date} }
<p class="date">Last modified: {$modified_date}</p>
{/if}

<p class="author">Author: <a href="https://orcid.org/{$orcid}">{$author}</a></p>

{if isset($collaborators)}
<p>
{section name=s loop=$collaborators}
Co-author: <a href="https://orcid.org/{$collaborators[s].orcid}">{$collaborators[s].name}</a></br>
{/section}
</p>
{/if}

<p class="description">{$description}</p>

{if {$embargoed}==1}
<h3>Embargo</h3>
<p>Access code: {$embargo_pass}</p>
<p><a href="/publish/release/?doi={$index}">Release embargo</a></p>
{/if}

{if $datument}
<h3>Live view</h3>
<iframe name="liveview" src="{$datument}" width="100%" height="600"></iframe>
{/if}
{if $files}
<h3>Files</h3>
<table class="sortable" border="1" width="100%">
<tr><th width="20%">Filename</th><th width="10%">Size</th><th width="30%">Type</th><th width="40%">Description</th></tr>
{section name=s loop=$files}
<tr>
<td>
<a href="{$files[s].url}">{$files[s].filename}</a>
</td>
<td>
{$files[s].size}
</td>
<td>
{$files[s].mimetype}
</td>
<td>
{$files[s].description}
</td>

</tr>
{/section}
</table>
{/if}

{if isset($memberof) || isset($members)}
{/if}

{if isset($memberof)}
<h3>Member of</h3>
<table class="sortable" border="1" width="100%">
<tr><th width="20%">DOI</th><th width="80%">Description</th></tr>
{section name=s loop=$memberof}
<tr>
<td>
<a href="?doi={$memberof[s].index}&access={$memberof[s].embargo_pass}">{$memberof[s].doi}</a>
</td>
<td>
{$memberof[s].description}</a>
</td>
</tr>
{/section}
</table>

{/if}

{if isset($members)}
<h3>Members</h3>
<table class="sortable" border="1" width="100%">
<tr><th width="20%">DOI</th><th width="80%">Description</th></tr>
{section name=s loop=$members}
<tr>
<td>
<a href="?doi={$members[s].index}&access={$members[s].embargo_pass}">{$members[s].doi}</a>
</td>
<td>
{$members[s].description}
</td>
</tr>
{/section}
</table>


{/if}
{if isset($urlmapping)}
<h3>Persistent URLS</h3>
<table class="sortable" border="1" width="100%">
<tr><th width="20%">DOI</th><th width="80%">Description</th></tr>
{section name=s loop=$urlmapping}
<tr>
<td>
<a href="https://doi.org/{$urlmapping[s].doi}">{$urlmapping[s].doi}</a>
</td>
<td>
< a href="$urlmappins[s].url">{$urlmapping[s].description}</a>
</td>
</tr>
{/section}
</table>



{/if}

{if isset($associated)}
<h3>Associated DOIs</h3>
<table class="sortable" border="1" width="100%">
<tr><th width="20%">DOI</th><th width="80%">Description</th></tr>
{section name=s loop=$associated}
<tr>
<td>
<a href="https://doi.org/{$associated[s].doi}">{$associated[s].doi}</a>
</td>
<td>
{$associated[s].description}</a>
</td>
</tr>
{/section}
</table>
{/if}


{if isset($metadata)}
<h3>Metadata</h3>
<table class="sortable" border="1" width="100%">
<tr><th width="20%">Key</th><th width="80%">Value</th></tr>
{section name=s loop=$metadata}
<tr>
<td>
{$metadata[s].key}
</td>
<td>
{$metadata[s].value}
</td>
</tr>
{/section}
</table>
{/if}




<p><a href="/publish/edit?doi={$suffix}&access={$embargo_pass}">Edit</a></p>

</div>

{include file="footer.tpl"}
