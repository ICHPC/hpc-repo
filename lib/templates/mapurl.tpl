{literal}
    <SCRIPT language="javascript">
        function addRow(tableID) {
 
            var table = document.getElementById(tableID);
 
            var rowCount = table.rows.length;
            var row = table.insertRow(rowCount);
 
            var cell1 = row.insertCell(0);
            var element1 = document.createElement("input");
            element1.type = "checkbox";
            element1.name="chkbox[]";
            cell1.appendChild(element1);
 
            var cell2 = row.insertCell(1);
            cell2.innerHTML = "File "+ (rowCount-2);
 
            var cell3 = row.insertCell(2);
            var ntab = document.createElement("table");
						cell3.appendChild(ntab);
						row = ntab.insertRow(0);
						cell1 = row.insertCell(0);

						var element2 = document.createElement("input");
            element2.type = "file";
            element2.name = "file-" + (rowCount-3);
            cell1.appendChild(element2);


						cell1 = row.insertCell(1);
						element2 = document.createElement("textarea");
            element2.type = "textarea";
						element2.cols=47;
						element2.rows=1;
            element2.name = "desc-" + (rowCount-3);
            cell1.appendChild(element2);
 
//<td><input type="file" name="input-0" size="50"></td>
 
        }
 
        function deleteRow(tableID) {
            try {
            var table = document.getElementById(tableID);
            var rowCount = table.rows.length;
 
            for(var i=0; i<rowCount; i++) {
                var row = table.rows[i];
                var chkbox = row.cells[0].childNodes[0];
                if(null != chkbox && true == chkbox.checked) {
                    table.deleteRow(i);
                    rowCount--;
                    i--;
                }
 
 
            }
            }catch(e) {
                alert(e);
            }
        }
 
    </SCRIPT>
{/literal}
{include file="header.tpl"}
    <!-- main Col start-->
    <div id="hmMain">

      <h1>Map DOI to URL</h1>


<form  enctype="multipart/form-data"  METHOD="POST">

<table class="MYTABLE" id="urlmapping">

<tr><td>URL</td>
<!--<td><input type="textfield" name="title" size="50"></td>-->
<td><textarea name="url" cols="80" rows="1"></textarea></td>
</tr>


<tr><td>Description</td>
<!--<td><input type="textfield" name="title" size="50"></td>-->
<td><textarea name="title" cols="80" rows="1"></textarea></td>
</tr>


</table>
</br>
<input type="Submit" value="Submit" >

</form>


{if isset($urlmapping)}
<h3>Persistent URLS</h3>
<table class="sortable" border="1" width="100%">
<tr><th width="25%">Creation Date</th><th width="20%">DOI</th><th width="55%">Description</th></tr>
{section name=s loop=$urlmapping}
<tr>
<td>{$urlmapping[s].creation_date}</td>
<td>
<a href="https://doi.org/{$urlmapping[s].doi}">{$urlmapping[s].doi}</a>
</td>
<td>
<a href="{$urlmapping[s].url}">{$urlmapping[s].description}</a>
</td>
</tr>
{/section}
</table>



{/if}


    </div>

{include file="footer.tpl"}

