/*
   文件名:
   描　述:

   作　者: 李肇阳, 清华大学软件学院, lizy14@yeah.net
   创建于: 2016-10-18

   环　境: WSL, Windows 10.0.14393, gcc (Ubuntu 4.8.4-2ubuntu1~14.04.3) 4.8.4
 */


#pragma once

char* anonymous_username = "anonymous";
char* greeting = "220 Hi there! What's your name? \n";
char* anonymous_accepted = "331 A mystery guest. Need password. Perhaps your email address? \n";
char* username_accepted = "331 Username accepted. Need password. \n";
char* need_username_before_password = "501 Need username before password. \n";
char* login_failed = "501 Authencation failed. Wrong password? \n";
char* login_successful = "230 Access granted. Enjoy! \n";
char* syst_hard_coded = "215 UNIX Type: L8\n";
char* type_hard_coded = "200 Type set to I.\n";
char* not_supported = "502 Don't test me too hard. \n";
char* good_bye = "221 Good bye.\n";
char* need_transfer_connection = "425 You have to call PORT or PASV first. \n";
char* begin_transfer = "150 Opening BINARY mode data connection. \n";
char* port_accepted = "200 PORT accepted, ready to connect. \n";
char* transfer_finished = "226 Transfer successful. \n";
char* permission_denied = "550 Permission denined. \n";
