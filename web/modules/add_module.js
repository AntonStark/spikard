function add_module () {
    var area = document.getElementById('contextmenu').menuTarget;

    var addModForm = document.createElement('form');
    addModForm.name = "modParam";
    addModForm.enctype="multipart/form-data";
    addModForm.action="add_module.php";
    addModForm.method="POST";

    addModForm.innerHTML = '    <label for="name">Имя нового модуля:</label>\
    <input type="text" name="name" id="name">\
        <br>\
        <label for="jsFile">JS файл:</label>\
    <input type="file" name="jsFile" id="jsFile">\
        <br>\
        <label for="cssFile">CSS файл:</label>\
    <input type="file" name="cssFile" id="cssFile">\
        <br>\
        <input type="submit" value="Создать">';

    area.appendChild(addModForm);
}