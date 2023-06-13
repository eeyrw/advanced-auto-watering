#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include "esp_netif.h"
#include <esp_http_server.h>
#include <esp_ota_ops.h>

#include "wifi_manager.h"
#include "http_app.h"
#include "cJSON.h"
/*
extern double eSpO2;
extern double Ebpm;
extern float ir_forWeb;
extern float red_forWeb;
extern uint32_t ir, red;
void handle_getBPM_SpO2() {
	char raw_JSON[1024];
	DynamicJsonDocument doc(1024);

	doc["millis"] = millis();
	doc["BPM"] = Ebpm;
	doc["SpO2"] = eSpO2;
	doc["ir_forGraph"] = ir_forWeb;
	doc["red_forGraph"] = red_forWeb;
	doc["ir"] = ir;
	doc["red"] = red;

	serializeJson(doc, raw_JSON);
	server.send(200, "application/json", raw_JSON);
}

extern uint32_t ESP_getFlashChipId();
void handle_getSystemStatus() {
	char compilationDate[50];
	sprintf(compilationDate, "%s %s", __DATE__, __TIME__);
	char raw_JSON[1024];
	DynamicJsonDocument doc(1024);

	doc["millis"] = millis();
	doc["deviceName"] = DEVICE_NAME;
	doc["STA_IP"] = WiFi.localIP();
	doc["compilationDate"] = compilationDate;

	doc["chipModel"] = ESP.getChipModel();
	doc["chipRevision"] = ESP.getChipRevision();
	doc["cpuFreqMHz"] = ESP.getCpuFreqMHz();
	doc["chipCores"] = ESP.getChipCores();

	doc["heapSizeKiB"] = ESP.getHeapSize() / 1024;
	doc["freeHeapKiB"] = ESP.getFreeHeap() / 1024;

	doc["psramSizeKiB"] = ESP.getPsramSize() / 1024;
	doc["freePsramKiB"] = ESP.getFreePsram() / 1024;

	doc["flashChipId"] = ESP_getFlashChipId();
	doc["flashSpeedMHz"] = ESP.getFlashChipSpeed() / 1000000;
	doc["flashSizeMib"] = ESP.getFlashChipSize() / 1024 / 1024;

	doc["sketchMD5"] = ESP.getSketchMD5();
	doc["sdkVersion"] = ESP.getSdkVersion();

	serializeJson(doc, raw_JSON);
	server.send(200, "application/json", raw_JSON);
}

void handle_setAPConfig() {
	DynamicJsonDocument doc(1024);
	//检查传入参数
	if (server.args() == 0) {
		return server.send(500, "text/plain", "BAD ARGS");
	}
	//反序列化 传入 json
	DeserializationError error = deserializeJson(doc, server.arg("plain").c_str());
	if (error) {
		log_e("反序列化配置文件失败",error.f_str());
		server.send(304, "application/json", "{\"msg\":\"write failed\"}");
		return;
	}

	//修改 配置缓存
	config_json["AP_ssid"] = doc["AP_ssid"];
	//保存
	if (saveConfigFile()) {
		log_e("保存配置文件失败");
		server.send(304, "application/json", "{\"msg\":\"write failed\"}");
		return;
	}
	//成功返回
	server.send(200, "application/json", "{\"msg\":\"successes\"}");
}
void handle_setSTAConfig() {
	DynamicJsonDocument doc(1024);
	//检查传入参数
	if (server.args() == 0) {
		return server.send(500, "text/plain", "BAD ARGS");
	}
	//反序列化 传入 json
	DeserializationError error = deserializeJson(doc, server.arg("plain").c_str());
	if (error) {
		log_e("反序列化配置文件失败",error.f_str());
		server.send(304, "application/json", "{\"msg\":\"write failed\"}");
		return;
	}

	//修改 配置缓存
	config_json["STA_ssid"] = doc["STA_ssid"];
	config_json["STA_passwd"] = doc["STA_passwd"];

	//保存
	if (saveConfigFile()) {
		log_e("保存配置文件失败");
		server.send(304, "application/json", "{\"msg\":\"write failed\"}");
		return;
	}
	//成功返回
	server.send(200, "application/json", "{\"msg\":\"successes\"}");
}
void handle_dirtyHacker() {
	//返回 肮脏的黑客
	server.send(401, "application/json", "{\"msg\":\"Dirty hacker\"}");
}
//获取过滤后的配置信息
void handle_getConfig() {
	String JSON = R""({"AP_ssid":")"";
	JSON += (const char*)config_json["AP_ssid"];
	JSON += R""(","STA_ssid":")"";
	JSON += (const char*)config_json["STA_ssid"];
	JSON += R""("})"";
	server.send(200, "application/json", JSON);
}

void webServer_Task(void* pvParameters) {
	//解决 跨越域 的问题
	server.enableCORS(); //This is the magic
	//绑定web钩子函数
	server.on("/getBPM_SpO2", handle_getBPM_SpO2);
	server.on("/getSystemStatus", handle_getSystemStatus);
	server.on("/setAPConfig", HTTP_POST, handle_setAPConfig);
	server.on("/setSTAConfig", HTTP_POST, handle_setSTAConfig);
	server.on("/getConfig", handle_getConfig);
	//防止直接访问配置文件
	server.on("/config.json", handle_dirtyHacker);
	server.on("/reboot", []() {
		server.send(200, "application/json", "{\"msg\":\"successes\"}");
		vTaskDelay(1000);
		abort();
	});
	server.onNotFound([]() {
		if (!handleFileRead(server.uri())) {
			server.send(404, "text/plain", "FileNotFound");
		}
		});
	//启动web服务
	server.begin();
	log_d("HTTP server started");

	while (1) {
		esp_task_wdt_reset();
		server.handleClient();
	}
}
*/

