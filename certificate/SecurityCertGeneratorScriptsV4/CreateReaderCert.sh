#Initialize from current directory
#Enable definition for environment variable OPENSSL_FIPS to execute in FIPS mode on system with FIPS compliant OpenSSL build
#export OPENSSL_FIPS=1
export WORKSPACE_DIR=$( cd "$( dirname "$0" )" && pwd )
export CA_KEY_PASSWORD=CA-abcd12345
export GENERATED_CERT_KEY_PASSWORD=abcd12345
cd $WORKSPACE_DIR/CA-Certs/myCA
#Create sample reader key and certificate
export OPENSSL_CONF=$WORKSPACE_DIR/samplereader.cnf
echo 'Creating reader key and certificate with its signing request ....'
touch tempreq.pem 
openssl req -newkey rsa:2048 -keyout reader_key.pem -keyform PEM -out tempreq.pem -outform PEM -passout pass:$GENERATED_CERT_KEY_PASSWORD
#CA now signs client certificate by processing its certificate sigining request
echo 'CA Signing reader certificate....'
export OPENSSL_CONF=$WORKSPACE_DIR/caconfig.cnf
openssl ca -extensions ssl_client_server -in tempreq.pem -out reader_crt.pem -passin pass:$CA_KEY_PASSWORD -batch
rm -f tempreq.pem
echo 'Exporting reader certificate and key to PKCS#12 format....'
openssl pkcs12 -export -out reader.pfx -inkey reader_key.pem -in reader_crt.pem -certfile cacert.crt -passin pass:$GENERATED_CERT_KEY_PASSWORD -passout pass:$GENERATED_CERT_KEY_PASSWORD
echo 'Reader certificate, key and export to PKCS#12 format (.pfx) completed.'
echo 'Note: PFX protected with password: '$GENERATED_CERT_KEY_PASSWORD
read
