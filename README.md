### SERVER:

 - ENABLE PORT: netsh advfirewall firewall add rule name="Socket Server" dir=in action=allow protocol=TCP localport=3490
 - DISABLE PORT: netsh advfirewall firewall delete rule name="Socket Server"

