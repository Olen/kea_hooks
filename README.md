# kea_hooks

g++ -std=c++11 -I /usr/include/kea -L /usr/lib64 -fpic -shared -o options_to_options.so load_unload.cc pkt4_send.cc version.cc -lkea-dhcpsrv -lkea-dhcp++ -lkea-hooks -lkea-log -lkea-util -lkea-exceptions
