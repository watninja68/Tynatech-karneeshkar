[[inputs.syslog]]
  server = "udp://:10514"

[[outputs.elasticsearch]]
  urls = [ "${EL_URL}" ]
  username = "${EL_USERNAME}"
  password = "${EL_PASSWORD}"
  index_name = "{{appname}}"
  health_check_interval = "10s"
  insecure_skip_verify = true