extern const uint8_t _binary_bootstrap_min_css_start[] asm("_binary_bootstrap_min_css_start");
extern const uint8_t _binary_bootstrap_min_css_end[] asm("_binary_bootstrap_min_css_end");

extern const uint8_t _binary_bootstrap_bundle_min_js_start[] asm("_binary_bootstrap_bundle_min_js_start");
extern const uint8_t _binary_bootstrap_bundle_min_js_end[] asm("_binary_bootstrap_bundle_min_js_end");

extern const uint8_t _binary_chart_min_js_start[] asm("_binary_chart_min_js_start");
extern const uint8_t _binary_chart_min_js_end[] asm("_binary_chart_min_js_end");

extern const uint8_t _binary_my_index_html_start[] asm("_binary_my_index_html_start");
extern const uint8_t _binary_my_index_html_end[] asm("_binary_my_index_html_end");

static esp_err_t my_post_handler(httpd_req_t *req)
{

	esp_err_t ret = ESP_OK;

	// ESP_LOGI(TAG, "POST %s", req->uri);

	// /* POST /connect.json */
	// if(strcmp(req->uri, http_connect_url) == 0){

	// 	/* buffers for the headers */
	// 	size_t ssid_len = 0, password_len = 0;
	// 	char *ssid = NULL, *password = NULL;

	// 	/* len of values provided */
	// 	ssid_len = httpd_req_get_hdr_value_len(req, "X-Custom-ssid");
	// 	password_len = httpd_req_get_hdr_value_len(req, "X-Custom-pwd");

	// 	if(ssid_len && ssid_len <= MAX_SSID_SIZE && password_len && password_len <= MAX_PASSWORD_SIZE){

	// 		/* get the actual value of the headers */
	// 		ssid = malloc(sizeof(char) * (ssid_len + 1));
	// 		password = malloc(sizeof(char) * (password_len + 1));
	// 		httpd_req_get_hdr_value_str(req, "X-Custom-ssid", ssid, ssid_len+1);
	// 		httpd_req_get_hdr_value_str(req, "X-Custom-pwd", password, password_len+1);

	// 		wifi_config_t* config = wifi_manager_get_wifi_sta_config();
	// 		memset(config, 0x00, sizeof(wifi_config_t));
	// 		memcpy(config->sta.ssid, ssid, ssid_len);
	// 		memcpy(config->sta.password, password, password_len);
	// 		ESP_LOGI(TAG, "ssid: %s, password: %s", ssid, password);
	// 		ESP_LOGD(TAG, "http_server_post_handler: wifi_manager_connect_async() call");
	// 		wifi_manager_connect_async();

	// 		/* free memory */
	// 		free(ssid);
	// 		free(password);

	// 		httpd_resp_set_status(req, http_200_hdr);
	// 		httpd_resp_set_type(req, http_content_type_json);
	// 		httpd_resp_set_hdr(req, http_cache_control_hdr, http_cache_control_no_cache);
	// 		httpd_resp_set_hdr(req, http_pragma_hdr, http_pragma_no_cache);
	// 		httpd_resp_send(req, NULL, 0);

	// 	}
	// 	else{
	// 		/* bad request the authentification header is not complete/not the correct format */
	// 		httpd_resp_set_status(req, http_400_hdr);
	// 		httpd_resp_send(req, NULL, 0);
	// 	}

	// }
	// else{

	// 	if(custom_post_httpd_uri_handler == NULL){
	// 		httpd_resp_set_status(req, http_404_hdr);
	// 		httpd_resp_send(req, NULL, 0);
	// 	}
	// 	else{

	// 		/* if there's a hook, run it */
	// 		ret = (*custom_post_httpd_uri_handler)(req);
	// 	}
	// }

	return ret;
}

