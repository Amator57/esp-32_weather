//Не перевірені версії
// ==========================
// 📄 Відправка списку архівів (HTML)
// ==========================
server.on("/logs", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    response->addHeader("Content-Type", "text/html; charset=utf-8");

    Serial.printf("📊 [LOGS] Free heap before: %u bytes\n", ESP.getFreeHeap());

    response->print(F("<h2>Доступні архіви:</h2><ul>"));

    File root = SPIFFS.open("/log");
    if (!root || !root.isDirectory()) {
        response->print(F("<li>Немає доступних лог-файлів.</li>"));
    } else {
        File file = root.openNextFile();
        while (file) {
            String fullPath = file.name();
            if (fullPath.endsWith(".csv")) {
                String fileName = fullPath.substring(fullPath.lastIndexOf('/') + 1);
                String datePart = fileName;
                datePart.replace(".csv", "");
                response->print("<li>");
                response->print("<a href=\"/log_viewer?date=" + datePart + "\">📊 " + datePart + "</a> ");
                response->print("(<a href=\"/log/" + fileName + "\">csv</a>)");
                response->print("</li>");
            }
            file = root.openNextFile();
        }
    }

    response->print(F("</ul><p><a href=\"/\">На головну</a></p>"));
    request->send(response);
    Serial.printf("📊 [LOGS] Free heap after: %u bytes\n", ESP.getFreeHeap());
});
			  

														 
						
																			
								
																	

// ==========================
// 📈 JSON для архівних графіків
// ==========================
server.on("/bme_chart_data_archive", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!request->hasParam("date")) {
        request->send(400, "text/plain", "❌ Вкажіть параметр ?date=YYYY-MM-DD");
        return;
							   
																									
								
																							
								  
    }

    String date = request->getParam("date")->value();
    String path = "/log/" + date + ".csv";
											  
							
								  
							   
							   

    if (!SPIFFS.exists(path)) {
        request->send(404, "application/json", "[]");
        return;
    }
		  

    File file = SPIFFS.open(path, FILE_READ);
											 
			  
    if (!file) {
        request->send(500, "application/json", "[]");
        return;
											   
															 
												 
    }

    Serial.printf("📈 [ARCHIVE] Sending %s (size: %u bytes)\n", path.c_str(), file.size());
    Serial.printf("📈 [ARCHIVE] Free heap before: %u bytes\n", ESP.getFreeHeap());

    AsyncResponseStream *response = request->beginResponseStream("application/json");
    response->print("[");
    bool first = true;

    int lineCounter = 0;
    while (file.available()) {
        String line = file.readStringUntil('\n');
        line.trim();
        if (line == "" || line.startsWith("Time")) continue;
        int idx1 = line.indexOf(',');
        int idx2 = line.indexOf(',', idx1 + 1);
        int idx3 = line.indexOf(',', idx2 + 1);
        if (idx1 < 0 || idx2 < 0 || idx3 < 0) continue;
		
      String timeStr = line.substring(0, idx1);
      float temp = line.substring(idx1 + 1, idx2).toFloat();
      float hum = line.substring(idx2 + 1, idx3).toFloat();
      float press = line.substring(idx3 + 1).toFloat();
      if (!first) response->print(",");
      first = false;
      response->printf(
        "{\"time\":\"%s\",\"temperature\":%.2f,\"humidity\":%.2f,\"pressure\":%.2f}",
        timeStr.c_str(), temp, hum, press
      );
	 

        lineCounter++;
        if (lineCounter > 500) {  // ⚠️ обмеження – не більше 500 точок
            Serial.println("⚠️ [ARCHIVE] JSON обрізано до 500 рядків!");
            break;
        }
    }
    file.close();

    response->print("]");
																									   																	
    request->send(response);
	 
    Serial.printf("📈 [ARCHIVE] JSON sent (%d lines). Free heap after: %u bytes\n", lineCounter, ESP.getFreeHeap());
 });
/////////////////////////////
//Додано. Є повтори
// ==========================
// 📄 /logs – HTML-список доступних архівів
// ==========================
server.on("/logs", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    response->addHeader("Content-Type", "text/html; charset=utf-8");

    Serial.printf("📊 [LOGS] Free heap before: %u bytes\n", ESP.getFreeHeap());

    response->print(F("<h2>Доступні архіви:</h2><ul>"));

    File root = SPIFFS.open("/log");
    if (!root || !root.isDirectory()) {
        response->print(F("<li>Немає доступних лог-файлів.</li>"));
    } else {
        File file = root.openNextFile();
        while (file) {
            String fullPath = file.name();
            if (fullPath.endsWith(".csv")) {
                String fileName = fullPath.substring(fullPath.lastIndexOf('/') + 1);
                String datePart = fileName;
                datePart.replace(".csv", "");

                response->print("<li>");
                response->print("<a href=\"/log_viewer?date=" + datePart + "\">📊 " + datePart + "</a> ");
                response->print("(<a href=\"/log/" + fileName + "\">csv</a>)");
                response->print("</li>");
            }
            file = root.openNextFile();
        }
    }

    response->print(F("</ul><p><a href=\"/\">На головну</a></p>"));
    request->send(response);
	 
    Serial.printf("📊 [LOGS] Free heap after: %u bytes\n", ESP.getFreeHeap());												 
  });

												 
																	  
												 
																	   
										  
															  

