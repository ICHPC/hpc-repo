{literal}
    <SCRIPT language="javascript">

        function addRowFileList(tableID) {
 
            var table = document.getElementById(tableID);
 
            var rowCount = table.rows.length;
            var row = table.insertRow(rowCount);
 
            var cell1 = row.insertCell(0);
            var element1 = document.createElement("input");
            element1.type = "checkbox";
            element1.name="chkbox";
            cell1.appendChild(element1);
 
            var cell2 = row.insertCell(1);
            cell2.innerHTML = "File "+ (rowCount);
 
            var cell3 = row.insertCell(2);
            var ntab = document.createElement("table");
            cell3.appendChild(ntab);
            row = ntab.insertRow(0);
            cell1 = row.insertCell(0);

            var element2 = document.createElement("input");
            element2.multiple="true";
            element2.type = "file";
            element2.name = "file-" + (rowCount-1) + "[]";
            cell1.appendChild(element2);


            cell1 = row.insertCell(1);
            element2 = document.createElement("textarea");
            element2.type = "textarea";
            element2.cols=47;
            element2.rows=1;
            element2.name = "filedesc-" + (rowCount-1);
            cell1.appendChild(element2);
        }
 
        function deleteRowFileList(tableID) {
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
                var chkbox = row.cells[0].childNodes[1];
                if(null != chkbox && true == chkbox.checked) {
                    table.deleteRow(i);
                    rowCount--;
                    i--;
                }

            }

            for(var i=1; i<rowCount; i++) {
                var row = table.rows[i];
                row.cells[1].innerHTML= "File " + i;
            }
            }catch(e) {
                alert(e);
            }
        }


        function addRow(tableID) {
 
            var table = document.getElementById(tableID);
 
            var rowCount = table.rows.length;
            var row = table.insertRow(rowCount);
 
            var cell1 = row.insertCell(0);
            var element1 = document.createElement("input");
            element1.type = "checkbox";
            element1.name="chkbox";
            cell1.appendChild(element1);
 
            var cell2 = row.insertCell(1);
            cell2.innerHTML = "DOI "+ (rowCount);
 
            var cell3 = row.insertCell(2);
            var ntab = document.createElement("table");
						cell3.appendChild(ntab);
						row = ntab.insertRow(0);
						cell1 = row.insertCell(0);

						var element2 = document.createElement("input");
            element2.name = "doi-" + (rowCount-1);
            cell1.appendChild(element2);


						cell1 = row.insertCell(1);
						element2 = document.createElement("textarea");
            element2.type = "textarea";
						element2.cols=47;
						element2.rows=1;
            element2.name = "desc-" + (rowCount-1);
            cell1.appendChild(element2);
 
//<td><input type="file" name="input-0" size="50"></td>
 
        }
 
        function deleteRow(tableID) {
            try {
            var table = document.getElementById(tableID);
            var rowCount = table.rows.length;
 
            for(var i=0; i<rowCount; i++) {
                var row = table.rows[i];
                var chkbox = row.cells[0].childNodes[1];
                console.log( chkbox );
                if(null != chkbox && true == chkbox.checked) {
                    table.deleteRow(i);
                    rowCount--;
                    i--;
                }
                var chkbox = row.cells[0].childNodes[0];
                console.log( chkbox );
                if(null != chkbox && true == chkbox.checked) {
                    table.deleteRow(i);
                    rowCount--;
                    i--;
                }

            }
            for(var i=1; i<rowCount; i++) {
                var row = table.rows[i];
								row.cells[1].innerHTML= "DOI " + i;
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

      <h1>Edit Entry</h1>


<form  enctype="multipart/form-data"  METHOD="POST">

      <h2>Add files</h2>
<table class="MYTABLE" id="filelist">
<tr><th></th><th>Description</th><th>File</th></tr>

<tr><td>
<input type="checkbox" name="chkbox" value="0" >
</td><td>File 1</td>
<td>
<table>
<tr>
<td><input type="file" multiple name="file-0[]" size="50"></td>
<td><textarea name="filedesc-0" cols=47 rows=1></textarea></td>
</tr>
</table>
</td>
</tr>

<!--<tr><td>Job description</td><td><input type="textfield" name="description" size="50"></td></tr></table>-->

</table>
<br>
<input type="button" value="Add file" onclick="addRowFileList('filelist')"/>
<input type="button" value="Delete selected" onclick="deleteRowFileList('filelist')"/>
</br>
</br>


      <h2>Modify Associated DOIs</h2>
<table class="MYTABLE" id="doilist">
<tr><th></th><th>DOI</th><th>Description</th></tr>

{if $associated} 
{section name=s loop=$associated} 
<tr>
<td>
<input type="checkbox" name="chkbox" value="0" >
</td>
<td>DOI {$associated[s].index}</td>
<td>
<table>
	<tr>
		<td><input  name="doi-{$associated[s].index}" value="{$associated[s].doi}"></td>
		<td><textarea name="desc-{$associated[s].index}" cols=47 rows=1>{$associated[s].description}</textarea></td>
	</tr>
</table>
</td>
</tr>
{/section}
{else}

<tr><td>
<input type="checkbox" name="chkbox" value="0" >
</td><td>DOI 1</td>
<td>
<table>
	<tr>
		<td><input  name="doi-0" ></td>
		<td><textarea name="desc-0" cols=47 rows=1></textarea></td>
	</tr>
</table>
</td>
</tr>


{/if}
</table>


<br>
<input type="button" value="Add DOI" onclick="addRow('doilist')"/>
<input type="button" value="Delete selected" onclick="deleteRow('doilist')"/>
</br>
</br>
{if isset($memberof)}
<p>Member of
<select name="memberof">
{section name=s loop=$memberof}
<option value="{$memberof[s].index}" {$memberof[s].selected}>{$memberof[s].description} -- {$memberof[s].doi}</option>
{/section}
</select>
{/if}
</br>

<h3>Collaborators</h3>

<script type="text/javaScript">
function forceselection() {
	var listLeft=document.getElementById('allUsers[]');
	var listRight=document.getElementById('selectedUsers[]');
  for( var i=0; i < listLeft.options.length; i++ ) {
    listLeft.options[i].selected=false;
  }
  for( var i=0; i < listRight.options.length; i++ ) {
    listRight.options[i].selected=true;
  }
}
function moveToRightOrLeft(side){
	var listLeft=document.getElementById('allUsers[]');
	var listRight=document.getElementById('selectedUsers[]');

	if(side==1){
		if(listLeft.options.length==0){
			return false;
		}else{
			var selectedCountry=listLeft.options.selectedIndex;

			move(listRight,listLeft.options[selectedCountry].value,listLeft.options[selectedCountry].text);
			listLeft.remove(selectedCountry);

			if(listLeft.options.length>0){
				listLeft.options[0].selected=true;
			}
		}
	}else if(side==2){
		if(listRight.options.length==0){
			return false;
		}else{
			var selectedCountry=listRight.options.selectedIndex;

			move(listLeft,listRight.options[selectedCountry].value,listRight.options[selectedCountry].text);
			listRight.remove(selectedCountry);

			if(listRight.options.length>0){
				listRight.options[0].selected=true;
			}
		}
	}
}

function move(listBoxTo,optionValue,optionDisplayText){
	var newOption = document.createElement("option"); 
	newOption.value = optionValue; 
	newOption.text = optionDisplayText; 
	listBoxTo.add(newOption, null); 
	return true; 
}
</script>

<div>
<table border="0">
<tr>
<td colspan="2">People</td>
<td></td>
<td colspan="2">Selection </td>
</tr>
<tr>
<td rowspan="3" align="right"><label>
<select name="allUsers[]" size="10" id="allUsers[]" > 
  {section name=s loop=$allusers}
   <option value={$allusers[s].user_id}>{$allusers[s].name}</option>
  {/section}
</select>
</label></td>
<td></td>
<td></td>
<!--<td align="left"></td>
<td align="left"></td>-->
<td rowspan="3" align="left"><select name="selectedUsers[]" size="10" id="selectedUsers[]" multiple>
  {section name=s loop=$selectedusers}
   <option value={$selectedusers[s].user_id} selected>{$selectedusers[s].name}</option>
  {/section}
</select></td>
</tr>
<tr>
<td align="left"></td>
<td align="left"><label>
<input name="btnRight" type="button" id="btnRight" value="&gt;&gt;" onClick="javaScript:moveToRightOrLeft(1);">
</label></td>
</tr>
<tr>
<td align="left"></td>
<td align="left"><label>
<input name="btnLeft" type="button" id="btnLeft" value="&lt;&lt;" onClick="javaScript:moveToRightOrLeft(2);">
</label></td>
</tr>
</table>
</div>
</br>

<input type="Submit" value="Submit"  onClick="javaScript:forceselection();">

</form>



    </div>

{include file="footer.tpl"}

