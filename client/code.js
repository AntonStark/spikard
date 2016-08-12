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

term.onkeydown = function (e) {
    if (e.keyCode == 13) {
        //parseComand
        $('#console').submit();
        term.value += "\n?>:";
        return false;
    }
};

$('#console').submit(function () {
    send(document.getElementById('terminal').value);
    return false;
});

function send (data) {
    /*$.ajax({
        data: data,
        url:'http://localhost:8000',
        success: function(data) {
            document.getElementById('result').innerHTML+='result:'+data+'<br>';
        }
    });*/
    var xhr = new XMLHttpRequest();
    xhr.open('POST','http://localhost:8000',true);
    xhr.onreadystatechange = function () {
        if (xhr.readyState == 4)
            document.getElementById('result').innerHTML+='result:'+xhr.readyState+xhr.responseText+'<br>';
    };
    xhr.send(data);
    return false;
}