s|<table\b[^>]*>|<DL>|
/<tr\b[^>]*>/ {
:loop
	      N
	      /<\/tr\b[^>]*>/! b loop
}
s|\[&nbsp;<a href[^>]*>[^<]*</a\b[^>]*>&nbsp;]||
s|<tr\b[^>]*>.*<td\b[^>]*>.*\[<a name="\([^"]*\)">\(.*\)</a[^>]*>].*</td\b[^>]*>.*<td\b[^>]*>\(.*\)</td\b[^>]*>.*</tr\b[^>]*>|<DT>[\2]</DT>\
<DD>\
\\anchor \2\3</DD>|
s|\[<a href="#\([^"]*\)">\([^<]*\)</a>]|\\ref \2 "[\2]"|g
s/[ab]2002/2002/g
s|\(\\anchor [^ ]*\)<sup>+</sup>|\1etal|
s|\(\\ref [^ ]*\)<sup>+</sup>|\1etal|
/<\/table/ i\
</DL>
/<\/table/,$ d
