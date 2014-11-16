
Access point with D-LINK DWA-121
================================

IP that will be used for wlan0:  192.168.42.1

1. Adjust /etc/network/interfaces

Change the wlan0 configuration to:

    auto wlan0
    allow-hotplug wlan0
    iface wlan0 inet static
        address 192.168.42.1
        netmask 255.255.255.0
        gateway 192.168.1.1

2. hostapd

Install hostapd

    $ sudo apt-get install hostapd

Since the installed hostapd doesn't work with the dwa-121, replace it:

    $ sudo cp /usr/sbin/hostapd /usr/sbin/hostapd.orig
    $ sudo cp ~/elinux/hostapd.dwa121 /usr/sbin/hostapd
    $ sudo chmod 755 /usr/sbin/hostapd

Configure hostapd:

    $ sudo nano /etc/hostapd/hostapd.conf

Input this in the editor:

    interface=wlan0
    driver=rtl871xdrv
    ssid=RaspberryPi
    channel=1
    wpa=2
    wpa_passphrase=1234
    wpa_key_mgmt=WPA-PSK
    wpa_pairwise=TKIP
    rsn_pairwise=CCMP

To use this configuration, add this line to /etc/default/hostapd:

    DAEMON_CONF="/etc/hostapd/hostapd.conf"

3. DHCP

Install DHCP:

    $ sudo apt-get install isc-dhcp-server

Edit configuration file:

    $ sudo /etc/dhcp/dhcpd.conf

Comment out the lines that start with "option domain-name".

Uncomment the line "authoritative;"

Add this at the bottom:

    subnet 192.168.42.0 netmask 255.255.255.0 {
        range 192.168.42.10 192.168.42.50;
        option broadcast-address 192.168.42.255;
        option routers 192.168.42.1;
        default-lease-time 600;
        max-lease-time 7200;
        option domain-name "local";
        option domain-name-servers 8.8.8.8, 8.8.4.4;
    }

Save the file and exit the editor.

Make the wireless adapter the default for the DHCP request:

    $ sudo nano /etc/default/isc-dhcp-server

Change INTERFACES="" to INTERFACES="wlan0"

4. Restart raspberry

    $ sudo shutdown -r now
