server:
  # http server port
  port: 8082

db:
  host: postgres
  user: "${PG_USERNAME}"
  password: "${PG_PASSWORD}"
  dbname: "${PG_DATABASE_ECP}"
  port: 5432
  autoMigrate: true
  timezone: "Asia/Shanghai"
  logLevel: info
  slowThreshold: "200ms"

logger:
  mode: console
  format: text
  path: /tmp/EMQX-ECP.log
  level: warn
  maxSize: 10
  maxAge: 7
  maxBackups: 3

file:
  root_folder: "assets/files"

# mqtt & cluster configure MQTT broker info for proxy and EMQX v4 management
# mqtt part is for ECP internal network usage
mqtt:
  # addr: MQTT broker address used within ECP internal network
  addr: mqtt:1883
  username: "ecp-mqtt-cloud"
  password: "ecp-mqtt-cloud1!"
  maxReconnectInterval: 3
  connectTimeout: 8
  cleanSession: true
  # useSSL: true if tls/ssl is enabled
  useSSL: false
  # verifyCertificate: true if to verify broker's tls/ssl certificate
  verifyCertificate: false
  # cacertFile: CA certificate file path
  cacertFile: ""
  # certFile: certificate file path
  certFile: ""
  # keyFile: key file path
  keyFile: ""

# cluster part is for external network usage
cluster:
  agent:
    # mqttServer: MQTT broker address used by the external network
    mqttServer: tcp://${MQTT_EXTERNAL_IP}:${MQTT_EXTERNAL_PORT}
    mqttUsername: "ecp-mqtt-agent"
    mqttPassword: "ecp-mqtt-agent2!"

edgeService:
  mode: mixed
  interval:
    health: 10
  export:
    prefix: /tmp
  offlineCycleCount2Alarm: 2