// ==========================
// 📂 /log_export – Видає CSV як файл для завантаження
// ==========================
server.on("/log_export", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!request->hasParam("date")) {
        request->send(400, "text/plain", "❌ Вкажіть параметр ?date=YYYY-MM-DD");
        return;
    }

    String date = request->getParam("date")->value();
    String path = "/log/" + date + ".csv";

    if (!SPIFFS.exists(path)) {
        request->send(404, "text/plain", "❌ Файл не знайдено");
        return;
    }
				 
											   

    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, path, "text/csv");
    response->addHeader("Content-Disposition", "attachment; filename=\"" + date + ".csv\"");
    request->send(response);

    Serial.printf("📂 [EXPORT] Відправлено CSV: %s\n", path.c_str());
});
												   
												   
												   
												   
												   

				   

// ==========================
// 📈 /bme_chart_data – JSON з поточними даними (історія)
// ==========================
	 
																		
																											   
							
					
				  
		 
		  
	  
		 
	 
	  
																																 
	 

															
																						  
												   
																						  

																						 
server.on("/bme_chart_data", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    response->print("[");

    bool first = true;
    int count = 0;

    for (int i = 0; i < MAX_POINTS; i++) {
        int idx = (historyIndex + i) % MAX_POINTS;
        const BMEData &d = history[idx];
        if (d.timeStr == "") continue;

      if (!first) response->print(",");
      first = false;

      response->printf(
        "{\"time\":\"%s\",\"temperature\":%.2f,\"humidity\":%.2f,\"pressure\":%.2f}",
        d.timeStr.c_str(), d.temperature, d.humidity, d.pressure
      );

        count++;
        if (count >= 300) break;  // ⚠️ обмеження на кількість точок
					   
																					 
																
		
    }

    response->print("]");
    request->send(response);

    Serial.printf("📈 [BME DATA] JSON відправлено (%d точок)\n", count);
});


// ==========================
// 📊 /bme_chart_data_archive – JSON для архівних графіків
// ==========================
server.on("/bme_chart_data_archive", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!request->hasParam("date")) {
      request->send(400, "text/plain", "❌ Вкажіть параметр ?date=YYYY-MM-DD");
      return;
    }

    String date = request->getParam("date")->value();
    String path = "/log/" + date + ".csv";

    if (!SPIFFS.exists(path)) {
      request->send(404, "application/json", "[]");
      return;
    }
    File file = SPIFFS.open(path, FILE_READ);
    if (!file) {
        request->send(500, "application/json", "[]");
        return;
    }

    Serial.printf("📊 [ARCHIVE] Читаю файл: %s (size: %u bytes)\n", path.c_str(), file.size());
    Serial.printf("📊 [ARCHIVE] Free heap before: %u bytes\n", ESP.getFreeHeap());

    AsyncResponseStream *response = request->beginResponseStream("application/json");
    response->print("[");
    bool first = true;

    int lineCounter = 0;
    while (file.available()) {
      String line = file.readStringUntil('\n');
      line.trim();
      if (line == "" || line.startsWith("Time")) continue;
      int idx1 = line.indexOf(',');
      int idx2 = line.indexOf(',', idx1 + 1);
      int idx3 = line.indexOf(',', idx2 + 1);
      if (idx1 < 0 || idx2 < 0 || idx3 < 0) continue;
      String timeStr = line.substring(0, idx1);
      float temp = line.substring(idx1 + 1, idx2).toFloat();
      float hum = line.substring(idx2 + 1, idx3).toFloat();
      float press = line.substring(idx3 + 1).toFloat();

      if (!first) response->print(",");
      first = false;
      response->printf(
        "{\"time\":\"%s\",\"temperature\":%.2f,\"humidity\":%.2f,\"pressure\":%.2f}",
        timeStr.c_str(), temp, hum, press
      );
																							   
        lineCounter++;
        if (lineCounter > 500) {  // ⚠️ ліміт на архівний JSON
            Serial.println("⚠️ [ARCHIVE] JSON обрізано до 500 рядків!");
            break;
				  						   
        }
    }
    file.close();
    response->print("]");
    request->send(response);
	 
    Serial.printf("📊 [ARCHIVE] JSON sent (%d lines). Free heap after: %u bytes\n",
                  lineCounter, ESP.getFreeHeap());
 });
																	
																		   

														
																												 
	 

																						  
									 
																						  
				 

											 
									 
										 
								  
 