"use strict";

var term = document.getElementById("terminal");

term.onmousedown = function (e) {
    if (!e.ctrlKey) {
        return true;
    }

    var coords = getCoords(term);
    var shiftX = e.pageX - coords.left;
    var shiftY = e.pageY - coords.top;

    term.style.position = 'absolute';
    document.body.appendChild(term);
    moveAt(e);

    term.style.zIndex = 1000; // над другими элементами

    function moveAt(e) {
        term.style.left = e.pageX - shiftX + 'px';
        term.style.top = e.pageY - shiftY + 'px';
    }

    document.onmousemove = function (e) {
        moveAt(e);
    };

    term.onmouseup = function () {
        document.onmousemove = null;
        term.onmouseup = null;
    };

    return false;
};

term.ondragstart = function () {
    return false;
};


function getCoords(elem) { // кроме IE8-
    var box = elem.getBoundingClientRect();

    return {
        top: box.top + pageYOffset,
        left: box.left + pageXOffset
    };

}

var xhr = new XMLHttpRequest();
xhr.onreadystatechange = function () {
    if (xhr.readyState == 4)
        document.getElementById('terminal').value += xhr.responseText + "?>:";
    return false;
};

/*var soc = new WebSocket("ws://localhost:8000");
soc.onmessage = function(event) {
    document.getElementById('terminal').value += event.data;
};*/

function termPost(data) {
    xhr.open('POST','http://localhost:8000',true);
    xhr.send(data);
}

var con = document.getElementById('console');
con.onsubmit = function () {
    termPost(terminal.value);
    return false;
};

term.onkeydown = function (e) {
    if (e.keyCode == 13) {
        var lines = term.value.split('\n');
        //parseComand

        termPost(lines.pop());
        //return false;
    }
};