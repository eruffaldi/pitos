# look hardisk 
# required Windows NT

use Win32API::File qw(:ALL);

$nome = shift;
die "Needed File" unless $nome;

open($f,"< $nome") or die "Cannot Open File $nome\n";
binmode($f);

$hf = CreateFile("//./A:",GENERIC_WRITE(),FILE_SHARE_READ()|FILE_SHARE_WRITE(),[],OPEN_EXISTING(),0,[])
or die "Cannot open floppy 0\n";

$n = 0;                                      
while(($k = sysread($f,$buf,512)) != 0) {
        $n += $k;
        print "Written $n\r";

        # copy the specified file
        WriteFile($hf,$buf,length($buf),[],[]);
}

