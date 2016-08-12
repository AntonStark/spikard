function auth() {
    var area = document.getElementById('contextmenu').menuTarget;

    var authForm = document.createElement('form');
    authForm.name = "auth";
    authForm.enctype="text/plain";
    authForm.action="auth.php";
    authForm.method="POST";

    authForm.innerHTML = '    <label for="login">Логин:</label>\
        <input type="text" name="login" id="login">\
        <br>\
        <label for="pass">Пароль:</label>\
    <input type="password" name="pass" id="pass">\
        <br>\
        <input type="submit" value="Войти">';

    area.appendChild(authForm);
}