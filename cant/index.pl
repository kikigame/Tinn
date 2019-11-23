#!/usr/bin/perl

# TINN Cooperative Anonymous Nonsense Testing
# ref: play on "tin can"

# This is a (relatively unCommon) Gateway Interface script,
# to connect the "tinn" executable to a web frontend.

use CGI;
use POSIX qw(mkfifo);
use utf8; # parse source in UTF8
use File::stat;

binmode STDOUT, ":encoding(utf8)";

use warnings;
use strict;

sub trim { # Perl supplies only chomp
    for my $str (@_) {
	$str =~ s/^\s*|\s*$//g;
    }
}

# Commms Protocol with executable: half-duplex:
# tinn sends any number of commands, each ending <!END!>
# tinn sends <!OVER!> to indicate that it is listening
# we send exactly one instruction, followed by <!END!>
# repeat until we receive instruction to stop.

my $cgi = CGI->new;
$cgi->charset('UTF-8');


# kill old sessions and clear any dead files
{
    for my $id (0..3) {
	if (-e "/tmp/tinn$id.pid") {
	    my $pid = `cat /tmp/tinn$id.pid`;
	    if ($pid) {
		warn "session $id has pid $pid";
		chomp $pid;
		my $stats = stat("/tmp/tinn$id.pid");
		my $lastTime = $$stats[9];
		if (defined $lastTime) {
		    my $age = time() - $lastTime;
		    warn "Idle time for session $id: $age seconds\n";
		    if ($age > 60 * 20) {
			warn "Killing session $id; pid $pid (timeout)";
			system("/bin/kill", "-9", $pid);
		    }
		}
		my $count = `/bin/ps -u www-data | /bin/grep $pid | /bin/grep tinn | /usr/bin/wc -l`;
		if (0 == $count) {
		    warn "Cleaning session $id (pid $pid dead)";
		    system("/bin/rm","-f","/tmp/tinn$id.pid","/tmp/tinn$id"."Up.fifo","/tmp/tinn$id"."Down.fifo");
		} else {
		    warn "Session $id has $count processes running";
		}
	    }
	    if (-z "/tmp/tinn$id.pid") {
		warn "Cleaning session $id (pid absent)";
		system("/bin/rm","-f","/tmp/tinn$id.pid","/tmp/tinn$id"."Up.fifo","/tmp/tinn$id"."Down.fifo");
	    }
	}
    }
    my $session = $cgi->param("session");
    if (defined $session) {
	$session = 0 + $session;
	`touch /tmp/tinn$session.pid`;
    }
}


