#Initialize from current directory
#Enable definition for environment variable OPENSSL_FIPS to execute in FIPS mode on system with FIPS compliant OpenSSL build
#export OPENSSL_FIPS=1
export WORKSPACE_DIR=$( cd "$( dirname "$0" )" && pwd )
export CA_KEY_PASSWORD=CA-abcd12345
export GENERATED_CERT_KEY_PASSWORD=abcd12345
cd $WORKSPACE_DIR/CA-Certs/myCA
echo 'Current dir:'$( cd "$( dirname "$0" )" && pwd )
#Create sample client key and certificate
export OPENSSL_CONF=$WORKSPACE_DIR/samplehost.cnf
echo 'Creating client key and certificate with its signing request ....'
touch temprep.pem
openssl req -newkey rsa:2048 -keyout client_key.pem -keyform PEM -out tempreq.pem -outform PEM -passout pass:$GENERATED_CERT_KEY_PASSWORD
#CA now signs client certificate by processing its certificate sigining request
echo 'CA Signing client certificate....'
export OPENSSL_CONF=$WORKSPACE_DIR/caconfig.cnf
openssl ca -in tempreq.pem -out client_crt.pem -extensions ssl_client_server -passin pass:$CA_KEY_PASSWORD -batch
rm -f tempreq.pem
echo 'Client key, certificate creation and signing completed. Use files client_key.pem and client_crt.pem'
read