extern char global_str_ip[];
static esp_err_t get_system_info(cJSON** pInfoJson)
{
	// doc["millis"] = millis();
	// doc["deviceName"] = DEVICE_NAME;
	// doc["STA_IP"] = WiFi.localIP();
	// doc["compilationDate"] = compilationDate;

	// doc["chipModel"] = ESP.getChipModel();
	// doc["chipRevision"] = ESP.getChipRevision();
	// doc["cpuFreqMHz"] = ESP.getCpuFreqMHz();
	// doc["chipCores"] = ESP.getChipCores();

	// doc["heapSizeKiB"] = ESP.getHeapSize() / 1024;
	// doc["freeHeapKiB"] = ESP.getFreeHeap() / 1024;

	// doc["psramSizeKiB"] = ESP.getPsramSize() / 1024;
	// doc["freePsramKiB"] = ESP.getFreePsram() / 1024;

	// doc["flashChipId"] = ESP_getFlashChipId();
	// doc["flashSpeedMHz"] = ESP.getFlashChipSpeed() / 1000000;
	// doc["flashSizeMib"] = ESP.getFlashChipSize() / 1024 / 1024;

	// doc["sketchMD5"] = ESP.getSketchMD5();
	// doc["sdkVersion"] = ESP.getSdkVersion();

	esp_chip_info_t chipInfo;
	esp_chip_info(&chipInfo);
	const esp_app_desc_t* pAppInfo;

	pAppInfo = esp_ota_get_app_description();

	uint32_t flashSize;
	uint32_t flashSpeed;
	esp_flash_get_size(NULL,&flashSize);

	/* 创建一个JSON数据对象(链表头结点) */
	(*pInfoJson) = cJSON_CreateObject();

	/* 添加一条字符串类型的JSON数据(添加一个链表节点) */
	cJSON_AddStringToObject((*pInfoJson), "deviceName", pAppInfo->project_name);
	cJSON_AddStringToObject((*pInfoJson), "STA_IP", global_str_ip);
	cJSON_AddStringToObject((*pInfoJson), "compilationDate", "UNKNOWN");
	cJSON_AddNumberToObject((*pInfoJson), "chipModel", chipInfo.model);
	cJSON_AddNumberToObject((*pInfoJson), "chipRevision", chipInfo.revision);
	cJSON_AddNumberToObject((*pInfoJson), "cpuFreqMHz", 40);
	cJSON_AddNumberToObject((*pInfoJson), "chipCores", chipInfo.cores);
	cJSON_AddNumberToObject((*pInfoJson), "heapSizeKiB", esp_get_free_heap_size()/1024.F);
	cJSON_AddNumberToObject((*pInfoJson), "freeHeapKiB", esp_get_free_heap_size()/1024.F);
	cJSON_AddNumberToObject((*pInfoJson), "psramSizeKiB", 0);
	cJSON_AddNumberToObject((*pInfoJson), "freePsramKiB", 0);
	cJSON_AddStringToObject((*pInfoJson), "flashChipId", "UNKNOWN");
	cJSON_AddNumberToObject((*pInfoJson), "flashSpeedMHz", 0);
	cJSON_AddNumberToObject((*pInfoJson), "flashSizeMib", flashSize/1024/1024);
	cJSON_AddStringToObject((*pInfoJson), "sketchMD5", "UNKNOWN");
	cJSON_AddStringToObject((*pInfoJson), "sdkVersion", pAppInfo->idf_ver);


	return ESP_OK;
}

