# RESET/INITIAL CONFIG

Reboot all Tuxs (2,3,4).

```bash
sudo reboot
```

Reset system configuration of switch and router via GTK Term (you need to change the cable connection from Switch Cons and Router Cons)

```
/system reset-configuration
```

Connect to all Tuxs via ssh:

```bash
ssh netedu@tuxY2

ssh netedu@tuxY3

ssh netedu@tuxY4
```

Go to root on all Tuxs:

```bash
sudo su
```

# Exp 1

Connect cables 3 and 4:

- TUXY3 E1 no 16
- TUXY4 E1 no 15

TUX Y3:

```bash
sudo su
ifconfig if_e1 172.16.Y0.1/24
```

TUX Y4:

```bash
sudo su
ifconfig if_e1 172.16.Y0.254/24
```

TUX Y3:

```bash
ping 172.16.Y0.254
```

# Exp 2

connect cables:

- TUXY2 E1 no 14

TUX Y2:

```bash
sudo su
ifconfig if_e1 172.16.Y1.1/24
```

connect cables:

- SW CONS - CROSS CONS
- CROSS PC - TUX12 S0

gtk:

```
enter
system reset-configuration

login: admin
enter enter enter

/interface bridge add name=bridgeY0
/interface bridge add name=bridgeY1
/interface bridge print brief (ver que as bridges estão lá)
```

no print aparece um bridge default que temos que remover:

```
/interface bridge port remove [find interface = ether14]
/interface bridge port remove [find interface = ether15]
/interface bridge port remove [find interface = ether16]

/interface bridge port add bridge=bridgeY0 interface = ether15
/interface bridge port add bridge=bridgeY0 interface = ether16
/interface bridge port add bridge=bridgeY1 interface = ether14

/interface bridge port print (brief)
```

TUX Y3:

```bash
ping 172.16.Y0.254 funciona!!!
ping 172.16.Y1.1 nao funciona!!!
```

because they are in different bridges

# Exp 3

connect cables:

- TUXY4 E2 no 17

TUX Y4:

```bash
sysctl net.ipv4.ip_forward=1
sysctl net.ipv4.icmp_echo_ignore_broadcasts=0
```

TUX Y4:

```bash
ifconfig if_e2 172.16.Y1.253/24
```

TUX Y2:

```
/interface bridge port remove [find interface=ether17]
/interface bridge port add bridge=bridge11 interface=ether17
```

TUX Y3:

```bash
route add -net 172.16.Y1.0/24 gw 172.16.Y0.254
```

TUX Y2:

```bash
route add -net 172.16.Y0.0/24 gw 172.16.Y1.253
```

in every TUX:

```bash
route -n
```

TUX Y3 (for testing):

```
ping 172.16.Y0.254
ping 172.16.Y1.253
ping 172.16.Y1.1
```

# Exp 4

Connect **ether1** of the router to **PY.24** and **ether2** of the router to **ether10** on the switch (bridgeY1)

In the Switch Cons:

```bash
# Remove port from default bridge
/interface bridge port remove [find interface=ether10]

# Add port to the corresponding bridges
/interface bridge port add bridge=bridgeY1 interface=ether10
```

In the Router Cons:

```bash
# Setup ether1 interface
/ip address add address=172.16.1.Y1/24 interface=ether1

# Setup ether2 interface
/ip address add address=172.16.Y1.254/24 interface=ether2
```

TUX Y3:

```bash
route add -net 172.16.Y1.0/24 gw 172.16.Y0.254
route add -net 172.16.1.0/24 gw 172.16.Y0.254
```

TUX Y4:

```bash
route add -net 172.16.1.0/24 gw 172.16.Y1.254
```

TUX Y2:

```bash
route add -net 172.16.Y0.0/24 gw 172.16.Y1.253
route add -net 172.16.1.0/24 gw 172.16.Y1.254
```

In the Router Cons:

