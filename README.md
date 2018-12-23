# UDP-send-recive
This id NetBeans IDE 8.2 project\
\
But it can be compilled under Linux and Windows as is by:\
gcc -o sendmsg_recvmsg mail.c\
\
\
Under Windows you need Cygwin or MSYS2 for compability with POSIX.\
\
Code looks like correct moder way to work with UDP.\
It is IPv4/IPV6 agnostic.\
recvmsg and sendmsg ar optial and fast way to send/recive UPD under Linux (looks lite sendto/recv trasforms to recvmsg/sendmsg inside kernel)\
Moreover, using recvmsg/sendmsg let you sending non-continius range of memory withuot serialisation inside userspace (array of iovec structs).

\
\
Lic is CC0