static esp_err_t my_get_handler(httpd_req_t *req)
{

	// char* host = NULL;
	// size_t buf_len;
	esp_err_t ret = ESP_OK;
	/* our custom page sits at /helloworld in this example */
	if (strcmp(req->uri, "/my_index") == 0 ||
		strcmp(req->uri, "/my_index.html") == 0)
	{
		httpd_resp_set_status(req, "200 OK");
		httpd_resp_set_type(req, "text/html");
		httpd_resp_send(req, (const char *)_binary_my_index_html_start,
						_binary_my_index_html_end - _binary_my_index_html_start);
	}
	else if (strcmp(req->uri, "/js/chart.min.js") == 0)
	{
		httpd_resp_set_status(req, "200 OK");
		httpd_resp_set_type(req, "text/javascript");
		httpd_resp_send(req, (const char *)_binary_chart_min_js_start,
						_binary_chart_min_js_end - _binary_chart_min_js_start);
	}
	else if (strcmp(req->uri, "/js/bootstrap.bundle.min.js") == 0)
	{
		httpd_resp_set_status(req, "200 OK");
		httpd_resp_set_type(req, "text/javascript");
		httpd_resp_send(req, (const char *)_binary_bootstrap_bundle_min_js_start,
						_binary_bootstrap_bundle_min_js_end - _binary_bootstrap_bundle_min_js_start);
	}
	else if (strcmp(req->uri, "/css/bootstrap.min.css") == 0)
	{
		httpd_resp_set_status(req, "200 OK");
		httpd_resp_set_type(req, "text/css");
		httpd_resp_send(req, (const char *)_binary_bootstrap_min_css_start,
						_binary_bootstrap_min_css_end - _binary_bootstrap_min_css_start);
	}
	else if (strcmp(req->uri, "/getSystemStatus") == 0)
	{
		httpd_resp_set_status(req, "200 OK");
		httpd_resp_set_type(req, "application/json");
		cJSON *infoJson = NULL;
		char* rawJsonStr = NULL;
		get_system_info(&infoJson);
		rawJsonStr = cJSON_Print(infoJson);
		httpd_resp_send(req,rawJsonStr,strlen(rawJsonStr));
		cJSON_Delete(infoJson);
		if(rawJsonStr!=NULL)
			free(rawJsonStr);
	}
	else
	{
		/* send a 404 otherwise */
		httpd_resp_send_404(req);
	}

	// ESP_LOGD(TAG, "GET %s", req->uri);

	// /* Get header value string length and allocate memory for length + 1,
	//  * extra byte for null termination */
	// buf_len = httpd_req_get_hdr_value_len(req, "Host") + 1;
	// if (buf_len > 1) {
	// 	host = malloc(buf_len);
	// 	if(httpd_req_get_hdr_value_str(req, "Host", host, buf_len) != ESP_OK){
	// 		/* if something is wrong we just 0 the whole memory */
	// 		memset(host, 0x00, buf_len);
	// 	}
	// }

	// /* determine if Host is from the STA IP address */
	// wifi_manager_lock_sta_ip_string(portMAX_DELAY);
	// bool access_from_sta_ip = host != NULL?strstr(host, wifi_manager_get_sta_ip_string()):false;
	// wifi_manager_unlock_sta_ip_string();

	// if (host != NULL && !strstr(host, DEFAULT_AP_IP) && !access_from_sta_ip) {

	// 	/* Captive Portal functionality */
	// 	/* 302 Redirect to IP of the access point */
	// 	httpd_resp_set_status(req, http_302_hdr);
	// 	httpd_resp_set_hdr(req, http_location_hdr, http_redirect_url);
	// 	httpd_resp_send(req, NULL, 0);

	// }
	// else{

	// 	/* GET /  */
	// 	if(strcmp(req->uri, http_root_url) == 0){
	// 		httpd_resp_set_status(req, http_200_hdr);
	// 		httpd_resp_set_type(req, http_content_type_html);
	// 		httpd_resp_send(req, (char*)index_html_start, index_html_end - index_html_start);
	// 	}
	// 	/* GET /code.js */
	// 	else if(strcmp(req->uri, http_js_url) == 0){
	// 		httpd_resp_set_status(req, http_200_hdr);
	// 		httpd_resp_set_type(req, http_content_type_js);
	// 		httpd_resp_send(req, (char*)code_js_start, code_js_end - code_js_start);
	// 	}
	// 	/* GET /style.css */
	// 	else if(strcmp(req->uri, http_css_url) == 0){
	// 		httpd_resp_set_status(req, http_200_hdr);
	// 		httpd_resp_set_type(req, http_content_type_css);
	// 		httpd_resp_set_hdr(req, http_cache_control_hdr, http_cache_control_cache);
	// 		httpd_resp_send(req, (char*)style_css_start, style_css_end - style_css_start);
	// 	}
	// 	/* GET /ap.json */
	// 	else if(strcmp(req->uri, http_ap_url) == 0){

	// 		/* if we can get the mutex, write the last version of the AP list */
	// 		if(wifi_manager_lock_json_buffer(( TickType_t ) 10)){

	// 			httpd_resp_set_status(req, http_200_hdr);
	// 			httpd_resp_set_type(req, http_content_type_json);
	// 			httpd_resp_set_hdr(req, http_cache_control_hdr, http_cache_control_no_cache);
	// 			httpd_resp_set_hdr(req, http_pragma_hdr, http_pragma_no_cache);
	// 			char* ap_buf = wifi_manager_get_ap_list_json();
	// 			httpd_resp_send(req, ap_buf, strlen(ap_buf));
	// 			wifi_manager_unlock_json_buffer();
	// 		}
	// 		else{
	// 			httpd_resp_set_status(req, http_503_hdr);
	// 			httpd_resp_send(req, NULL, 0);
	// 			ESP_LOGE(TAG, "http_server_netconn_serve: GET /ap.json failed to obtain mutex");
	// 		}

	// 		/* request a wifi scan */
	// 		wifi_manager_scan_async();
	// 	}
	// 	/* GET /status.json */
	// 	else if(strcmp(req->uri, http_status_url) == 0){

	// 		if(wifi_manager_lock_json_buffer(( TickType_t ) 10)){
	// 			char *buff = wifi_manager_get_ip_info_json();
	// 			if(buff){
	// 				httpd_resp_set_status(req, http_200_hdr);
	// 				httpd_resp_set_type(req, http_content_type_json);
	// 				httpd_resp_set_hdr(req, http_cache_control_hdr, http_cache_control_no_cache);
	// 				httpd_resp_set_hdr(req, http_pragma_hdr, http_pragma_no_cache);
	// 				httpd_resp_send(req, buff, strlen(buff));
	// 				wifi_manager_unlock_json_buffer();
	// 			}
	// 			else{
	// 				httpd_resp_set_status(req, http_503_hdr);
	// 				httpd_resp_send(req, NULL, 0);
	// 			}
	// 		}
	// 		else{
	// 			httpd_resp_set_status(req, http_503_hdr);
	// 			httpd_resp_send(req, NULL, 0);
	// 			ESP_LOGE(TAG, "http_server_netconn_serve: GET /status.json failed to obtain mutex");
	// 		}
	// 	}
	// 	else{

	// 		if(custom_get_httpd_uri_handler == NULL){
	// 			httpd_resp_set_status(req, http_404_hdr);
	// 			httpd_resp_send(req, NULL, 0);
	// 		}
	// 		else{

	// 			/* if there's a hook, run it */
	// 			ret = (*custom_get_httpd_uri_handler)(req);
	// 		}
	// 	}

	// }

	// /* memory clean up */
	// if(host != NULL){
	// 	free(host);
	// }

	return ret;
}

