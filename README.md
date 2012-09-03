FTPUtils
========

FTPUtils è sviluppato come progetto per corso di Reti di Calcolatori tenutosi presso il DTI di Crema durante l'anno scolastico 2011/2012.
Esso è composto da un server e un client FTP che supportano le funzioni: SYST - LIST - PWD - CWD - RETR - STOR - DELE - MKD - RMD - RNM.<br>
Lo studente sviluppatore del progetto è:

* Paolo Stivanin (SSRI in presenza, DM270)

Requisiti
---------

* GCC/Clang	: per compilare i programmi (per Clang **obbligatoria versione >= 3.1**, per GCC consigliata v4.4.0 e maggiori)
* Make      : per lanciare il Makefile
* GNU/Linux	: i software sono stati sviluppati su ambienti GNU/Linux e quindi **NON** compileranno nè su ambienti MS Windows nè su ambienti MAC OSX.

Compilazione
------------
* Clonare il repository<br>
`git clone https://github.com/polslinux/FTPUtils.git`<br>
* entrare nella root del repository appena clonato:<br>
`cd $PWD/FTPUtils`<br>
* infine dare il comando make:<br>
`make`<br>
Verranno quindi creati due files: ftpclient e ftpserver.

Come usare client e server
--------------------------
Attualmente il server è solo iterativo (1 connessione alla volta) ma è in sviluppo anche il server parallelo che userà la chiamata di sistema "fork".<br>
Il server supporta una forma di autenticazione basata su un file di testo contenente il nome utente e lo sha256 (x50000) della password.<br>
Prima di usare il server bisogna quindi **aggiungere il nome utente e lo sha256 della password** (tramite il tool generate_sha256) delle persone a cui si vuole concedere l'accesso FTP scrivendo tali informazioni **nel file /etc/ftputils/auth** nella forma:<br>
USERNAME SHA256_PASSWORD<br>
USERNAME SHA256_PASSWORD<br>
Per avviare il server basta quindi dare il seguente comando:<br>
`./ftpserver <numporta>`<br>
e similarmente per il client:<br>
`./ftpclient <host> <numporta>`

Licenza
-------

Il software è rilasciato sotto due tipi di licenza, una per il client e l'altra per il server.<br>
Il client FTP è rilasciato sotto licenza GPL:<br>
<http://www.gnu.org/licenses/gpl-3.0.html><br>
mentre il server FTP è rilasciato sotto licenza AGPL:<br>
<http://www.gnu.org/licenses/agpl-3.0.html><br>

Sito web
--------

Sito web dello sviluppatore:	<http://www.polslinux.it><br>
Sito web del progetto:			<https://github.com/polslinux/FTPUtils>
