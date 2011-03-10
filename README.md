IP address trie implementation.

1. Run `node-waf configure build`.
2. Test `node test/t.js`.


## Synposis

An example:

       var iptrie = require('iptrie'),
           lookup = new iptrie.IPTrie();
       lookup.add("10.0.0.0", 8, 'rfc1918');
       lookup.add("10.80.116.0", 23,  'my little bit of the world');
       var expectnull = lookup.find("1.2.3.4"); # is null
       var otherplace = lookup.find("10.10.2.2"); # not mine
       var myplace    = lookup.find("10.80.117.4"); # mine

### IPTrie.add(ipaddress, prefix_length, value)

Add a route to ipaddress/prefix and attach the provided value to it.

### IPTrie.del(ipaddress, prefix)

Remove the route ipaddress/prefix returning true/false based on success.

### IPTrie.find(ipaddress)

Find the data attached to the route that best fits the provided
ipaddress. This will use "BPM" biggest prefix matching just as typical
routing policies dictate.

## Performance

; `NODE_PATH=lib:. node test/benchmark.js ~/myroutemap.cidr`

        343748 entries in 1.459 seconds 
        235605.20904729265 add/sec
        66.225.209.7 performance: 552486.1878453039 lookups/sec
        199.15.227.10 performance: 591715.9763313609 lookups/sec
        1.2.3.4 performance: 806451.6129032258 lookups/sec
        224.0.2.3 performance: 1041666.6666666666 lookups/sec
        10.0.2.3 performance: 917431.1926605505 lookups/sec

## License

Copyright (c) 2011, OmniTI Computer Consulting, Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.
    * Neither the name OmniTI Computer Consulting, Inc. nor the names
      of its contributors may be used to endorse or promote products
      derived from this software without specific prior written
      permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
