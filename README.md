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

## IPTrie.add(ipaddress, prefix_length, value)

Add a route to ipaddress/prefix and attach the provided value to it.

## IPTrie.del(ipaddress, prefix)

Remove the route ipaddress/prefix returning true/false based on success.

## IPTrie.find(ipaddress)

Find the data attached to the route that best fits the provided
ipaddress. This will use "BPM" biggest prefix matching just as typical
routing policies dictate.
