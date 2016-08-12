"use strict";

function makeVHMovable (h) {
    h.onmousedown = function (e) {
        h.style.background = "green";
        var startX = e.pageX;
        var leftElem = h.previousElementSibling;
        var leftWidth = leftElem.offsetWidth;
        var rightElem = h.nextElementSibling;
        var partial = (leftElem.offsetWidth + h.offsetWidth + rightElem.offsetWidth)/h.parentElement.offsetWidth;
        h.parentElement.onmousemove = function (ev) {
            var leftPs = (leftWidth + ev.pageX - startX)/h.parentElement.offsetWidth*100;
            leftElem.style.width = leftPs + '%';
            rightElem.style.width = 99.7*partial - leftPs + '%';
        };
        h.parentElement.onmouseup = function () {
            h.parentElement.onmousemove = null;
            h.onmouseup = null;
            h.style.background = "";
        };
    };
}

function bisectionCol (col) {
    while (!(col.className == 'row' || col.className == 'col'))
        col = col.parentElement;
    if (col.className == 'row') {
        var wrap = document.createElement('div');
        wrap.className = 'col';
        col = col.appendChild(wrap);
        while (col.parentElement.childNodes[0] != col)
            col.appendChild(col.parentElement.childNodes[0]);
    }

    //заменить innerHTML на три блока новых колонок и поместить его в первую новую
    if (col.nextSibling && col.nextSibling.nodeName == "#text")
        col.parentElement.removeChild(col.nextSibling);

    var newCol = document.createElement('div');
    newCol.className = 'col';
    newCol.addEventListener('contextmenu', drawConMenu);
    var handler = document.createElement('div');
    handler.className = 'vertical_handler';
    makeVHMovable(handler);
    var partial = col.offsetWidth/col.parentElement.offsetWidth;
    newCol.style.width = 49.9*partial + '%';
    handler.style.width = 0.2*partial + '%';
    col.style.width = 49.9*partial + '%';

    col.parentElement.insertBefore(handler, col.nextElementSibling);
    col.parentElement.insertBefore(newCol, col.nextElementSibling.nextElementSibling);
    
    var r = Math.floor(Math.random() * (256));
    var g = Math.floor(Math.random() * (256));
    var b = Math.floor(Math.random() * (256));
    newCol.style.background = 'rgba(' + r.toString() +', '+ g.toString() +', '+ b.toString() +', ' + '0.2)';
}

function makeHHMovable (h) {
    h.onmousedown = function (e) {
        h.style.background = "green";
        var startY = e.pageY;
        var topElem = h.previousElementSibling;
        var topHeight = topElem.offsetHeight;
        var bottomElem = h.nextElementSibling;
        var partial = (topElem.offsetHeight + h.offsetHeight + bottomElem.offsetHeight)/h.parentElement.offsetHeight;
        h.parentElement.onmousemove = function (ev) {
            var topPs = (topHeight + ev.pageY - startY)/h.parentElement.offsetHeight*100;
            topElem.style.height = topPs + '%';
            bottomElem.style.height = 99.7*partial - topPs + '%';
        };
        h.parentElement.onmouseup = function () {
            h.parentElement.onmousemove = null;
            h.onmouseup = null;
            h.style.background = "";
        };
    };
}

function bisectionRow (row) {
    while (!(row.className == 'row' || row.className == 'col'))
        row = row.parentElement;
    if (row.className == 'col') {
        var wrap = document.createElement('div');
        wrap.className = 'row';
        row = row.appendChild(wrap);
        while (row.parentElement.childNodes[0] != row)
            row.appendChild(row.parentElement.childNodes[0]);
    }

    if (row.nextSibling && row.nextSibling.nodeName == "#text")
        row.parentElement.removeChild(row.nextSibling);

    var newRow = document.createElement('div');
    newRow.className = 'row';
    newRow.addEventListener('contextmenu', drawConMenu);
    var handler = document.createElement('div');
    handler.className = 'horizontal_handler';
    makeHHMovable(handler);
    var partial = row.offsetHeight/row.parentElement.offsetHeight;
    newRow.style.height = 49.9*partial + '%';
    handler.style.height = 0.2*partial + '%';
    row.style.height = 49.9*partial + '%';

    row.parentElement.insertBefore(handler, row.nextElementSibling);
    row.parentElement.insertBefore(newRow, row.nextElementSibling.nextElementSibling);

    var r = Math.floor(Math.random() * (256));
    var g = Math.floor(Math.random() * (256));
    var b = Math.floor(Math.random() * (256));
    newRow.style.background = 'rgba(' + r.toString() +', '+ g.toString() +', '+ b.toString() +', ' + '0.3)';
}

function isMenuDescendant (src) {
    while (src) {
        if (src.id == 'contextmenu')
            return true;
        else
            src = src.parentElement;
    }
    return false;
}

var coreIFace = {};

function hideMenu () {
    document.getElementById('contextmenu').style.display = '';
}

function drawConMenu (e) {
    var menu = document.getElementById('contextmenu');
    menu.style.display = 'block';
    menu.menuTarget = e.target;

    if (menu.offsetWidth+e.clientX >= document.body.offsetWidth)
        menu.style.left = e.clientX-menu.offsetWidth + 'px';
    else
        menu.style.left = e.clientX + 'px';
    if (menu.offsetHeight+e.clientY >= document.body.offsetHeight)
        menu.style.top = e.clientY-menu.offsetHeight + 'px';
    else
        menu.style.top = e.clientY + 'px';

    menu.onmousedown = function () {
        return false;
    };
    document.onclick = function (e) {
        if (!isMenuDescendant(e.target))
            hideMenu();
        else
            return false;
    };

    e.preventDefault();
}

function getInteface() {
    var xhr = new XMLHttpRequest();
    xhr.onreadystatechange = function () {
        var term_text = document.getElementById('terminal_text');
        if (xhr.readyState == 3)
            term_text.value += "Запрос на обновление интерфейса...\n";
        else if (xhr.readyState == 4) {
            term_text.value += "Ответ получен.\nСписок доступных вызовов:\n";
            var pairs = xhr.responseText.split('\n');
            pairs.forEach(function (elem) {
                var pair = elem.split(':');
                coreIFace[pair[0]] = pair[1];
                term_text.value += pair[0] + '\n';
            });
            xhr.onreadystatechange = function () {
                if (xhr.readyState == 4)
                    term_text.value += xhr.responseText + '\n';
                return false;
            };
        }
    };
    xhr.open('GET', 'http://localhost:8000/interface', true);
    xhr.send();
}

window.onload = function () {
    var getMenu = new XMLHttpRequest();
    getMenu.onreadystatechange = function () {
        if (getMenu.readyState == 4) {
            var modules = getMenu.responseText.split('\n');
            var endLi, liTag, notLi,jsFile, cssFile;
            for (var i = 0, len = modules.length; i < len; ++i)
            {
                endLi = modules[i].indexOf('</li>')+'</li>'.length;
                liTag = modules[i].substring(0, endLi);
                notLi = modules[i].substring(endLi+1).split(' ');
                jsFile = notLi[0];
                cssFile = notLi[1];

                var modsMenu = document.getElementById('contextmenu-mods');
                modsMenu.innerHTML += liTag;

                var script = document.createElement('script');
                script.type = 'text/javascript';
                script.src =  'modules/'+jsFile;
                modsMenu.appendChild(script);

                var link = document.createElement('link');
                link.rel = 'stylesheet';
                link.href = 'modules/'+cssFile;
                modsMenu.appendChild(link);
            }
        }
    };
    getMenu.open('GET', 'modules/modules.list', true);
    getMenu.send();
};