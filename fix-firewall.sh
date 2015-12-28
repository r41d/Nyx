iptables -I INPUT -i lo -p tcp --tcp-flags ALL RST,ACK -j DROP
