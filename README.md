# DHCP_Server
DHCP server and client simulation that allows each client to connect to the server and request an IP address.

# Compile:

    $ make

# Execute:

    on machine A:
        
        $ ./dhcp_server <port-number>

    on machine B:

        $ ./client <same-port-number>


# Testing:

    Start the server. Enter your gateway IP address, and then
    enter any valid subnet. For each client that connects,
    the server will complete a DHCP request for that client. The
    client will receive an in-order IP address offer, which they accept.
    Server then waits for new clients while any previously connected client
    stays connected until the user enters something. Then that client exits.
    Server will always remain open until the user presses CTRL+Z to terminate
    the process.

    If no IP's are available (we have used them all according to our subnet
    mask), then the server will state "No IP's available" and then shut down.
