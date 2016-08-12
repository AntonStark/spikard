var term = document.getElementsByClassName('terminal')[0];

term.onmousedown = function (e) {
    if (!e.ctrlKey) {
        return;
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

term.onkeydown = function (e) {
    if (e.keyCode == 13) {
        e.preventDefault();
        //parseComand
        term.value += "\n?>:";
    }
};