```bash
/ip route add dst-address=172.16.Y0.0/24 gateway=172.16.Y1.253
```

TUX Y3:

```bash
ping 172.16.Y0.254 
ping 172.16.Y1.1 
ping 172.16.Y1.254
```

TUX Y2:

```bash
sysctl net.ipv4.conf.if_e1.accept_redirects=0
sysctl net.ipv4.conf.all.accept_redirects=0

route add -net 172.16.Y0.0/24 gw 172.16.Y1.254

// ping tux3
ping 172.16.Y0.1

// traceroute tux3
traceroute 172.16.Y0.1 // 3 respostas

// change the routes to use again tux4 as the gateway to subnet instead of Rc
route del -net 172.16.Y0.0/24 gw 172.16.Y1.253
route add -net 172.16.Y0.0/24 gw 172.16.Y1.253
traceroute 172.16.Y0.1 // 2 respostas

// activate the acceptance of icmp redirect
route del -net 172.16.Y0.0/24 gw 172.16.Y1.253
sysctl net.ipv4.conf.if_e1.accept_redirects=1
sysctl net.ipv4.conf.all.accept_redirects=1
traceroute 172.16.Y0.1 // 3 respostas de novo
```

TUX Y3:

```html
// ping ftp server ping 172.16.1.10 funciona!!!!!
```

TUX Y2:

```
 // desativa o firewall
 /ip firewall nat disable 0
```

TUX Y3:

```html
ping 172.16.1.254 nao funciona!!!
```

TUX Y2:

```
 // reativa o firewall
 /ip firewall nat enable 0
```

# Exp 5

Edit `/etc/resolv.conf` (`nano /etc/resolv.conf`) on Tux3, Tux4 and Tux2 by adding this entry:

```bash
services.netlab.fe.up.pt 10.227.20.3
```

In every TUX:

```bash
ping google.com
```

ping do google para verificar se podem ser usados nomes como host

# Exp 6

1. Compile your download application in **tuxY3**
2. In **tuxY3**, run your application with files:

**Files Hosted at the NetLab FTP Server**
files in **anonymous** mode:

- pipe.txt (about 189 Bytes)
- /pub/teste.txt (about 6 Bytes)
- /pub/ubuntu-24.04.2-desktop-amd64.iso (about 6.3 GB)

files in **rcom:rcom** mode:

- /var/ftp/pipe.txt (about 189 Bytes)
- /var/ftp/pub/teste.txt (about 6 Bytes)
- /var/ftp/pub/ubuntu-24.04.2-desktop-amd64.iso (about 6.3 GB)

```bash
./download ftp://ftp.netlab.fe.up.pt/pipe.txt
./download ftp://ftp.netlab.fe.up.pt/pub/pipe.txt
./download ftp://ftp.netlab.fe.up.pt/pub/teste.txt

./download ftp://rcom:rcom@ftp.netlab.fe.up.pt//var/ftp/pipe.txt
./download ftp://rcom:rcom@ftp.netlab.fe.up.pt//var/ftp/pub/teste.txt
./download ftp://rcom:rcom@ftp.netlab.fe.up.pt//var/ftp/pub/ubuntu-24.04.2-desktop-amd64.iso
```

**Public FTP Servers**

- URL 1: ftp://ftp.up.pt/pub/archlinux/archive/iso/arch-0.8-base-i686.iso
- URL 2: ftp://demo:password@test.rebex.net/readme.txt
- URL 3: ftp://anonymous:anonymous@ftp.bit.nl/speedtest/100mb.bin

```bash
./download ftp://ftp.up.pt/pub/archlinux/archive/iso/arch-0.8-base-i686.iso
./download ftp://demo:password@test.rebex.net/readme.txt
./download ftp://anonymous:anonymous@ftp.bit.nl/speedtest/100mb.bin
```

3. Repeat the download in **tuxY3** but now, in the middle of the transfer, start a new
   download in **tuxY2**
