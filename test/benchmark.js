var fs = require('fs'),
    assert = require('assert'),
    iptrie = require('../index');

var lookup = new iptrie.IPTrie();

fs.readFile(process.argv[2], 'utf-8', function(err, data) {
  var count = 0;
  var lines = data.split(/\n/);
  var start = +(new Date());
  for (var i=0; i<lines.length; i++) {
    var m = /^([^\/]+)\/(\d+)\s+(.*)/.exec(lines[i]);
    if(m) {
      try {
        lookup.add(m[1],parseInt(m[2]),m[3]);
        count++;
      }
      catch(e) {
        console.log(e, m);
      }
    }
  }
  var elapsed = (+(new Date()) - start)/1000.0;
  console.log(count + " entries in " + elapsed + " seconds ");
  console.log(count/elapsed + " add/sec");

  var timelookup = function(ip, cnt) {
    if(!cnt) cnt = 100000;
    var start = +(new Date());
    for(var i=0; i<cnt; i++) lookup.find(ip);
    var elapsed = (+(new Date()) - start)/1000.0;
    console.log(ip + " performance: " + cnt/elapsed + " lookups/sec");
  };
  timelookup('66.225.209.7');
  timelookup('199.15.227.10');
  timelookup('1.2.3.4');
  timelookup('224.0.2.3');
  timelookup('10.0.2.3');
});

