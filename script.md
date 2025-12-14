# Experiences Script 

## TUXs
(DO IT IN TUX2)

**Instructions:**
1. create and open file `nano setup_from_tux2.sh`
2. paste de code below, save and exit
3. make executable `chmod +x setup_from_tux2.sh`
4. run `./setup_from_tux2.sh <GROUP_NUMBER>`


```
#!/bin/bash

# Usage: ./setup_from_tux2.sh <Y_NUMBER>
# Example: ./setup_from_tux2.sh 1

Y=$1
PASS="alanturing"

if [ -z "$Y" ]; then
  echo "Error: Please provide your Group Y number."
  echo "Usage: ./setup_from_tux2.sh <Y_NUMBER>"
  exit 1
fi

# [cite_start]CALCULATE CONTROL NETWORK IPs [cite: 26]
# If Y=1, TuxY3=13 -> 10.227.20.13
# If Y=1, TuxY4=14 -> 10.227.20.14
IP_TUX3="10.227.20.${Y}3"
IP_TUX4="10.227.20.${Y}4"

echo "--- Configuring Group Y=$Y (Running from Tux${Y}2) ---"
echo "Targeting IPs: Tux3=$IP_TUX3, Tux4=$IP_TUX4"

# ==========================================
# 1. CONFIGURE TUX Y2 (LOCAL EXECUTION)
# ==========================================
echo "Configuring LOCAL MACHINE (Tux${Y}2)..."

# Refresh sudo token locally
echo "$PASS" | sudo -S -v

# [cite_start]IP Configuration [cite: 172]
echo "$PASS" | sudo -S ifconfig if_e1 172.16.${Y}1.1/24 up

# [cite_start]Routes [cite: 384]
echo "$PASS" | sudo -S route add -net 172.16.${Y}0.0/24 gw 172.16.${Y}1.253 2>/dev/null || true
echo "$PASS" | sudo -S route add -net 172.16.1.0/24 gw 172.16.${Y}1.254 2>/dev/null || true

# [cite_start]Accept Redirects (Exp 4 settings) [cite: 434]
echo "$PASS" | sudo -S sysctl -w net.ipv4.conf.if_e1.accept_redirects=0
echo "$PASS" | sudo -S sysctl -w net.ipv4.conf.all.accept_redirects=0

# [cite_start]DNS FIX [cite: 566]
# We append to allow preserving search domains if possible, or just force the nameserver
echo "$PASS" | sudo -S bash -c "echo 'nameserver 10.227.20.3' > /etc/resolv.conf"

echo ">> Tux${Y}2 Configured."


# ==========================================
# 2. CONFIGURE TUX Y4 (REMOTE VIA SSH IP)
# ==========================================
echo "Configuring REMOTE Tux${Y}4 ($IP_TUX4)..."
echo "(Enter 'netedu' password if asked for SSH login)"

# Using IP address instead of hostname to avoid DNS issues
ssh -t netedu@$IP_TUX4 "echo '$PASS' | sudo -S bash -c '
  # [cite_start]IP Configuration [cite: 99, 249]
  ifconfig if_e1 172.16.${Y}0.254/24 up
  ifconfig if_e2 172.16.${Y}1.253/24 up

  # [cite_start]IP Forwarding [cite: 250]
  sysctl -w net.ipv4.ip_forward=1
  sysctl -w net.ipv4.icmp_echo_ignore_broadcasts=0

  # [cite_start]Routes [cite: 383]
  route add -net 172.16.1.0/24 gw 172.16.${Y}1.254 || true

  # [cite_start]DNS FIX [cite: 566]
  echo \"nameserver 10.227.20.3\" > /etc/resolv.conf
'"


# ==========================================
# 3. CONFIGURE TUX Y3 (REMOTE VIA SSH IP)
# ==========================================
echo "Configuring REMOTE Tux${Y}3 ($IP_TUX3)..."
echo "(Enter 'netedu' password if asked for SSH login)"

ssh -t netedu@$IP_TUX3 "echo '$PASS' | sudo -S bash -c '
  # [cite_start]IP Configuration [cite: 98]
  ifconfig if_e1 172.16.${Y}0.1/24 up

  # [cite_start]Routes [cite: 382]
  route add -net 172.16.${Y}1.0/24 gw 172.16.${Y}0.254 || true
  route add -net 172.16.1.0/24 gw 172.16.${Y}0.254 || true

  # [cite_start]DNS FIX [cite: 566]
  echo \"nameserver 10.227.20.3\" > /etc/resolv.conf
'"

echo "--- Configuration Complete ---"


```

## GTKTERM

(Y = 1)
### SWITCH
```
/interface bridge add name=bridge10
/interface bridge add name=bridge11
/interface bridge port remove [find interface=ether10]
/interface bridge port remove [find interface=ether14]
/interface bridge port remove [find interface=ether15]
/interface bridge port remove [find interface=ether16]
/interface bridge port remove [find interface=ether17]
/interface bridge port add bridge=bridge10 interface=ether16
/interface bridge port add bridge=bridge10 interface=ether15
/interface bridge port add bridge=bridge11 interface=ether14
/interface bridge port add bridge=bridge11 interface=ether17
/interface bridge port add bridge=bridge11 interface=ether10
```

### ROUTER

```
/ip address add address=172.16.1.11/24 interface=ether1 
/ip address add address=172.16.11.254/24 interface=ether2 
/ip route add dst-address=172.16.10.0/24 gateway=172.16.11.253 
/ip firewall nat add chain=srcnat action=masquerade out-interface=ether1
```