void my_http_app_init(void)
{
	http_app_set_handler_hook(HTTP_GET, &my_get_handler);
	http_app_set_handler_hook(HTTP_POST, &my_post_handler);
	// esp_err_t err;

	// if(httpd_handle == NULL){

	// 	httpd_config_t config = HTTPD_DEFAULT_CONFIG();

	// 	/* this is an important option that isn't set up by default.
	// 	 * We could register all URLs one by one, but this would not work while the fake DNS is active */
	// 	config.uri_match_fn = httpd_uri_match_wildcard;
	// 	config.lru_purge_enable = lru_purge_enable;

	// 	/* generate the URLs */
	// 	if(http_root_url == NULL){
	// 		int root_len = strlen(WEBAPP_LOCATION);

	// 		/* all the pages */
	// 		const char page_js[] = "code.js";
	// 		const char page_css[] = "style.css";
	// 		const char page_connect[] = "connect.json";
	// 		const char page_ap[] = "ap.json";
	// 		const char page_status[] = "status.json";

	// 		/* root url, eg "/"   */
	// 		const size_t http_root_url_sz = sizeof(char) * (root_len+1);
	// 		http_root_url = malloc(http_root_url_sz);
	// 		memset(http_root_url, 0x00, http_root_url_sz);
	// 		strcpy(http_root_url, WEBAPP_LOCATION);

	// 		/* redirect url */
	// 		size_t redirect_sz = 22 + root_len + 1; /* strlen(http://255.255.255.255) + strlen("/") + 1 for \0 */
	// 		http_redirect_url = malloc(sizeof(char) * redirect_sz);
	// 		*http_redirect_url = '\0';

	// 		if(root_len == 1){
	// 			snprintf(http_redirect_url, redirect_sz, "http://%s", DEFAULT_AP_IP);
	// 		}
	// 		else{
	// 			snprintf(http_redirect_url, redirect_sz, "http://%s%s", DEFAULT_AP_IP, WEBAPP_LOCATION);
	// 		}

	// 		/* generate the other pages URLs*/
	// 		http_js_url = http_app_generate_url(page_js);
	// 		http_css_url = http_app_generate_url(page_css);
	// 		http_connect_url = http_app_generate_url(page_connect);
	// 		http_ap_url = http_app_generate_url(page_ap);
	// 		http_status_url = http_app_generate_url(page_status);

	// 	}
	// }
}
