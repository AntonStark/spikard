<?
//print_r($_FILES);
//print_r($_POST);

//move_uploaded_files($filename_from, $filename_to);
move_uploaded_file($_FILES['jsFile']['tmp_name'], __DIR__ . "/modules/" . $_POST['name'] . ".js");
move_uploaded_file($_FILES['cssFile']['tmp_name'], __DIR__ . "/modules/" . $_POST['name'] . ".css");
if (  file_put_contents(__DIR__ . "/modules/modules.list",
        "\n<li id=\"".$_POST['name']."\" onclick=\"".$_POST['name']."();hideMenu();\">"
        .$_POST['name']."</li> " .$_POST['name'].".js" ." ".$_POST['name'].".css", FILE_APPEND)  )
echo 'ok';
else
echo 'some error';