if ($cgi->param("ajax") && $cgi->param("session")) {
    print $cgi->header("text/plain");
    # check if session exists. If not, return error.
    # connect to named pipe.
    # pass input from CGI to named pipe
    # return output to browser.
    my $session = $cgi->param("session");
    #    $cgi->param("POSTDATA"); // where it goes if it can't be parsed

    # UNIX domain sockets are bidirectional, but the data isn't buffered after process disconnect
    # So we need a pair of FIFOs.
    # A good reference to sockets in case I need to try it again:
    # https://troydhanson.github.io/network/Unix_domain_sockets.html

    my $inpipe = "/tmp/tinn$session" . "Down.fifo";
    my $outpipe = "/tmp/tinn$session" . "Up.fifo";
    my $in; my $out;

    warn "Connecting to $outpipe for writing";
    open ($out, "+<$outpipe") or die "Lost connection (output)"; # +< is r/w; doesn't block if server isn't reading
    binmode $out, ":encoding(utf8)";

    if ($cgi->param("KEY")) { # usual keystroke
	my $KEY = $cgi->param("KEY");
	my $alt = $cgi->param("Alt");

	if ($alt) { $KEY = chr(ord($KEY) + 320); }
	print $out "$KEY\n<!END!>";
#	print $out "<!OVER!>\n";

	# print "M:This is a message.\n<!END!>\n";
	#    print "L:This is a long message.\nMultiple lines.\n<!END!>";
	# print "Y:This is a yes/no prompt\n<!END!>";
    } elsif ($cgi->param("RES")) { # result
	# print "M:You responded ", $cgi->param("RES"), "<!END!>\n";
	# print "I:What is your input?<!END!>\n";
	# print "S:This is a select prompt\n";
	# print "<!OPT!>1:First option\nIf you choose this option, things happen.\n";
	# print "<!OPT!>2:Second option\nThis option has multi-line help.\nLike this.\n";
	# print "<!OPT!>Y:First option\nIf you choose this option, things happen.\n";
	# print "<!END!>";
	# print "P:**/**;1%25;35%;--;??<!END!>";
	#print "R:";
	#for my $y (0..19) {
	#    print " Test\nDescription here\nmultiline<!>";
	#     for my $x (1..69) {
#		 print "AName\nDescription\nmultiline<!>";
#	     }
	#	}
	print $out $cgi->("RES");
	print $out "<!END!>";
    } elsif ($cgi->param("BEGIN")) {
	print $out "\n";#"<!END!>";
    } elsif ($cgi->param("LINE")) {
	my $line = $cgi->param("LINE");
	warn "Line input: $line";
	print $out "$line\n<!END!>";
    } elsif ($cgi->param("gen")) {
	my $male = 0 + $cgi->param("male");
	my $female = 0 + $cgi->param("female");
	print $out "$male;$female\n<!END!>";
    } else {
	alert("BUG: Unknown param!");
    }
    flush $out;	

    warn "Connecting to $inpipe for reading";
    open ($in, "<$inpipe") or die "Lost connection (input)";
    binmode $in, ":encoding(utf8)";
    print $out "\n";#"<!END!>";
    flush $out;

    my $line = "";
    while ($line = <$in>) {
	trim($line);
	last if $line eq "<!OVER!>";
	print "$line\n";
    }
    close $in;
    close $out;
    exit(0);
}

print $cgi->header({-encoding => "UTF-8"});

