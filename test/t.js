var sys = require('sys'),
    fs = require('fs'),
    assert = require('assert'),
    iptrie = require('iptrie');

var lookup = new iptrie.IPTrie();

var expectations = {
  '1.2.3.4': null,
  '10.120.2.1': 'rfc1918',
  '10.80.117.4': 'my special place',
  '2001:470:0:76::2': 'website',
  '2001:470:0:76::3': 'he',
  '2620:0:0:0::2': null,
  '2620:1f:0:1::1': 'omniti',
  '75.49.14.236': 'boom'
};

fs.readFile("test/test.cidr", 'utf-8', function(err, data) {
  var count = 0;
  var lines = data.split(/\n/);
  for (var i=0; i<lines.length; i++) {
    var m = /^([^\/]+)\/(\d+)\s+(.*)/.exec(lines[i]);
    if(m) {
      try {
        lookup.add(m[1],parseInt(m[2]),m[3]);
        count++;
      }
      catch(e) {
        sys.puts(e,m);
      }
    }
  }
  assert.equal(count, 10, "loaded entries");
  for(var target in expectations) {
    var result = lookup.find(target);
    assert.equal(result, expectations[target],
                 "test "+target + " ["+result+" != "+expectations[target]+"]");
  }
});

