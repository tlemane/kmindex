# Server deploy

## kmindex-server

For easy integration, *kmindex* is also composed of a server supporting http requests.
The server can be started using the *kmindex-server* binary, see options below.

!!! note "Options"

    ```
    kmindex-server v0.5.2

    DESCRIPTION
      kmindex-server allows to perform queries via POST requests.

    USAGE
      kmindex-server -i/--index <STR> [-a/--address <STR>] [-p/--port <INT>] [-d/--log-directory <STR>]
                     [-t/--threads <INT>] [--verbose <STR>] [-s/--no-stderr] [-h/--help]
                     [--version]

    OPTIONS
      [global] - global parameters
        -i --index         - Index path.
        -a --address       - Address to use (empty string to bind any address)
                                 IPv4: dotted decimal form
                                 IPv6: hexadimal form.
                                 default ->  {127.0.0.1}
        -p --port          - Port to use. {8080}
        -d --log-directory - Directory for daily logging. {kmindex_logs}
        -s --no-stderr     - Disable stderr logging. [⚑]

      [common]
        -t --threads - Max number of parallel connections. {1}
        -h --help    - Show this message and exit. [⚑]
           --version - Show version and exit. [⚑]
           --verbose - Verbosity level [debug|info|warning|error]. {info}
    ```


