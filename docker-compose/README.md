Setup EMQX ECP on DockerCompose

1. run `./emqx_ecp_ctl precheck` to do precheck and prequisite installation
2. run `./emqx_ecp_ctl configure` to configure ECP environments
3. run `./emqx_ecp_ctl start` to run EMQX ECP services
4. run `./emqx_ecp_ctl create-user` to create admin user of EMQX ECP
5. open http://<ip>:8082 in browser

If you need to install ElasticSearch service, please refer to:
https://www.elastic.co/guide/en/elasticsearch/reference/current/docker.html

or use OpenSearch Instead
(telegraf ouputs.OpenSearch refer: https://github.com/influxdata/telegraf/tree/release-1.29/plugins/outputs/opensearch)
https://opensearch.org/docs/latest/install-and-configure/install-opensearch/docker/