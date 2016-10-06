/*
文件名:
描　述:

作　者: 李肇阳, 清华大学软件学院, lizy14@yeah.net
创建于: 2016-10-06

环　境: node.js v4.4.7, Windows 10.0.14393
*/

var util = require('util');

var PORT = 9876;
var HOST = '0.0.0.0';


var dgram = require('dgram');

var server = dgram.createSocket('udp4');

var counter = 0;

server.on('message', function(message, remote) {
    counter++;

    console.log("%s, message #%s from %s:%s\t `%s`, length %s",
        new Date().toLocaleTimeString(),
        counter,
        remote.address, remote.port,
        message.toString(),
        message.length);

    var buffer = new Buffer(util.format("%s %s", counter, message.toString()));

    server.send(buffer, 0, buffer.length, remote.port, remote.address);
});

server.bind(PORT, HOST);
