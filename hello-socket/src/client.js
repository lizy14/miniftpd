/*
文件名:
描　述:

作　者: 李肇阳, 清华大学软件学院, lizy14@yeah.net
创建于: 2016-10-06

环　境: node.js v4.4.7, Windows 10.0.14393
*/

var PORT = 9876;
var HOST = '127.0.0.1';

var dgram = require('dgram');
var client = dgram.createSocket('udp4');


client.on('message', function(message, remote) {
    console.log("%s, message from %s:%s\t `%s`, length %s",
        new Date().toLocaleTimeString(),
        remote.address, remote.port,
        message.toString(),
        message.length);
});


range(50 + 1).forEach(function(i) {
    var buffer = new Buffer(i.toString());
    client.send(buffer, 0, buffer.length, PORT, HOST);
});


//util
function range(n) {
    var arr = [];
    var i;
    for (i = 0; i < n; i++) {
        arr.push(i);
    }
    return arr;
}
