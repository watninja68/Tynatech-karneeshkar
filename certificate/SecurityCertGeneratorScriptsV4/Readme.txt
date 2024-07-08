Openssl based scripts for creating Certification Authrority(CA) and issuing certificates for reader (server) and client hosts:
------------------------------------------------------------------------------------------------------------------------------

Revisions
---------
v1. 11-Apr-13: Initial version
v2. 06-May-13: Updated extensions to server and client certificate to set purpose to server authentication and client autentication, respectively

2. System Requirements
----------------------
Host operating system with 
	OpenSSL installed, and
	BASH script execution support
Path to openssl exe need to be set in the PATH enviroment variable.

3. Usage:
---------
Extract contents to suitable folder on the host machine

3.1 CA Initialization:
----------------------
Edit caconfig.cnf if any configuration for CA needs to be changed.
Execute CA initialization command sequence by invoking ./InitRootCA.sh

3.2 Client certificate issual:
-------------------------------
Certificate and key issued by this method can be directly used with LLRP client
Edit samplehost.cnf if any configuration, such as hostname, for client needs to be updated
Execute CreateClientCert.sh by invoking ./CreateClientCert.sh

3.3 Reader certificate issual:
-------------------------------------
Edit samplereader.cnf if any configuration for reader needs to be updated
Execute CreateReaderCert.sh by invoking CreateReaderCert.sh