sub go() {
    
    # I can't get File::Temp not to clean up the file, so I'll try something else.
    # This does allow one user to grab another user's session, but I'm not too concerned for a test program.
    # (there is no PII to expose)
    my $session = $cgi->param('session');
    unless (defined $session) {
	warn "Trying to get a new session\n";
	for my $id (1..3) {
	    if (! -e "/tmp/tinn$id"."Up.fifo") {
		$session = $id;
		$cgi->param(session => $id);
		warn "New session $id\n";
		warn "ls /tmp: ".`ls -l /tmp`;
		mkfifo("/tmp/tinn$id"."Up.fifo", 0600) or die "Can't create upload fifo: $!";
		mkfifo("/tmp/tinn$id"."Down.fifo", 0600) or die "Can't create download fifo: $!";
		die "File missing!" unless (-e "/tmp/tinn$id"."Up.fifo" && -e "/tmp/tinn$id"."Down.fifo");
		if (fork() == 0) {
		    system("echo $$ > /tmp/tinn$id.pid");
		    exec("./tinn","--fifos=/tmp/tinn$id") or die "Failed to spawn tinn: $!";
		}
		last;
	    }
	}
    }
    unless (defined $session) {
	warn "No free sessions\n";
	print <<"EOF";
<!doctype html>
<html>
<head><title>Tinn Cant</title>
<body>
Sorry, all games are currently in use. Please try back later.
</body></html>
EOF
    exit(0);
    }

    my $protocol = CGI::https() ? "https" : "http";
    
print <<"EOF";
<!doctype html>
<html>
<head><title>Tinn Cant</title>
<meta charset="UTF-8"/>
<script language="javascript">
document.move = 0; // each AJAX call a unique URL
document.ajaxlock = 0;
// based on https://plainjs.com/javascript/ajax/send-ajax-get-and-post-requests-47/
function postAjax(data, success) {
if (document.ajaxlock == 1) return;
document.ajaxlock = 1;
var params = typeof data == 'string' ? data : Object.keys(data).map(
            function(k){ return encodeURIComponent(k) + '=' + encodeURIComponent(data[k]) }
    ).join('&');
params += '&ajax=1&session=$session&move=' + (++document.move)

var xhr = window.XMLHttpRequest ? new XMLHttpRequest() : new ActiveXObject("Microsoft.XMLHTTP");
xhr.open('POST', "$protocol://rjlee.homelinux.org/tinn/");
xhr.onreadystatechange = function() {
  if (xhr.readyState>3 && xhr.status==200) { success(xhr.responseText); }
};
xhr.setRequestHeader('X-Requested-With', 'XMLHttpRequest');
xhr.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded;charset=utf-8');
xhr.send(params);
return xhr;
}
function processResponse(dataStr) {
 document.ajaxlock = 0;
 var data = dataStr.split("<!END!>");
 for (var i =0; i < data.length; ++i) {
  var ins = data[i].trim();
  if (ins.startsWith('M:')) {message(ins.substr(2));}
  else if (ins.startsWith('L:')) {longMsg(ins.substr(2));}
  else if (ins.startsWith('Y:')) {ynPrompt(ins.substr(2));}
  else if (ins.startsWith('I:')) {linePrompt(ins.substr(2));}
  else if (ins.startsWith('G:')) {genderPrompt(ins.substr(2));}
  else if (ins.startsWith('S:')) {selectPrompt(ins.substr(2));}
  else if (ins.startsWith('K:')) {message(ins.substr(2));} // key prompt
  else if (ins.startsWith('R:')) {render(ins.substr(2));}
  else if (ins.startsWith('C:')) {clear();}
  else if (ins.startsWith('P:')) {updatePlayer(ins.substr(2));}
  else if (ins.length == 0) {}
  else alert("BUG: unknown instruction "+ins);
 }
}
function begin() {
postAjax({"BEGIN" : 1}, function(data) { processResponse(data); });
}
function doKey(key) {
postAjax({"KEY" : key, "Alt" : (document.altDown == true ? "T": "")}, function(data) { processResponse(data); });
}
function message(msg) {
var m = document.getElementById('m');
m.textContent += msg + "\\n";
m.scrollTop = m.scrollHeight; // scroll to end (move scroll top down by height)
}
function longMsg(msg) {
alert(msg);
message("--\\n" + msg.trim() + "\\n--\\n");
}
function readin() {
var i = document.getElementById('i');
var key = i.value.trim();
i.value = "";
if (key.length > 0) {
 var q = document.getElementById('q');
 if (q.style.display == 'block') {
  var opts = q.valid;
  if (opts[key[0].toUpperCase()])
    dialogResult(key[0].toUpperCase());
 } else {
  doKey(key[0].toUpperCase());
 }
}
}
function ynPrompt(msg) {
 dialog(msg, {'Y' : "yes", 'N' : "no"}, "")
}
function selectPrompt(dlg) {
 opts = dlg.split("<!OPT!>");
 var msg = opts[0];
 var sel = {};
 var help = {};
 for (var i=1; i < opts.length; ++i) {
  var cPos = opts[i].indexOf(':');
  var key = opts[i].slice(0,cPos);
  var nPos = opts[i].indexOf('\\n');
  var val = opts[i].slice(cPos+1,nPos);
  help[key] = opts[i].substring(nPos).trim();
  sel[key] = val;
 }
 dialog(msg, sel, help);
}
function linePrompt(msg) {
 var res;
 while (!res) {
  res= prompt(msg);
 }
 message(msg + " : " + res);
 postAjax({"LINE" : res}, function(data) { processResponse(data); });
}
function dialog(msg, opts, help) {
 var m = document.getElementById("m");
 m.style.display = 'none';
 m.textContent += msg.trim() + ": ";
 m.scrollTop = m.scrollHeight; // scroll to end (move scroll top down by height)
 
 var q = document.getElementById("q");
 q.style.display = 'block';
 q.innerHTML = "<div style='margin: auto' id='d'></div><div style='margin: auto' id='p'></div>";
 var d = document.getElementById('d');
 d.textContent = msg;
 if (help.length > 0) d.textContent += "\\n\\n" + help;
 
 var p = document.getElementById('p');
 for (key in opts) {
  key = key.toUpperCase();
  var btn = document.createElement("button");
// #1 Workaround for onclick not working with generated script
  btn.setAttribute('onclick','javascript:dialogResult("' + key.trim() + '")');
  btn.textContent = key + ':' + opts[key];
  btn.title = help[key];
  p.appendChild(btn);
  q.valid=opts;
 }
}
function dialogResult(res) {
 var m = document.getElementById("m");
 m.style.display = 'block';
 m.textContent += res + "\\n";
 document.getElementById("q").style.display = 'none';
 postAjax({"KEY" : res}, function(data) { processResponse(data); });
 document.getElementById('i').focus();
}
function genderPrompt(msg) {
 var m = document.getElementById("m");
 m.style.display = 'none';
 m.textContent += "Gender: ";
 m.scrollTop = m.scrollHeight; // scroll to end (move scroll top down by height)

 var q = document.getElementById("q");
 q.style.display = 'block';

 q.innerHTML = "<div style='margin: auto' id='d'></div>" +
  "<form accept-charset='UTF-8' method='post' id='gend'>" +
  "<label for='male' style='width: 6em;'>Male:</label><input type='range' id='male' name='male' value='50'/>" +
  "<br/><label for='female' style='width: 6em;'>Female:</label><input type='range' id='female' name='female' value='50'/>" +
  "<input type='hidden' name='gen' value='gen'/>" +
  "<br/><input type='button' value='Choose' onclick='genderResult()' id='genIn'/></form>";
 var d = document.getElementById('d');
 d.textContent = msg;
 document.getElementById('genIn').focus();
}
function genderResult() {
 var g = new FormData(document.getElementById('gend'));
 var m = document.getElementById("m");
 m.style.display = 'block';
 m.textContent += g.get('male') + ':' + g.get('female') + "\\n";
 document.getElementById("q").style.display = 'none';
 var o = {}; for (let [kk,vv] of g) { o[kk] = vv; } // convert to object
 postAjax(o, function(data) { processResponse(data); });
 document.getElementById('i').focus();
}
function updatePlayer(res) {
 var r = res.split(';');
 var ids=['nm','ss','sf','sd','sm','sa'];
 for (var i =0; i < 6; ++i) {
  document.getElementById(ids[i]).value=r[i];
 }
}
function clear() {
 var b = document.getElementById('b');
 b.innerHTML = '';
}
function render(res) {
 clear();
 var b = document.getElementById('b');
 var x=-1, y=0;
 while (res.length > 0) {
 if (++x == 70) {
 x = 0; b.innerHTML += "\\n"; ++y;
 }
// Grrr... array.substring() splits codepoints, not grapheme clusters.
 var h,c,counter;
 counter=0;
 for (c of res) {
  if (counter++ == 0) h = c;
  else break;
 }
// var c = res[1];
 if (c == " ") c = "\\xa0";
// var h = res[0];
 var next = res.indexOf('<!>');
 var text = res.substring(3, next);
 res = res.substring(next+3);
 var span = document.createElement("span");
 if (h == '*') { span.style= "font-weight: bold; background: #ada; color: black; border: 1px solid #afa; margin: -1px;" }
 span.textContent = c;
 span.title = '(' + x + ',' + y + "): " + text;
 b.appendChild(span);
 }
}
function toggleAlt() {
document.altDown = (!document.altDown);
document.getElementById('t').style = document.altDown ? "background: red;" : "";
document.getElementById('i').focus();
}
</script>
</head>
<body style="height: 100vh;" onload="javascript:begin()"><div style="display: flex; flex-direction: column; height: calc( 100% - 2em ); overflow: hidden">
<p>Please <strong>report</strong> bugs at <a href="https://www.github.com/kikigame/tinn/issues">Github</a>
<!-- http://code.iamkate.com/html-and-css/fixing-browsers-broken-monospace-font-handling/ -->
<div style="white-space: pre; font-family: monospace, monospace; line-height: 1em; font-size: 1em; width: 70ch; height: 20em; border: 4px inset grey; margin: -0.8em auto 2px auto; background: darkgrey; color: green; padding: 2px;" id="b">0123456789012345678901234567890123456789012345678901234567890123456789
0123456789012345678901234567890123456789012345678901234567890123456789
0123456789012345678901234567890123456789012345678901234567890123456789
0123456789012345678901234567890123456789012345678901234567890123456789
0123456789012345678901234567890123456789012345678901234567890123456789
0123456789012345678901234567890123456789012345678901234567890123456789
0123456789012345678901234567890123456789012345678901234567890123456789
0123456789012345678901234567890123456789012345678901234567890123456789
0123456789012345678901234567890123456789012345678901234567890123456789
0123456789012345678901234567890123456789012345678901234567890123456789
0123456789012345678901234567890123456789012345678901234567890123456789
0123456789012345678901234567890123456789012345678901234567890123456789
0123456789012345678901234567890123456789012345678901234567890123456789
0123456789012345678901234567890123456789012345678901234567890123456789
0123456789012345678901234567890123456789012345678901234567890123456789
0123456789012345678901234567890123456789012345678901234567890123456789
0123456789012345678901234567890123456789012345678901234567890123456789
0123456789012345678901234567890123456789012345678901234567890123456789
0123456789012345678901234567890123456789012345678901234567890123456789
0123456789012345678901234567890123456789012345678901234567890123456789
</div>
<div>
    <button title="North" onClick="doKey('W')">W</button>
    <button title="West" onClick="doKey('A')">A</button>
    <button title="South" onClick="doKey('S')">S</button>
    <button title="East" onClick="doKey('D')">D</button>
    <button title="Up" onClick="doKey('&lt;')">&lt;</button>
    <button title="Down" onClick="doKey('&gt;')">&gt;</button>
    <button title="Inventory" onClick="doKey('I')">I</button>
    <button title="Consume" onClick="doKey('C')">C</button>
    <button title="Equip" onClick="doKey('E')">E</button>
    <button title="Use" onClick="doKey('U')">U</button>
    <button title="Pick Up (items)" onClick="doKey(',')">,</button>
    <button title="Leave (items)" onClick="doKey('L')">L</button>
    <button title="View Objectives" onClick="doKey('O')">O</button>
    <button title="Quit" onClick="doKey('Q')">Q</button>
    <button id="t" title="Move into traps?" onclick="toggleAlt()">Force</button>
    <p>Tip: mouseover for detail</p>
</div>
<div style="flex-grow: 1;white-space: pre;overflow-y: scroll; display: none" id="q"></div>
<div style="flex-grow: 1;white-space: pre;overflow-y: scroll;" id="m">
Press any key to begin...
</div>
<form accept-charset="UTF-8" style="display: flex; flex-wrap: wrap-reverse; justify-content: flex-end;">
<label for="i" style="transform: rotate(-25deg)">Input:</label><!-- cant -->
<input type="text" name="input" value="" id="i" autofocus="autofocus" onkeyup="javascript:readin()" onsubmit="return false;"/>
<div id="nm" style="text-align: right; flex-grow: 1"/>
<label for="ss" style="text-align: right; flex-grow: 1">Strength:</label><input type="text" disabled id="ss" size='5'></input>
<label for="sf">Fight:</label><input type="text" disabled id="sf" size='5'></input>
<label for="sd">Dodge:</label><input type="text" disabled id="sd" size='5'></input>
<label for="sm">Damage:</label><input type="text" disabled id="sm" size='5'></input>
<label for="sa">Appearance:</label><input type="text" disabled id="sa" size='5'></input>
</form>
</div>
</body>
</html>
EOF
}

if ($cgi->param("go")) { go() }
else {
print <<'EOF';
<!doctype html>
<html>
<head><title>Tinn Cant</title></head>
<body>
<p><span style="display: inline-block; transform: rotate(-45deg); padding: 2em 0;">Welcome</span> to <em>Tinn Is Not Nethack Collaborative Anonymous Nonsense Testing</em></p>
<p>Games are <strong>limited</strong> and availale on a first-come, first-served basis.</p>
<p>Games may be <strong>terminated</strong> due to inactivity.</p>
<p>This game is in active development and <strong>bugs</strong> are likely.</p>
<p>Please <strong>report</strong> bugs at <a href="https://www.github.com/kikigame/tinn/issues">Github</a>
<form accept-charset="UTF-8">
<input type="hidden" name="go" value="go"/>
<input type="submit" value="Test out Tinn?" autofocus="autofocus"/>
</form>
</body>
</html>
EOF
}
