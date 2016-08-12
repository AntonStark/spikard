<?

print_r($_POST['login']);
if ($_POST['newUser'])
file_put_contents(__DIR__."/users.list", $_POST['login']." ".$_POST['pass']", FILE_APPEND);
else
echo 'запрос в базу данных...';