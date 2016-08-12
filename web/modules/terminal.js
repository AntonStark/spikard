function terminal () {
    var area = document.getElementById('contextmenu').menuTarget;
    var text = document.createElement('textarea');
    text.id = 'terminal_text';

    xhr = new XMLHttpRequest();
    function termPost(data) {
        xhr.open('POST', 'http://localhost:8000', true);
        xhr.send(data);
    }

    text.addEventListener('keydown', function (ev) {
        if (ev.keyCode == 13) {
            var lines = text.value.split('\n');
            var last = lines.pop();
            if (last.length > 0) {
                var words = last.split(' ');
                words[0] = coreIFace[words[0]];
                var post = "";
                for (var i = 0, len = words.length - 1; i < len; ++i)
                    post += words[i]+' ';
                post += words[words.length -1];
                termPost(post);
            }
        }
    });

    area.appendChild(text);
    getInteface();
}