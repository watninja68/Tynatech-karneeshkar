#Initialize from current directory
#Enable definition for environment variable OPENSSL_FIPS to execute in FIPS mode on system with FIPS compliant OpenSSL build
#export OPENSSL_FIPS=1
export WORKSPACE_DIR=$( cd "$( dirname "$0" )" && pwd )
export CA_KEY_PASSWORD=CA-abcd12345
#Cleanup Certificate Store folder
rm -rf $WORKSPACE_DIR/CA-Certs
#Change directory to CA-Certs and create folders for certificate and key storage in myCA
mkdir -p $WORKSPACE_DIR/CA-Certs
cd $WORKSPACE_DIR/CA-Certs
mkdir -p myCA/signedcerts
mkdir -p myCA/private
cd myCA
#Initialize serial number
echo '01' > serial  && touch index.txt
#Create CA private key and certificate
export OPENSSL_CONF=$WORKSPACE_DIR/caconfig.cnf
echo 'Creating CA key and certificate....'
openssl req -x509 -newkey rsa:2048 -out cacert.pem -outform PEM -days 1825 -passout pass:$CA_KEY_PASSWORD
openssl x509 -in cacert.pem -out cacert.crt
echo 'Test Certificate Authority Initialized. CA certificate saved in cacert.crt. Install it to trusted CA certificate store'
read